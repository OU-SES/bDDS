#pragma once
#include <string>
#include <vector>
struct Locator{
	std::string ip;
	int port;
	Locator(std::string ip, int port) : ip(ip), port(port) {};
};

class BrokerProxy
{
public:
	BrokerProxy(std::string protocol, std::string ip, int port) 
	{
		brokerProtocolKind.push_back(protocol);
		Locator locator(ip,port);
		brokerLocator.push_back(locator);
	}
	std::vector<std::string> get_protocol_list() { return this->brokerProtocolKind; }
	std::vector<Locator> get_broker_locator() { return this->brokerLocator; }
	void add_locator(std::string ip, int port) { brokerLocator.push_back(Locator(ip,port)); }
private:
	std::vector<std::string> brokerProtocolKind;
	std::vector<Locator> brokerLocator;
};

