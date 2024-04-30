#include "MQTTSession.h"
#include <MQTTPacket/MQTTControlPacketList.h>
#include "MQTTPacket/PacketInterpreter.h"
#include "MQTTPacket/MQTTMessageFactory.h"
#include "MQTTPacket/PacketGenerator.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <SESSolutionX/SBDP/SBDPThreadMutex.h>
#include <SESSolutionX/BrokerClient.h>
#include <SESSolutionX/DDSThread/DDSThread.h>
#include "BrokerTask.h"
#include <SESSolutionX/BrokerThreadMutex.h>
using namespace std::chrono;

void MQTTSession::start() 
{
	{
		std::lock_guard<std::mutex> lock(mqtt_clients_mutex_);
		mqtt_clients_.insert(shared_from_this());
	}
	read();
}

void MQTTSession::send(const std::string& message) 
{
	//boost::asio::write(socket_, boost::asio::buffer(message, MAX_PACKET_SIZE));
	auto self(shared_from_this());
	boost::asio::async_write(socket_, boost::asio::buffer(message, MAX_PACKET_SIZE),
        [this, self](boost::system::error_code ec, std::size_t length) {
            //Erase the client if there is an issue on trasnfering data
            if (ec) {
				//std::ofstream myfile;
				//myfile.open("rec_evednt.txt", ios::app);
				//myfile << "[Critical][Client" << name <<"] Error sending message : " << ec.message() << std::endl;
				//myfile.close();
				//std::lock_guard<std::mutex> lock(mqtt_clients_mutex_);
                //mqtt_clients_.erase(self);
            }
        });
}

int MQTTSession::send(std::vector<char> message, int task_id)
{
	/*char data_send[1024];
	strncpy(data_send, message.data(), sizeof(data_send) - 1);
	data_send[sizeof(data_send) - 1] = '\0';
	*/
	//boost::asio::write(socket_, boost::asio::buffer(message.data(), MAX_PACKET_SIZE));
	if (message.data() == nullptr)
	{
		std::cout << "----[Message] NULL point detected in send...." << std::endl;
	}

	if (message.size() <= 0) 
	{
		std::cout << "----[Message] message is 0..." << std::endl;
	}
	else if (message.size() > 1024) 
	{
		//std::ofstream myfile;
		//myfile.open("rec_evednt.txt", ios::app);
		//myfile << "----[message][size:" << message.size() << "]" << std::endl;
		//myfile.close();
		return -1;
	}

	auto self(shared_from_this());
	boost::asio::async_write(socket_, boost::asio::buffer(message.data(), MAX_PACKET_SIZE),
		[this, self, &message, &task_id](boost::system::error_code ec, std::size_t length) {
			//Erase the client if there is an issue on trasnfering data
			if (ec) {
				//std::ofstream myfile;
				//myfile.open("rec_evednt.txt", ios::app);
				int arrived = duration_cast<std::chrono::milliseconds>(system_clock::now().time_since_epoch()).count();
				
				//myfile << "[Critical][Client" << name << "][length:" << length << "][task_id:" << task_id << "] Error sending message : " << ec.message() << std::endl;
				//myfile << "----[message][size:" << message.size() << "]" << std::endl;
				
				//myfile.close();
				//std::lock_guard<std::mutex> lock(mqtt_clients_mutex_);
				//mqtt_clients_.erase(self);
				return -1;
			}
		});
	return 0;
}

void MQTTSession::read() 
{
    auto self(shared_from_this());

    //socket_.async_read_some(boost::asio::buffer(packet_, MAX_PACKET_SIZE),

	socket_.async_receive(boost::asio::buffer(packet_, MAX_PACKET_SIZE),
        [this, self](boost::system::error_code ec, std::size_t length) 
        {
            if (!ec) 
            {
                //read and generate response message.
				message_process(packet_);
                //mqtt_messages_.push(std::string(packet_, length));
                read();
            }
            else 
            {
				//std::ofstream myfile;
				//myfile.open("rec_evednt.txt", ios::app);
				//myfile << "[Critical][Client" << name << "][" << length << "] Error reading message : " << ec.message() << std::endl;
				//myfile.close();
				//std::lock_guard<std::mutex> lock(mqtt_clients_mutex_);
                //mqtt_clients_.erase(self);
            }
        });
}

