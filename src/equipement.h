#ifndef EQUIPEMENT
#define EQUIPEMENT

#include <string>

class Equipement
{
	std::string name;
	int num_sock;

	public:
		Equipement();
		Equipement(int val);
		int getNumSock();

};

#endif
