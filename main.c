#include "tests.h"
#include <windows.h>

int main(void) {
    SetConsoleOutputCP(65001);
    run_automatic_tests();
    run_demo();
    return 0;
}