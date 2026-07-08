#include "my_malloc.h"
#include <stdint.h>   // SIZE_MAX
#include <string.h>   // memset


#define ALIGN 16//требование по выравниваню (для x64 это 16 байт)
//константы и глобальный пул 

#define HEAP_SIZE (1024 * 1024)//1 МБ статической памяти 
// Куча, выровненная на 16 байт
#ifdef _MSC_VER
__declspec(align(ALIGN)) static char heap[HEAP_SIZE];
#else
_Alignas(ALIGN) static char heap[HEAP_SIZE];
#endif
static int heap_initialized = 0;

//Структура заголовка блока 
typedef struct BlockHeader {
	size_t size;//общий размер блока
	int free;//1 – свободен, 0 – занят
	struct BlockHeader* next;//указатель на следующий свободный блок

}BlockHeader;
//Вычисляем выровненный размер заголовка
#define HEADER_SIZE_RAW sizeof(BlockHeader)
#define HEADER_SIZE (((HEADER_SIZE_RAW + ALIGN - 1) / ALIGN) * ALIGN)

static BlockHeader* free_list = NULL;

//Инициализация

static void init_heap() {
	if (heap_initialized) return;
	heap_initialized = 1;
	// Первый блок занимает всю кучу
	BlockHeader* first = (BlockHeader*)heap;
	first->size = HEAP_SIZE;
	first->free = 1;
	first->next = NULL;
	free_list = first;
}

//Разделение блока
static void split_block(BlockHeader* block, size_t needed) {
	size_t remaining = block->size - needed;
	if (remaining >= HEADER_SIZE + 8) {
		BlockHeader* new_free = (BlockHeader*)((char*)block + needed);
		new_free->size = remaining;
		new_free->free = 1;
		new_free->next = free_list;
		free_list = new_free;
		block->size = needed;
	}
}
// Ищет свободный блок и удаляет его из списка свободных.
// Возвращает указатель на блок или NULL.
static BlockHeader* find_and_remove_free_block(size_t total_size) {
	BlockHeader** curr = &free_list;
	while (*curr) {
		BlockHeader* entry = *curr;
		if (entry->free && entry->size >= total_size) {
			// Удаляем entry из списка
			*curr = entry->next;
			// Изолируем entry, чтобы случайно не осталось ссылок
			entry->next = NULL;
			return entry;
		}
		curr = &entry->next;
	}
	return NULL;
}

//Слияние соседних свободных блоков
static void coalesce_free_list() {
	BlockHeader* curr = free_list;
	while (curr && curr->next) {
		// Проверяем, что оба блока свободны и физически смежны
		if (curr->free && curr->next->free &&
			(char*)curr + curr->size == (char*)curr->next) {
			curr->size += curr->next->size;
			curr->next = curr->next->next;
			// не переходим к следующему, продолжаем с curr
		}
		else {
			curr = curr->next;
		}
	}
}

//Публичные функции
void* my_malloc(size_t size) {
	if (size == 0) return NULL;

	init_heap();

	// Общий размер: заголовок + запрошенные данные, выровненный вверх
	size_t total = HEADER_SIZE + size;
	// Округляем total до ALIGN, чтобы следующий блок начинался с выровненного адреса
	if (total % ALIGN != 0) {
		total = (total + ALIGN - 1) & ~(ALIGN - 1);
	}

	BlockHeader* block = find_and_remove_free_block(total);
	if (!block) return NULL;

	split_block(block, total);
	block->free = 0;
	return (void*)(block + 1);
}

void my_free(void* ptr) {
	if (!ptr) return;

	BlockHeader* block = (BlockHeader*)ptr - 1;
	block->free = 1;
	block->next = free_list;
	free_list = block;

	coalesce_free_list();
}

void* my_calloc(size_t count, size_t size) {
	// Проверка переполнения умножения
	if (count > 0 && size > SIZE_MAX / count) {
		return NULL;   // переполнение
	}
	size_t total = count * size;
	// Если оба нули – стандарт разрешает вернуть NULL или указатель, вернём NULL
	if (total == 0) {
		return NULL;
	}
	void* ptr = my_malloc(total);
	if (ptr) {
		memset(ptr, 0, total);
	}
	return ptr;
}

void* my_realloc(void* ptr, size_t size) {
	if (!ptr) return my_malloc(size);
	if (size == 0) { my_free(ptr); return NULL; }

	BlockHeader* block = (BlockHeader*)ptr - 1;
	size_t old_data_size = block->size - HEADER_SIZE;

	if (size <= old_data_size) return ptr;  

	void* new_ptr = my_malloc(size);
	if (!new_ptr) return NULL;
	memcpy(new_ptr, ptr, old_data_size);
	my_free(ptr);
	return new_ptr;
}
