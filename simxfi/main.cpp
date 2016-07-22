#include "cgi.h"
#include <iostream>

using namespace std;

int main(void)
{
	cout<<"Content-Type: text/html;\n\n";
	head("Ma page en C !");
	
	cout<<"Hello World";
	
	foot();
	return 0;
}
