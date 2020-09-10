// string::find_first_of
#include <iostream>       // std::cout
#include <string>         // std::string
#include <cstddef>        // std::size_t
#include <usg_common.h>
int main ()
{
  std::string str ("Please, replace the vowels in this sentence by asterisks.");
  std::size_t found = str.find_first_of(BLANK);
  while (found!=std::string::npos)
  {
    //str[found]='*';
    found=str.find_first_of(BLANK, found+1);
    std::cout << found << '\n';
  }

  std::cout << str << '\n';

  return 0;
}