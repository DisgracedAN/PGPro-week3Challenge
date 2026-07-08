#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "my_malloc.h"

//Автоматические тесты
void run_automatic_tests(void) {
    printf("Автоматические тесты my_malloc\n");

    // Тест 1: выделение 0 байт должно возвращать NULL
    assert(my_malloc(0) == NULL);
    printf("  [OK] malloc(0) == NULL\n");

    // Тест 2: выделение маленького блока, запись и чтение
    char* p1 = (char*)my_malloc(10);
    assert(p1 != NULL);
    strcpy(p1, "hello");
    assert(strcmp(p1, "hello") == 0);
    printf("  [OK] malloc(10) и запись 'hello'\n");

    // Тест 3: выделение ещё одного блока, проверка на перекрытие
    char* p2 = (char*)my_malloc(20);
    assert(p2 != NULL);
    strcpy(p2, "world");
    assert(strcmp(p1, "hello") == 0);  // p1 не повреждён
    assert(strcmp(p2, "world") == 0);
    printf("  [OK] второй блок не повредил первый\n");

    // Тест 4: освобождение p1 и выделение снова (должен переиспользовать память)
    my_free(p1);
    char* p3 = (char*)my_malloc(10);
    assert(p3 != NULL);

    printf("  [OK] переиспользование после free\n");

    // Тест 5: calloc – проверка зануления
    int* arr = (int*)my_calloc(5, sizeof(int));
    assert(arr != NULL);
    for (int i = 0; i < 5; i++) {
        assert(arr[i] == 0);
    }
    printf("  [OK] calloc обнуляет память\n");

    // Тест 6: realloc увеличение размера
    char* r1 = (char*)my_malloc(5);
    strcpy(r1, "1234");
    char* r2 = (char*)my_realloc(r1, 10);
    assert(r2 != NULL);
    assert(strcmp(r2, "1234") == 0);  // данные сохранились
    // Пишем дальше
    strcat(r2, "56789");
    assert(strlen(r2) == 9);
    printf("  [OK] realloc увеличение\n");

    // Тест 7: realloc с нулевым размером (должен освобождать)
    char* r3 = (char*)my_malloc(5);
    char* r4 = (char*)my_realloc(r3, 0);
    assert(r4 == NULL);
    printf("  [OK] realloc(ptr, 0) освобождает память\n");

    // Тест 8: realloc с NULL (как malloc)
    char* r5 = (char*)my_realloc(NULL, 10);
    assert(r5 != NULL);
    my_free(r5);
    printf("  [OK] realloc(NULL, size) работает как malloc\n");

    // Тест 9: большой запрос (должен вернуть NULL, если не хватает памяти)
    // HEAP_SIZE = 1 МБ, запросим 2 МБ
    void* big = my_malloc(2 * 1024 * 1024);
    assert(big == NULL);
    printf("  [OK] слишком большой запрос возвращает NULL\n");

    //Тесты calloc
    printf("calloc\n");

    // 1. Нулевое выделение (count=0 или size=0)
    void* c1 = my_calloc(0, 10);
    void* c2 = my_calloc(10, 0);
    assert(c1 == NULL);
    assert(c2 == NULL);
    printf("  [OK] calloc(0,...) и calloc(...,0) возвращают NULL\n");

    // 2. Нормальное выделение и обнуление
    int* arr2 = (int*)my_calloc(100, sizeof(int));   // переименовано в arr2
    assert(arr2 != NULL);
    for (int i = 0; i < 100; i++) {
        assert(arr2[i] == 0);
    }
    // Пишем данные и проверяем, что память независима
    for (int i = 0; i < 100; i++) {
        arr2[i] = i;
    }
    for (int i = 0; i < 100; i++) {
        assert(arr2[i] == i);
    }
    my_free(arr2);
    printf("  [OK] calloc(100, sizeof(int)) – зануление и доступ\n");

    // 3. Проверка выравнивания
    double* darr = (double*)my_calloc(10, sizeof(double));
    assert(darr != NULL);
    // Проверка выравнивания на границу double
    assert(((uintptr_t)darr & (sizeof(double) - 1)) == 0);
    for (int i = 0; i < 10; i++) assert(darr[i] == 0.0);
    my_free(darr);
    printf("  [OK] calloc double – выравнивание и нули\n");

    // 4. Переполнение при умножении
    void* over = my_calloc(SIZE_MAX, 2);
    assert(over == NULL);
    over = my_calloc(2, SIZE_MAX);
    assert(over == NULL);
    printf("  [OK] calloc с переполнением возвращает NULL\n");
    printf("Все автоматические тесты пройдены.\n\n");

}

//Демонстрация 
void run_demo(void) {
    printf("Демонстрация my_malloc\n");

    printf("1. Выделяем 3 блока по 100 байт.\n");
    char* a = (char*)my_malloc(100);
    char* b = (char*)my_malloc(100);
    char* c = (char*)my_malloc(100);
    printf("   Адреса: a=%p, b=%p, c=%p\n", (void*)a, (void*)b, (void*)c);

    printf("2. Записываем в них строки.\n");
    strcpy(a, "Block A");
    strcpy(b, "Block B");
    strcpy(c, "Block C");

    printf("3. Освобождаем блок b.\n");
    my_free(b);

    printf("4. Выделяем новый блок d размером 50 байт (должен занять место b или его часть).\n");
    char* d = (char*)my_malloc(50);
    printf("   Адрес d = %p\n", (void*)d);

    printf("5. Проверяем, что данные в a и c не повреждены:\n");
    printf("   a = '%s'\n", a);
    printf("   c = '%s'\n", c);

    printf("6. Изменяем размер a до 200 байт через realloc.\n");
    a = (char*)my_realloc(a, 200);
    printf("   Новый адрес a = %p\n", (void*)a);
    strcat(a, " + extended");
    printf("   a = '%s'\n", a);

    
    my_free(a);
    my_free(c);
    my_free(d);

    printf("Демонстрация завершена.\n");
}