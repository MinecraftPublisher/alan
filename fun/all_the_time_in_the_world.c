// This is not related to the compiler, rather it's a fun challenge for me.
// This C file aims to print out every single millisecond in a day, and in my opinion, it does it
// damn good. On my laptop, it takes 2s to run, and it takes 4 cycles to calculate, account for
// overflow and write it to a buffer, and 57 cycles for every number printed out.

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define __MAX_I (1000 * 60 * 60 * 24)
const unsigned long long MAX_I = __MAX_I;
#define PER_ENTRY 14
char my_chars[ __MAX_I * PER_ENTRY ];

#define p(name)                                                                                    \
    output          = (char) time.name;                                                            \
    my_chars[ i++ ] = output + '0'

#define cap_3(var, c, next)                                                                        \
    output = time.var < c;                                                                         \
    time.var *= output;                                                                            \
    if (output) goto __PRINT;                                                                      \
    time.next += 1;

static inline long long rdtsc() {
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((long long) hi << 32) | lo;
}

int main() {
    unsigned long long start_cycle = rdtsc();
    unsigned long      i           = 0;

    struct time {
        unsigned char ml_1 : 4;
        unsigned char ml_2 : 4;
        unsigned char ml_3 : 4;
        unsigned char ml_4 : 4;
        unsigned char s_1 : 4;
        unsigned char s_2 : 3;
        unsigned char mi_1 : 4;
        unsigned char mi_2 : 3;
        unsigned char h_1 : 4;
        unsigned char h_2 : 2;
        unsigned char none : 1;
    } time = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    unsigned char output = 0;
__TOP:
    time.ml_1++;

    cap_3(ml_1, 10, ml_2);
    cap_3(ml_2, 10, ml_3);
    cap_3(ml_3, 10, ml_4);
    cap_3(ml_4, 10, s_2);

    cap_3(s_1, 10, s_2);
    cap_3(s_2, 6, mi_1);

    cap_3(mi_1, 10, mi_2);
    cap_3(mi_2, 6, h_1);

    cap_3(h_1, 10, h_2);
    cap_3(h_2, 2, none);

__PRINT:

    p(h_2);
    p(h_1);
    my_chars[ i++ ] = ':';
    p(mi_2);
    p(mi_1);
    my_chars[ i++ ] = ':';
    p(s_2);
    p(s_1);
    my_chars[ i++ ] = '.';
    p(ml_4);
    p(ml_3);
    p(ml_2);
    p(ml_1);

    my_chars[ i++ ] = '\n';

    if (i < MAX_I * PER_ENTRY) goto __TOP;
    unsigned long long end_cycle = rdtsc();
    unsigned long long diff      = end_cycle - start_cycle;
    float              div       = diff / (float) MAX_I;
    write(1, my_chars, (MAX_I * PER_ENTRY) + 1);
    fprintf(
        stderr, "\nTook %llu cycles, %f average and %f per digit\n", diff, div, div / PER_ENTRY);
}