#ifndef BROKER_CONFIG
#define BROKER_CONFIG
namespace BROKER_CONFIG{
	const std::string BROKER_ID = "RSU_NY_UNIT_001";
	const short TASK_PUB_CLIENT = 0x00;
	const short TASK_PUB_ALL = 0x01;
	//const std::string SBDP_MULTICAST_IP = "224.0.0.0";
	const std::string SBDP_MULTICAST_IP = "239.255.0.11";
	const int SBDP_MULTICAST_PORT = 1234;
	const std::string MQTT_PROTOCOL = "MQTTv5.0";
	//const std::string BROKER_IP = "10.0.0.70";
	const std::string BROKER_IP = "192.168.0.200";
	const int BROKER_PORT = 32888; //Ori_broker 32888, alt_broker 32889
	const int MAX_CLIENT_NUM = 250;
	const int DEFAULT_MAX_CLIENT_NUM = 250;
}

namespace BROKER_ALT_CONFIG {
	const std::string BROKER_ID = "RSU_NY_UNIT_002";
	const short TASK_PUB_CLIENT = 0x00;
	const short TASK_PUB_ALL = 0x01;
	//const std::string SBDP_MULTICAST_IP = "224.0.0.0";
	const std::string SBDP_MULTICAST_IP = "239.255.0.11";;
	const int SBDP_MULTICAST_PORT = 1234;
	const std::string MQTT_PROTOCOL = "MQTTv5.0";
	//const std::string BROKER_IP = "10.0.0.70";
	//const std::string BROKER_IP = "192.168.0.117";
	const std::string BROKER_IP = "192.168.0.200";
	const int BROKER_PORT = 32889; //Ori_broker 32888, alt_broker 32889
	const int MAX_CLIENT_NUM = 250;
	const int DEFAULT_MAX_CLIENT_NUM = 250;
}
extern int current_client_num;
extern int max_retain_queue_message_count;
extern bool broker_trigger_;
extern int global_qos;
#endif // !BROKER_CONFIG
