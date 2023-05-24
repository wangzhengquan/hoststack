
#include <iostream>
#include "usg_common.h"
#include "sole.h"
 
int test_libuuid()
{
	std::string uuid = sole::uuid4().str();; 
	std::cout << "uuid== " << uuid << std::endl;
	return 0;
}
  

int main() {
	test_libuuid();
}