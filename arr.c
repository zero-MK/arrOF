#include<stdio.h>
int
main ()
{
  long int minus_one = -1;
  char arr[8] = "testAr";
  int i;
  unsigned int uint;		//数组下标就是无符号的
  while (1)
    {
      scanf ("%d", &i);
      printf ("var i : %d\n", i);
      uint = i;
      printf ("index array : %u\n", uint);
      printf ("%c\n", arr[i]);
    }
  return 0;
}
