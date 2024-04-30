//=============================================================================
/**
 * @file    BrokerTask.h
 *
 *
 * This file contains DDS Broker specific data type task.
 * The task id should be unique and identifiable. 
 * The type of task is defined at BrokerConfigurationSet.
 *
 *
 * @author Hwimin Kim <hwiminkim@oakland.edu>
 */
 //=============================================================================

#ifndef BROKER_TASK_H
#define BROKER_TASK_H
#include <string>
#include "BrokerConfigurationSet.h"


#include <SESSolutionX/DDSThread/DDSThread.cpp>
#include <boost/asio.hpp>
#include <SESSolutionX/Broker/MQTTSession.h>


class Broker_task {
	public:
		
		int get_task_id() { return task_id; }
		std::string get_topic() { return topic; }
		std::string get_data() { return data; }
		
		Broker_task(int task_id_, std::string topic_, std::string data_) 
		{
			task_id = task_id_;
			topic = topic_;
			data = data_;
		}

		Broker_task() 
		{
			task_id = -1;
			topic = "N/A";
			data = "N/A";
		}

	private:
		short task_id;		//The id should be identifiable.
		short task_type;	//The type is defined at BrokerConfigurationSet.h
		short source_id;	//entity id
		std::string topic;
		std::string data;
};



class MQTT_task {
	public:

		int get_task_option() { return task_option_; }
		int get_task_id() { return task_id_; }
		std::vector<char> get_data() { return data_; }
		std::string get_client_id() { return client_id_; }

		MQTT_task(int task_id, int task_option, std::vector<char> data, std::string client_id) :
			task_id_(task_id), task_option_(task_option), data_(data), client_id_(client_id) {}

		MQTT_task() :
			task_id_(-1), task_option_(-1), data_(), client_id_("default") 
		{
			data_ = std::vector<char>();
		}

		bool operator<(const MQTT_task& other) const {
			return task_option_ > other.task_option_; 
		}

	private:
		int task_id_;
		int task_option_;
		std::vector<char> data_;
		std::string client_id_;
};

extern DDSThread::DDSThreadPool dds_task_pool;
extern boost::asio::thread_pool *dds_thread_pool;
extern std::queue <std::string> broker_cache;
extern std::string log_name;
extern int task_count;
#endif BROKER_TASK_H