#ifndef SERVICE
#define SERVICE
#include <string>

class Service
{
	std::string serviceName;
	std::string hostname;
	std::string port;

	public:
		int socket;
		Service(std::string serviceName, std::string hostname, std::string port);
		std::string getServiceName(){return serviceName;}
		std::string getHostname(){return hostname;}
		std::string getPort() {return port;}
		
};

#endif
