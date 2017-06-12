#include "service.h"

using namespace std;

Service::Service(string serviceName, string hostname, string port)
{
	this->serviceName = serviceName;
	this->hostname = hostname;
	this->port = port;
}
