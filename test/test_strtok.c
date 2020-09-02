// #include "usg_common.h"

 
/* strtok example */
#include <stdio.h>
#include <string.h>

int test1() {
	char volume[]=":B";
	char *src = strtok(volume, ":");
  char *dest = strtok(NULL, ":");
  printf("%s %d\n", src, src == NULL);
  printf("%s %d\n", dest, dest == NULL);
}

int test2 ()
{
  char str[] ="- This, a sample string.";
  char * pch;
  printf ("Splitting string \"%s\" into tokens:\n",str);
  pch = strtok (str," ,.-");
  while (pch != NULL)
  {
    printf ("%s\n",pch);
    pch = strtok (NULL, " ,.-");
  }
  return 0;
}

int main() {
	test2();
	test1();
}
