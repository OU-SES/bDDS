#pragma once
class BrokerParticipantBuiltinTopicData
{
private:
	int connectedClientCount;
	int maximumNumClient;
public:
	BrokerParticipantBuiltinTopicData(int c_count, int m_count) : connectedClientCount(c_count), maximumNumClient(m_count) {};
	int get_connected_client_count() { return connectedClientCount; }
	int get_maximum_num_client() { return maximumNumClient; }
	void set_connected_client_count(int client_count) { this->connectedClientCount = client_count; }
	void set_maximum_num_client(int max_count) { this->maximumNumClient = max_count; }
};

