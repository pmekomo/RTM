#include "service.h"

using namespace std;

Service::Service(string serviceName, string hostname, string port, bool connected)
{
	this->serviceName = serviceName;
	this->hostname = hostname;
	this->port = port;
}
