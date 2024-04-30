#pragma once
#include "SBDP/BrokerParticipantBuiltinTopicData.h"
#include "SBDP/BrokerProxy.h"
#include "Broker/BrokerConfigurationSet.h"
class DiscoveredBrokerData : public BrokerParticipantBuiltinTopicData, public BrokerProxy
{
public:
	DiscoveredBrokerData(int count, int m_count) : BrokerParticipantBuiltinTopicData{ count, m_count }, BrokerProxy{ BROKER_CONFIG::MQTT_PROTOCOL, BROKER_CONFIG::BROKER_IP, BROKER_CONFIG::BROKER_PORT } {};
	DiscoveredBrokerData(int count, int m_count, std::string protocol, std::string ip, int port) : BrokerParticipantBuiltinTopicData{ count, m_count }, BrokerProxy{ protocol, ip, port } {};
};