void MQTTSession::message_process(char * recv_data)
{
	bool send_flag_ = false;;
	int first_byte_fheader = recv_data[0];
	const int message_type = (first_byte_fheader >> 4) & 0x0f;
	//std::cout << "Message recieved:" << std::hex << first_byte_fheader << std::endl;
	
	std::string status = "before try";

	try
	{
		if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_CONNECT) {
			status = "before connect";

			MQTTConnectMessage temp;
			interpret_connect_packet(&temp, recv_data);

			//update connected client information
			connected_client.set_protocol(temp.get_vHeader().get_protocol_name().get_data() + std::to_string(temp.get_vHeader().get_protocol_version()));
			connected_client.set_flags(temp.get_vHeader().get_connect_flags());
			connected_client.set_keep_alive(temp.get_vHeader().get_keep_alive().get_data());
			connected_client.set_id(temp.get_payload().get_id().get_data());
			connected_client.set_password(temp.get_payload().get_password().get_data());
			this->name = temp.get_payload().get_id().get_data();

			SBDP_THREAD::task_mutex_wlock(&SBDP_THREAD::mutex);
			BROKER_CONFIGURATION::increase_count();
			client_n = BROKER_CONFIGURATION::get_count();
			SBDP_THREAD::task_mutex_unlock(&SBDP_THREAD::mutex);

			//std::cout << "client come:" << temp.get_payload().get_id().get_data() << std::endl;

			//// get broker
			//Locator broker_address = AlternativeServerInfo::get_broker(connected_client);
			//// convert Locator into JSON string
			boost::property_tree::ptree output;
			//output.put("BROKER.Ip", broker_address.ip);
			//output.put("BROKER.Port", broker_address.port);
			//output.put("BROKER.Ip", "192.168.0.117");
			output.put("BROKER.Ip", "192.168.0.117");
			output.put("BROKER.Port", "32889");
			std::stringstream broker_locator_str;
			boost::property_tree::json_parser::write_json(broker_locator_str, output);

			MQTTConnackMessage connack;
			generate_default_connack(connack, "Client_" + client_n, "ref", broker_locator_str.str());
			std::vector<char> packet;
			packet.clear();
			convert_connact_into_packet(&connack, &packet);

			//mqtt_messages_.push(std::string(packet_, length));
			//std::lock_guard<std::mutex> lock(mqtt_messages_mutex_);

			// previous version
			//std::unique_lock<std::mutex> lock(mqtt_messages_mutex_);

			//BROKER_THREAD::task_mutex_wlock(&BROKER_THREAD::mutex);
			//MQTT_task task(1, packet, connected_client.get_id());
			MQTT_task task(task_count++, 4, packet, connected_client.get_id());
			//std::ofstream myfile;
			//myfile.open("rec_evednt.txt", ios::app);
			//myfile << "MQTT_CONACT_Meesage_added_[" << task.get_client_id() << "][task id:" << task.get_task_id() << "][option:" << task.get_task_option() << "][size" << task.get_data().size() << "]messaage " << std::endl;
			//myfile.close();

			// Previous version
			//this->mqtt_messages_.push(std::move(task));
			// lock free version
			MQTT_broker_task_queue.enqueue(std::move(task));

			//std::cout << "client " << temp.get_payload().get_id().get_data() << " will recieve connack" << std::endl;
			//BROKER_THREAD::task_mutex_unlock(&BROKER_THREAD::mutex);

			// previous version
			//lock.unlock();
			//message_cv_.notify_one();
			//message_cv_.notify_all();
			status = "after connect";
		}
		else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE)
		{
			status = "before subscribe";
			MQTTSubscribeMessage temp;
			interpret_subscribe_packet(&temp, recv_data);
			std::vector<Topic> subscription_list = temp.get_payload().get_list();

			Topic data = temp.get_payload().get_list().at(0);
			int qos = temp.get_payload().get_list().at(0).get_maximum_qos();

			DTHREAD::task_mutex_wlock();
			BROKER_CONFIGURATION::add_mqtt_client_to_topic(data.filter.get_data(), this);
			DTHREAD::task_mutex_unlock();

			//std::cout << "sub come:" << std::endl;


			MQTTSubackMessage suback;
			generate_default_suback(suback, client_n, 1);
			std::vector<char> packet;
			packet.clear();
			convert_suback_into_packet(&suback, &packet);

			{//mqtt_messages_.push(std::string(packet_, length));
				//std::lock_guard<std::mutex> lock(mqtt_messages_mutex_);
				//std::unique_lock<std::mutex> lock(mqtt_messages_mutex_);
				//BROKER_THREAD::task_mutex_wlock(&BROKER_THREAD::mutex);
				//MQTT_task task(1, packet, connected_client.get_id());
				MQTT_task task(task_count++, 1, packet, connected_client.get_id());
				//this->mqtt_messages_.push(std::move(task));
				MQTT_broker_task_queue.enqueue(std::move(task));

				//std::ofstream myfile;
				//myfile.open("rec_evednt.txt", ios::app);
				//myfile << "MQTT_Suback_Meesage_added_[" << task.get_client_id() << "][task id:" << task.get_task_id() << "][option:" << task.get_task_option() << "][size" << task.get_data().size() << "]messaage " << std::endl;
				//myfile.close();
				//std::cout << "suack sent:" << std::endl;
				//BROKER_THREAD::task_mutex_unlock(&BROKER_THREAD::mutex);
				//lock.unlock();
				//message_cv_.notify_one();
				//message_cv_.notify_all();
			}
			if (!broker_cache.empty())
				//if (!MQTT_broker_retain_queue.empty())
			{
				//DTHREAD::task_mutex_rlock();
				//std::queue <std::string> retain_queue(broker_cache);
				retain_messages = std::queue<std::string>(broker_cache);
				//MQTTSession::retain_messages = MQTT_broker_retain_queue;

				//DTHREAD::task_mutex_unlock();

				//std::cout << "[SYSTEM] there are [" << retain_queue.size() << "] messages in the queue" << std::endl;
				/*if (qos > 0) {
					if (retain_messages.size() > 0)
					{
						//boost::this_thread::sleep_for(boost::chrono::milliseconds(5));
						MQTTPublishMessage p_message;
						std::vector<char> packet_;
						std::string retain_message = retain_messages.front();
						//cout << "[SYSTEM] retained queue [" << retain_message << "] message in the queue" << endl;
						retain_messages.pop();
						std::stringstream stream_gps(retain_message);
						boost::property_tree::ptree tree_gps;
						boost::property_tree::read_json(stream_gps, tree_gps);
						std::string subject_id = tree_gps.get<std::string>("Vehicle_GPS.subject_id");

						generate_default_publish(p_message, stoi(subject_id), "GPS", retain_message, global_qos);
						convert_publish_into_packet(&p_message, &packet_);
						//packet_.push_back('\n');
						{


							MQTT_task task(task_count++, 2, packet_, connected_client.get_id());

							//std::ofstream myfile;
							//myfile.open("rec_evednt.txt", ios::app);
							//myfile << "Retain_Message_added_[" << task.get_client_id() << "][task id:" << task.get_task_id() << "][option:" << task.get_task_option() << "][size" << task.get_data().size() << "]messaage " << std::endl;
							//myfile.close();

							// lock free version
							MQTT_broker_task_queue.enqueue(std::move(task));
							//this->mqtt_messages_.push(std::move(task));
							//message_cv_.notify_one();
							//message_cv_.notify_all();
						}
					}
				}
				else
				{
					while (!retain_messages.empty())
					{
						MQTTPublishMessage p_message;
						std::vector<char> packet_;
						std::string retain_message = retain_messages.front();
						//cout << "[SYSTEM] retained queue [" << retain_message << "] message in the queue" << endl;
						retain_messages.pop();
						std::stringstream stream_gps(retain_message);
						boost::property_tree::ptree tree_gps;
						boost::property_tree::read_json(stream_gps, tree_gps);
						std::string subject_id = tree_gps.get<std::string>("Vehicle_GPS.subject_id");

						generate_default_publish(p_message, stoi(subject_id), "GPS", retain_message, global_qos);
						convert_publish_into_packet(&p_message, &packet_);
						//packet_.push_back('\n');
						{
							std::lock_guard<std::mutex> lock(mqtt_messages_mutex_);
							//MQTT_task task(1, packet_, connected_client.get_id());
							MQTT_task task(task_count++, 2, packet_, connected_client.get_id());
							//std::ofstream myfile;
							//myfile.open("rec_evednt.txt", ios::app);
							//myfile << "Retain_Message_added_[" << task.get_client_id() << "][task id:" << task.get_task_id() << "][option:" << task.get_task_option() << "][size" << task.get_data().size() << "]messaage " << std::endl;
							//myfile.close();
							//goothis->mqtt_messages_.push(std::move(task));
							message_cv_.notify_one();
							//message_cv_.notify_all();
						}
					}
				}*/
				status = "after subscribe";
			}
			//send_flag = false;
		}
		else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBACK)
		{
			status = "before publish";
			MQTTPubackMessage puback;
			interpret_puback_packet(&puback, recv_data);

			//cout << "puback received..." << endl;
			/*
			if (retain_messages.size() > 0)
			{
				//boost::this_thread::sleep_for(boost::chrono::milliseconds(5));
				MQTTPublishMessage p_message;
				std::vector<char> packet_;
				std::string retain_message = retain_messages.front();
				//cout << "[SYSTEM] retained queue [" << retain_message << "] message in the queue" << endl;
				retain_messages.pop();
				std::stringstream stream_gps(retain_message);
				boost::property_tree::ptree tree_gps;
				boost::property_tree::read_json(stream_gps, tree_gps);
				std::string subject_id = tree_gps.get<std::string>("Vehicle_GPS.subject_id");

				generate_default_publish(p_message, stoi(subject_id), "GPS", retain_message, global_qos);
				convert_publish_into_packet(&p_message, &packet_);
				//packet_.push_back('\n');
				{
					//std::lock_guard<std::mutex> lock(mqtt_messages_mutex_);
					MQTT_task task(task_count++, 3, packet_, connected_client.get_id());
					//std::ofstream myfile;
					//myfile.open("rec_evednt.txt", ios::app);
					//myfile << "MQTT_PUBACK_Message_added_[" << task.get_client_id() << "][task id:" << task.get_task_id() << "][option:" << task.get_task_option() << "][size" << task.get_data().size() << "]messaage " << std::endl;
					//myfile.close();

					// lock free version
					MQTT_broker_task_queue.enqueue(std::move(task));
					//this->mqtt_messages_.push(std::move(task));
					//message_cv_.notify_one();
					//message_cv_.notify_all();
				}
			}*/
			status = "after publish";
		}
		else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBREC)
		{
			status = "before pubrec";
			std::vector<char> packet;
			MQTTPubrecMessage pubrec;
			interpret_pubrec_packet(&pubrec, recv_data);
			TwoByteInteger packet_id = pubrec.get_vHeader().get_packet_identifier();
			//cout << "pubrec received..." << packet_id.get_data() << endl;

			MQTTPubrelMessage pubrel;
			generate_default_pubrel(pubrel, packet_id.get_data());
			convert_pubrel_into_packet(&pubrel, &packet);

			/*MQTTPubrelMessage pubrel_r;
			interpret_pubrel_packet(&pubrel_r, packet.data());
			//cout << "pubrel sent...." << pubrel_r.get_vHeader().get_packet_identifier().get_data() << endl;*/

			{

				//mqtt_messages_.push(std::string(packet_, length));
				//std::lock_guard<std::mutex> lock(mqtt_messages_mutex_);
				//MQTT_task task(2, packet, connected_client.get_id());
				MQTT_task task(task_count++, 5, packet, connected_client.get_id());
				//std::ofstream myfile;
				//myfile.open("rec_evednt.txt", ios::app);
				//myfile << "MQTT_Pubrel_Message_added_[" << task.get_client_id() << "][task id:" << task.get_task_id() << "][option:" << task.get_task_option() << "][size" << task.get_data().size() << "]messaage " << std::endl;
				//myfile.close();
				//this->mqtt_messages_.push(std::move(task));
				//std::cout << "surel sent:" << std::endl;

				//myfile.open("rec_evednt.txt", ios::app);
				//int arrived = duration_cast<std::chrono::milliseconds>(system_clock::now().time_since_epoch()).count();
				//myfile << "[SYSTEM]rec client[" << this->name << "][" << packet_id.get_data() << "]. received at[" << arrived << "]" << std::endl;
				//myfile.close();

				// lock free version
				MQTT_broker_task_queue.enqueue(std::move(task));
				//message_cv_.notify_one();
				//message_cv_.notify_all();
			}
			status = "after pubrec";
		}
		else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBREL)
		{
			status = "before pubrel";
			std::vector<char> packet;
			MQTTPubrelMessage pubrel;
			interpret_pubrel_packet(&pubrel, recv_data);
			auto packet_id = pubrel.get_vHeader().get_packet_identifier().get_data();
			//std::cout << "pubrel received..." << packet_id << std::endl;

			MQTTPubcompMessage pubcomp;
			generate_default_pubcomp(pubcomp, packet_id);
			convert_pubcomp_into_packet(&pubcomp, &packet);

			{
				MQTT_task task(task_count++, 5, packet, connected_client.get_id());
				MQTT_broker_task_queue.enqueue(std::move(task));
			}

			packet_id = pubcomp.get_vHeader().get_packet_identifier().get_data();
			auto target_publish_msg = MQTT_publish_map.find(packet_id)->second;
			MQTT_publish_map.erase(packet_id);

			MQTTPublishMessage publish;
			interpret_publish_packet(&publish, target_publish_msg.data());
			Broker_task BK_task(task_count, publish.get_vHeader().get_topic_name_str(), publish.get_payload().get_data());
			DDS_task_queue.enqueue(std::move(BK_task));

			MQTT_task task(task_count++, 0, target_publish_msg, "-99");
			MQTT_broker_task_queue.enqueue(std::move(task));
			status = "after pubrel";
		}
		else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBCOMP)
		{
			status = "before pubcomp";
			MQTTPubcompMessage pubcomp;
			interpret_pubcomp_packet(&pubcomp, recv_data);
			//cout << "pubcomp received..." << pubcomp.get_vHeader().get_packet_identifier().get_data() << endl;
			/*if (retain_messages.size() > 0)
			{
				//boost::this_thread::sleep_for(boost::chrono::milliseconds(5));
				MQTTPublishMessage p_message;
				std::vector<char> packet_;
				std::string retain_message = retain_messages.front();
				//cout << "[SYSTEM] retained queue [" << retain_message << "] message in the queue" << endl;
				retain_messages.pop();
				std::stringstream stream_gps(retain_message);
				boost::property_
				::ptree tree_gps;
				boost::property_tree::read_json(stream_gps, tree_gps);
				std::string subject_id = tree_gps.get<std::string>("Vehicle_GPS.subject_id");

				generate_default_publish(p_message, stoi(subject_id), "GPS", retain_message, global_qos);
				convert_publish_into_packet(&p_message, &packet_);
				//packet_.push_back('\n');
				{
					//std::lock_guard<std::mutex> lock(mqtt_messages_mutex_);
					MQTT_task task(task_count++, 3, packet_, connected_client.get_id());
					//std::ofstream myfile;
					//myfile.open("rec_evednt.txt", ios::app);
					//myfile << "MQTT_Pubcmp_Retain_Message_added_[" << task.get_client_id() << "][task id:" << task.get_task_id() << "][option:" << task.get_task_option() << "][size" << task.get_data().size() << "]messaage " << std::endl;
					//myfile.close();
					// lock free version
					MQTT_broker_task_queue.enqueue(std::move(task));
					//this->mqtt_messages_.push(std::move(task));
					//message_cv_.notify_one();
					//message_cv_.notify_all();
				}
			}*/
			status = "after pubcomp";

		}
		else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBLISH)
		{
			MQTTPublishMessage publish;
			status = "before publish";

			std::vector<char> recv_data_(*recv_data);
			interpret_publish_packet(&publish, recv_data);
			//std::ofstream myfile;

			//myfile.open("rec_evednt.txt", ios::app);
			//myfile << "MQTT_PUBLISH_recv_data____:" << std::endl;
			/*for (char c : recv_data_) {

				myfile << "0x"
					<< std::hex  // output in hexadecimal format
					<< std::uppercase  // use uppercase letters for A-F
					<< std::setw(2)  // ensure at least two digits are written
					<< std::setfill('0')  // pad with zeros if needed
					<< static_cast<int>(c)  // cast char to int
					<< " ";
			}
			myfile << "\n";*/

			/*TwoByteInteger packet_id = publish.get_vHeader().get_packet_identifier();

			auto payload = publish.get_payload().get_data();
			std::stringstream stream_gps(payload);
			boost::property_tree::ptree tree_gps;
			boost::property_tree::read_json(stream_gps, tree_gps);

			auto topic_name = publish.get_vHeader().get_topic_name_str();
			*/
			//Broker_topic topic = BROKER_CONFIGURATION::find_topic(topic_name);

			//myfile.open("rec_evednt.txt", ios::app);
			//myfile << "MQTT_PUBLISH_received:" << payload << std::endl;
			//myfile.close();
			auto f_header = publish.get_fHeader();
			auto qos = f_header.get_qos();

			if (qos != 0)
			{
				auto packet_id = publish.get_vHeader().get_packet_identifier().get_data();
				std::vector<char> packet;
				std::vector<char> ack_packet;
				packet.clear();
				ack_packet.clear();

				status = "after qos";

				if (qos == 1)
				{
					MQTTPubackMessage temp;
					generate_puback(temp, packet_id, 0);
					convert_puback_into_packet(&temp, &ack_packet);
					ack_packet.push_back('\n');
				}
				else if (qos == 2)
				{
					MQTTPubrecMessage temp;
					generate_pubrec(temp, packet_id, 0);
					convert_pubrec_into_packet(&temp, &ack_packet);
					ack_packet.push_back('\n');
					convert_publish_into_packet(&publish, &packet);
					MQTT_publish_map[packet_id] = packet;
					//std::cout << "client_session[" << this->connected_client.get_id() << "] store message [" << packet_id << "] into the map" << std::endl;
				}

				MQTT_task task(task_count++, 5, ack_packet, connected_client.get_id());
				MQTT_broker_task_queue.enqueue(std::move(task));
			}

			if (qos != 2)
			{
				// store it into MQTT queue
				std::vector<char> packet;
				packet.clear();
				convert_publish_into_packet(&publish, &packet);
				MQTT_task MQ_task(task_count++, 0, packet, "-99");
				MQTT_broker_task_queue.enqueue(std::move(MQ_task));

				// store it into DDS queue
				Broker_task BK_task(task_count, publish.get_vHeader().get_topic_name_str(), publish.get_payload().get_data());
				/*Messenger::Message dds_message;
				auto payload = publish.get_payload().get_data();
				std::stringstream stream_gps(payload);
				boost::property_tree::ptree tree_gps;
				boost::property_tree::read_json(stream_gps, tree_gps);

				dds_message.subject_id = tree_gps.get<int>("Vehicle_GPS.subject_id");
				dds_message.utc = tree_gps.get<long long>("Vehicle_GPS.utc");
				dds_message.posstatus = tree_gps.get<char>("Vehicle_GPS.posstatus");
				dds_message.lat = tree_gps.get<char>("Vehicle_GPS.lat");
				dds_message.lat_dir = tree_gps.get<char>("Vehicle_GPS.lat_dir");
				dds_message.lon = tree_gps.get<double>("Vehicle_GPS.lon");
				dds_message.lon_dir = tree_gps.get<double>("Vehicle_GPS.lon_dir");
				dds_message.speed = tree_gps.get<int>("Vehicle_GPS.speed");
				dds_message.track = tree_gps.get<int>("Vehicle_GPS.track");
				dds_message.date = tree_gps.get<int>("Vehicle_GPS.date");
				dds_message.mag = tree_gps.get<float>("Vehicle_GPS.mag");
				dds_message.mag_dir = tree_gps.get<char>("Vehicle_GPS.mag_dir");
				dds_message.mode = tree_gps.get<char>("Vehicle_GPS.mode");
				dds_message.check = tree_gps.get<int>("Vehicle_GPS.check");
				*/
				DDS_task_queue.enqueue(std::move(BK_task));
			}

			status = "after publish";
		}
	}
	catch (const std::exception& error)
	{
		//printf("[SYSTEM] MQTT_Session :%s\n", error.what());
		std::cout << "[ERROR] MQTT_Session[client" << this->connected_client.get_id() << " has problem.] STATUS[ " + status + "] publish_mqtt_thread[" << error.what() << "]" << std::endl;
		std::ofstream myfile;
		myfile.open("rec_evednt.txt", ios::app);
		myfile << "[ERROR] MQTT_Session[client" << this->connected_client.get_id() << " has problem.] STATUS[ " + status + "] publish_mqtt_thread[" << error.what() << "]" << std::endl;
		myfile.close();
	}
}
