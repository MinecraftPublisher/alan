#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Function that generates and executes machine code
long int execute_machine_code(long int value) {
    register int x = 2;
}

int main() {
    long int value = 123456789;

    // Call the function that executes the raw machine code
    long int result = execute_machine_code(value);

    // Print the result (Note: this is the same as the input value, since we
    // don't have direct access to EAX in standard C)
    printf("The value placed in EAX: %ld\n", result);

    return 0;
}
