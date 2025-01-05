#include <sys/mman.h>

int main() {
    int* val = mmap(0x0, 32, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);

    return 0;
}
