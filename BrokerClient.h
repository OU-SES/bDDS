#pragma once

#ifndef __BROKER_CLIENT_H
#define __BROKER_CLIENT_H

#include "MQTTPacket/SubscribePayload.h"
#include "Broker/BrokerSubscriber.h"

#include <iostream>
struct Broker_topic{
	Topic s_topic;
	std::vector<BROKERSUB::Session *> client_sessions;
	std::vector<MQTTSession*> mqtt_client_sessions;

	Broker_topic(Topic subscription) : s_topic(subscription) {}
	
	void set_subscription(Topic subscription) { s_topic = subscription; }
	Topic get_sub_topic() { return s_topic; }
	int get_sub_topic_qos() { return s_topic.get_maximum_qos(); }
	std::string get_sub_topic_str() { return s_topic.get_filter().get_data(); }
	void add_client(BROKERSUB::Session *client) 
	{ 
		client_sessions.push_back(client); 
		/*for (auto client : client_sessions)
		{
			std::cout << "current state = " << client->get_client_no() << std::endl;
		}
		*/
	}

	void add_mqtt_client(MQTTSession* mqtt_client) 
	{
		mqtt_client_sessions.push_back(mqtt_client);
	}
};
extern int client_no;
extern std::vector<Broker_topic> topic_list;

namespace BROKER_CONFIGURATION {
	
	static Broker_topic add_mqtt_client_to_topic(std::string topic_name, MQTTSession* client)
	{
		for (auto temp = topic_list.begin(); temp != topic_list.end(); ++temp)
		{
			if (!temp->get_sub_topic_str().compare(topic_name))
			{
				temp->add_mqtt_client(client);
				//std::cout << "Topic[" << topic_name << "] add client[" << client->name << "] into the list" << std::endl;
				return temp->s_topic;
			}
		}
		
		Topic n_topic(topic_name, 0, 0, 0, 0);
		Broker_topic b_item(n_topic);
		topic_list.push_back(b_item);
		//std::cout << "Topic[" << topic_name << "] is added in broker" << std::endl;
		topic_list.at(topic_list.size() - 1).add_mqtt_client(client);
		return topic_list.at(topic_list.size() - 1);
	}

	static Broker_topic add_client_to_topic(std::string topic_name, BROKERSUB::Session* client)
	{
		for (auto temp = topic_list.begin(); temp != topic_list.end(); ++temp)
		{
			if (!temp->get_sub_topic_str().compare(topic_name))
			{
				temp->add_client(client);
				//std::cout << "Topic[" << topic_name << "] add client[" << client->name << "] into the list" << std::endl;
				return temp->s_topic;
			}
		}

		Topic n_topic(topic_name, 0, 0, 0, 0);
		Broker_topic b_item(n_topic);
		topic_list.push_back(b_item);
		//std::cout << "Topic[" << topic_name << "] is added in broker" << std::endl;
		topic_list.at(topic_list.size() - 1).add_client(client);
		return topic_list.at(topic_list.size() - 1);
	}

	static Broker_topic find_topic(std::string topic_name)
	{
		for (Broker_topic temp : topic_list)
		{
			if (!temp.get_sub_topic_str().compare(topic_name)) 
			{
				return temp;
			}
		}
		Topic n_topic("null",0,0,0,0);
		Broker_topic b_item(n_topic);

		return b_item;
	}

	static void send_message(Broker_topic topic, std::vector<char> packet, int size) 
	{
		for (auto temp : topic.client_sessions) 
		{
			//qos = topic.get_sub_topic_qos();
			//temp->just_do_write(packet, size);
			//boost::thread publisher_thrd(boost::bind(&BROKERSUB::Session::do_sync_write, temp, packet, size));
			/*dds_task_pool.enqueue([temp, &packet, &size]() {
				temp->do_sync_write(packet, size);
			});*/
			temp->do_sync_write(packet, size);
			//if (qos > 0) temp->do_read();
			//if (qos > 1) temp->do_read();
		}
	}

	static void send_message_dis(Broker_topic topic, std::vector<char> packet, int size)
	{
		int qos = 0;
		for (auto& temp : topic.client_sessions)
		{
			//qos = topic.get_sub_topic_qos();
			//temp->just_do_write(packet, size);
			temp->do_sync_write(packet, size);
			//if (qos > 0) temp->do_read();
			//if (qos > 1) temp->do_read();
		}
	}


	static void increase_count() { client_no++; /*std::cout << "current_client_num = " + client_no << std::endl;*/ }
	static int get_count() { /*std::cout << "current_client_num = " + client_no << "will be returned..." << std::endl;*/ return client_no; }
}

#endif