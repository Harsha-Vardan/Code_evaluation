#include <iostream>

int main() {
    int* p = nullptr;
    *p = 10; // Segfault
    return 0;
}
