// Code taken from https://lcamtuf.substack.com/p/weekend-projects-getting-silly-with
// Weird stuff!
// Compiles only with GCC.

#include <stdio.h>

int main() {

  /* Iterate from i = 0 to i = 5: */

  int i = (i = 0) & ({_: 0;}) | printf("i = %d\n", i) * 
          (++i > 5) ?: ({goto *&&_; 0;});

  return 0;

}