#pragma once
#include "BrokerParticipantBuiltinTopicData.h"
#include "../DiscoveredBrokerData.h"
#include <string>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
class AltDiscoveredBrokerData : public DiscoveredBrokerData
{
private:
	int expectedClientNum;
	std::string broker_id;
		
public:
	AltDiscoveredBrokerData(std::string broker_id, int broker_expected_client_num, int broker_connected_client_num, int broker_maximum_client_num, std::string broker_protocol, std::string broker_ip, int broker_port)
		:DiscoveredBrokerData{ broker_connected_client_num, broker_maximum_client_num, broker_protocol, broker_ip, broker_port }, broker_id(broker_id), expectedClientNum(broker_expected_client_num){};
	AltDiscoveredBrokerData() : DiscoveredBrokerData{ 0, BROKER_CONFIG::DEFAULT_MAX_CLIENT_NUM }, expectedClientNum(0), broker_id(BROKER_CONFIG::BROKER_ID){};
	AltDiscoveredBrokerData(int max_num) : DiscoveredBrokerData{ 0, max_num }, expectedClientNum(0), broker_id(BROKER_CONFIG::BROKER_ID){};
	void set_expected_client_num(int expect) { this->expectedClientNum = expect; }
	int get_expected_client_num() { return this->expectedClientNum; }
	void set_broker_id(std::string name) { this->broker_id = name; }
	std::string get_broker_id() { return this->broker_id; }
	std::string to_json();
};

