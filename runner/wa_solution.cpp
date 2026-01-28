#include <iostream>

int main() {
    int a, b;
    if (!(std::cin >> a >> b)) return 0;
    std::cout << a + b + 1 << std::endl; // Wrong answer
    return 0;
}
