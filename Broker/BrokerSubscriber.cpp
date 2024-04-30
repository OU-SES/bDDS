#include "BrokerSubscriber.h"
#include "MQTTPacket/PacketInterpreter.h"
#include <boost/thread.hpp>

#include "MQTTPacket/MQTTMessageFactory.h"
#include "MQTTPacket/PacketGenerator.h"
#include <SESSolutionX\BrokerClient.h>
#include <SESSolutionX\DDSThread\DDSThread.h>
#include <SESSolutionX\SBDP\SBDPThreadMutex.h>
#include "../SBDP/AlternativeServerInfo.h"

boost::asio::thread_pool subscribe_pool(10);

using namespace std;
namespace BROKERSUB {
	void Broker_Subscriber::start_accept()
	{
		Session::pointer new_connection =
			Session::create(io_context_);
		
		acceptor_.async_accept(new_connection->socket(),
			boost::bind(&Broker_Subscriber::handle_accept, this, new_connection,
				boost::asio::placeholders::error));

		Broker_Subscriber::io_context_.run();
	}

	void Broker_Subscriber::handle_accept(Session::pointer new_sessions, const boost::system::error_code& error)
	{
		//if (!error) new_sessions->start();
		if (!error) boost::thread subscriber_thrd(boost::bind(&BROKERSUB::Session::start, new_sessions));
		//if (!error) boost::asio::post(subscribe_pool, boost::bind(&BROKERSUB::Session::start, new_sessions));
		Broker_Subscriber::start_accept();
	}

	bool Broker_Subscriber::create_topic() {
		return true;
	}

	DDS_BROKER::DataBrokerReader_ptr create_broker_reader(){
		return NULL;
	}

	void Session::do_read() {
		//cout << "Client:";
		
		//auto self(shared_from_this());
		
		/*
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
			[this, self](boost::system::error_code ec, std::size_t length) {
				bool send_flag = true;
				if (!ec) {
					std::vector<char> packet;
					cout << "Packet received: length[" << length << "]" << endl;
					int first_byte_fheader = data_[0];
					int message_type = (first_byte_fheader >> 4) & 0x0f;
					if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_CONNECT) {
						MQTTConnectMessage temp;
						interpret_connect_packet(&temp, data_);

						//update connected client information
						connected_client.set_protocol(temp.get_vHeader().get_protocol_name().get_data() +
							std::to_string(temp.get_vHeader().get_protocol_version()));
						connected_client.set_flags(temp.get_vHeader().get_connect_flags());
						connected_client.set_keep_alive(temp.get_vHeader().get_keep_alive().get_data());
						connected_client.set_id(temp.get_payload().get_id().get_data());
						connected_client.set_password(temp.get_payload().get_password().get_data());
						this->name = temp.get_payload().get_id().get_data();
						
						//modify configuration value including the number of connected clients
						//// configuration part
						SBDP_THREAD::task_mutex_wlock(&SBDP_THREAD::mutex);						
						BROKER_CONFIGURATION::increase_count();
						client_n = BROKER_CONFIGURATION::get_count();
						//// get broker
						//Locator broker_address = AlternativeServerInfo::get_broker(connected_client);
						//// convert Locator into JSON string
						boost::property_tree::ptree output;
						//output.put("BROKER.Ip", broker_address.ip);
						//output.put("BROKER.Port", broker_address.port);
						//output.put("BROKER.Ip", "192.168.0.117");
						output.put("BROKER.Ip", "192.168.0.200");
						output.put("BROKER.Port", "32889");
						std::stringstream broker_locator_str;
						boost::property_tree::json_parser::write_json(broker_locator_str, output);

						SBDP_THREAD::task_mutex_unlock(&SBDP_THREAD::mutex);

						MQTTConnackMessage connack;
						generate_default_connack(&connack, "Client_" + client_n, "ref", broker_locator_str.str());
						convert_connact_into_packet(&connack, &packet);
						packet.push_back('\n');
					}
					else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE)
					{
						MQTTSubscribeMessage temp;
						interpret_subscribe_packet(&temp, data_);
						std::vector<Topic> subscription_list = temp.get_payload().get_list();

						Topic data = temp.get_payload().get_list().at(0);
						int qos = temp.get_payload().get_list().at(0).get_maximum_qos();

						DTHREAD::task_mutex_wlock();
						//BROKER_CONFIGURATION::find_topic(data.filter.get_data()).add_client(this);
						BROKER_CONFIGURATION::add_client_to_topic(data.filter.get_data(), this);
						DTHREAD::task_mutex_unlock();

						MQTTSubackMessage suback;
						generate_default_suback(&suback, client_n, 1);
						convert_suback_into_packet(&suback, &packet);

						just_do_sync_write(packet.data(), packet.size());
						
						if (!broker_cache.empty())
						{
							DTHREAD::task_mutex_rlock();
							std::queue <std::string> retain_queue(broker_cache);
							DTHREAD::task_mutex_unlock();

							cout << "[SYSTEM] there are [" << retain_queue.size() << "] messages in the queue" << endl;

							while (!retain_queue.empty())
							{
								
								MQTTPublishMessage p_message;
								std::vector<char> packet;
								std::string retain_message = retain_queue.front();
								cout << "[SYSTEM] retained queue [" << retain_message << "] message in the queue" << endl;
								retain_queue.pop();
								generate_default_publish(&p_message, temp.get_vHeader().get_packet_identifier().get_data(), "GPS", retain_message);
								convert_publish_into_packet(&p_message, &packet);
								packet.push_back('\n');

								just_do_sync_write(packet.data(), packet.size());
								if (qos == 1)
								{
									socket_.async_read_some(boost::asio::buffer(data_, max_length),
										[this, self](boost::system::error_code ec, std::size_t length) {
											bool send_flag = true;
											if (!ec) {
												std::vector<char> packet;
												cout << "Packet received: length[" << length << "]" << endl;
												int first_byte_fheader = data_[0];
												int message_type = (first_byte_fheader >> 4) & 0x0f;
												if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBACK)
												{
													MQTTPubackMessage puback;
													interpret_puback_packet(&puback, data_);
													cout << "puback received..." << endl;
													send_flag = false;
												}
											}

										}
									);
								}
								else if (qos == 2) 
								{
									socket_.async_read_some(boost::asio::buffer(data_, max_length),
										[this, self](boost::system::error_code ec, std::size_t length) {
											bool send_flag = true;
											if (!ec) {
												std::vector<char> packet;
												cout << "Packet received: length[" << length << "]" << endl;
												int first_byte_fheader = data_[0];
												int message_type = (first_byte_fheader >> 4) & 0x0f;
												if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBREC)
												{
													MQTTPubrecMessage pubrec;
													interpret_pubrec_packet(&pubrec, data_);
													cout << "pubrec received..." << endl;

													MQTTPubrelMessage pubrel;
													generate_default_pubrel(&pubrel, client_n);
													convert_pubrel_into_packet(&pubrel, &packet);
												}
												just_do_write(packet.data(), packet.size());
											}
										}
									);
									
									socket_.async_read_some(boost::asio::buffer(data_, max_length),
										[this, self](boost::system::error_code ec, std::size_t length) {
											bool send_flag = true;
											if (!ec) {
												std::vector<char> packet;
												cout << "Packet received: length[" << length << "]" << endl;
												int first_byte_fheader = data_[0];
												int message_type = (first_byte_fheader >> 4) & 0x0f;
												if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBREC)
												{
													MQTTPubcompMessage pubcomp;
													interpret_pubcomp_packet(&pubcomp, data_);
													cout << "pubcomp received..." << endl;
												}
											}
										}
									);
								}
							}
						}
						send_flag = false;
					}
					else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBACK) 
					{
						MQTTPubackMessage puback;
						interpret_puback_packet(&puback, data_);
						cout << "puback received..." << endl;
						send_flag = false;
					}
					else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBREC) 
					{
						MQTTPubrecMessage pubrec;
						interpret_pubrec_packet(&pubrec, data_);
						cout << "pubrec received..." << endl;

						MQTTPubrelMessage pubrel;
						generate_default_pubrel(&pubrel, client_n);
						convert_pubrel_into_packet(&pubrel, &packet);
					}
					else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBCOMP)
					{
						MQTTPubcompMessage pubcomp;
						interpret_pubcomp_packet(&pubcomp, data_);
						cout << "pubcomp received..." << endl;
						send_flag = false;
					}

					//if (send_flag) do_write(packet.data(), packet.size());
					if (send_flag) just_do_sync_write(packet.data(), packet.size());
					if (!send_flag) do_read();
					
				}
			});*/
		//auto length = socket_.read_some(boost::asio::buffer(data_, max_length));
		while (true)
		{
			std::vector<char> packet;
			boost_sync_read(&packet);
			boost::thread message_thrd(boost::bind(&Session::message_process, this, packet));
			/*dds_task_pool.enqueue([this, &packet]() {
				Session::message_process(packet);
			});*/
		}
		//std::vector<char> packet;
		//socket_.async_read_some(boost::asio::buffer(packet, 1024), boost::bind(&Session::message_process, this, packet));
		//std::vector<char> packet;
		//boost_sync_read(&packet);
		//boost::thread message_thrd(boost::bind(&Session::message_process, this, packet));
		//do_read();
	}

	void Session::do_write(const char packet[], int size) {
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(packet, size),
			[this, self](boost::system::error_code ec, std::size_t) {
				if (!ec) {
					do_read();
				}
			});
	}

	void Session::just_do_write(const char packet[], int size) {
		auto self(shared_from_this());
		//boost::asio::write(socket_, boost::asio::buffer(packet, size));
		boost::asio::async_write(socket_, boost::asio::buffer(packet, size),
			[this, self](boost::system::error_code ec, std::size_t) {
				if (!ec) {
				}
			});
	}

	void Session::just_do_sync_write(const char packet[], int size) {
		auto self(shared_from_this());
		boost::asio::write(socket_, boost::asio::buffer(packet, size));
	}

	void Session::boost_sync_read(std::vector<char> *packet)
	{
		//boost::asio::read(socket_, boost::asio::buffer(&recv_size, sizeof(recv_size)));
		std::vector<char> buffer(1024);
		boost::asio::read(socket_, boost::asio::buffer(buffer));
		//cout << "Packet received: length[" << buffer.size() << "]" << endl;
		
		packet->insert(packet->end(), std::make_move_iterator(buffer.begin()), std::make_move_iterator(buffer.end()));

		return;
	}
	
	void Session::boost_sync_write(const char packet[], int size)
	{
		//boost::asio::write(socket_, boost::asio::buffer(&size, sizeof(size)));
		boost::asio::write(socket_, boost::asio::buffer(packet, 1024));
		
		//cout << "Packet sent[" << this->get_client_no() << "]: length[" << size << "]" << endl;
		//cout << "packet:" << packet << endl;
		return;
	}

	void Session::do_sync_write(std::vector<char> packet, int size)
	{
		//boost_sync_write(packet, size);
		//boost::asio::write(socket_, boost::asio::buffer(&size, sizeof(size)));
		//cout << "Packet sent[" << this->get_client_no() << "]: length[" << size << "]" << endl;
		//cout << "packet:" << packet.data() << endl;
		boost::asio::write(socket_, boost::asio::buffer(packet.data(), 1024));
		return;
	}

	void Session::message_process(std::vector<char> packet) 
	{
		bool send_flag_ = false;;
		recv_data = packet.data();
		int first_byte_fheader = recv_data[0];
		const int message_type = (first_byte_fheader >> 4) & 0x0f;
		if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_CONNECT) {
			MQTTConnectMessage temp;
			interpret_connect_packet(&temp, recv_data);

			//update connected client information
			connected_client.set_protocol(temp.get_vHeader().get_protocol_name().get_data() +
				std::to_string(temp.get_vHeader().get_protocol_version()));
			connected_client.set_flags(temp.get_vHeader().get_connect_flags());
			connected_client.set_keep_alive(temp.get_vHeader().get_keep_alive().get_data());
			connected_client.set_id(temp.get_payload().get_id().get_data());
			connected_client.set_password(temp.get_payload().get_password().get_data());
			this->name = temp.get_payload().get_id().get_data();

			//modify configuration value including the number of connected clients
			//// configuration part
			SBDP_THREAD::task_mutex_wlock(&SBDP_THREAD::mutex);
			BROKER_CONFIGURATION::increase_count();
			client_n = BROKER_CONFIGURATION::get_count();
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

			SBDP_THREAD::task_mutex_unlock(&SBDP_THREAD::mutex);

			MQTTConnackMessage connack;
			generate_default_connack(connack, "Client_" + client_n, "ref", broker_locator_str.str());
			packet.clear();
			convert_connact_into_packet(&connack, &packet);
			send_flag = true;
		}
		else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE)
		{
			MQTTSubscribeMessage temp;
			interpret_subscribe_packet(&temp, recv_data);
			std::vector<Topic> subscription_list = temp.get_payload().get_list();

			Topic data = temp.get_payload().get_list().at(0);
			int qos = temp.get_payload().get_list().at(0).get_maximum_qos();

			DTHREAD::task_mutex_wlock();
			
			BROKER_CONFIGURATION::add_client_to_topic(data.filter.get_data(), this);
			DTHREAD::task_mutex_unlock();

			MQTTSubackMessage suback;
			generate_default_suback(suback, client_n, 1);
			convert_suback_into_packet(&suback, &packet);

			boost_sync_write(packet.data(), packet.size());
			//cout << "[SYSTEM] client comming [" << endl;
			/*if (!broker_cache.empty())
			{
				DTHREAD::task_mutex_rlock();
				std::queue <std::string> retain_queue(broker_cache);
				DTHREAD::task_mutex_unlock();

				//cout << "[SYSTEM] there are [" << retain_queue.size() << "] messages in the queue" << endl;

				while (!retain_queue.empty())
				{
					//boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
					MQTTPublishMessage p_message;
					std::vector<char> packet_;
					std::string retain_message = retain_queue.front();
					//cout << "[SYSTEM] retained queue [" << retain_message << "] message in the queue" << endl;
					retain_queue.pop();
					std::stringstream stream_gps(retain_message);
					boost::property_tree::ptree tree_gps;
					boost::property_tree::read_json(stream_gps, tree_gps);
					std::string subject_id = tree_gps.get<std::string>("Vehicle_GPS.subject_id");

					generate_default_publish(&p_message, stoi(subject_id), "GPS", retain_message, global_qos);
					convert_publish_into_packet(&p_message, &packet_);
					//packet_.push_back('\n');

					boost_sync_write(packet_.data(), packet_.size());
					
					/*if (qos == 1)
					{
						if (!packet.empty()) packet.clear();
						boost_sync_read(&packet);
						recv_data = packet.data();
						first_byte_fheader = recv_data[0];
						const int puback_message_type_ = (first_byte_fheader >> 4) & 0x0f;
						//if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBACK) cout << "puback received.." << endl;
					}
					else if (qos == 2)
					{
						if (!packet.empty()) packet.clear();
						boost_sync_read(&packet);
						recv_data = packet.data();
						first_byte_fheader = recv_data[0];
						const int pubrec_message_type_ = (first_byte_fheader >> 4) & 0x0f;
						//if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBREC) cout << "pubrec received.." << endl;

						MQTTPubrelMessage pubrel;
						generate_default_pubrel(&pubrel, client_n);
						if (!packet.empty()) packet.clear();
						convert_pubrel_into_packet(&pubrel, &packet);
						boost_sync_write(packet.data(), packet.size());

						if (!packet.empty()) packet.clear();
						boost_sync_read(&packet);
						recv_data = packet.data();
						first_byte_fheader = recv_data[0];
						const int pubcomp_message_type_ = (first_byte_fheader >> 4) & 0x0f;
						//if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBCOMP) cout << "pubcomp received.." << endl;
					}
				}
			}*/
			send_flag = false;
		}
		else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBACK)
		{
			MQTTPubackMessage puback;
			interpret_puback_packet(&puback, recv_data);
			//cout << "puback received..." << endl;
			send_flag = false;
		}
		else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBREC)
		{
			MQTTPubrecMessage pubrec;
			interpret_pubrec_packet(&pubrec, recv_data);
			TwoByteInteger packet_id = pubrec.get_vHeader().get_packet_identifier();
			//cout << "pubrec received..." << packet_id.get_data() << endl;

			MQTTPubrelMessage pubrel;
			generate_default_pubrel(pubrel, packet_id.get_data());
			packet.clear();
			convert_pubrel_into_packet(&pubrel, &packet);

			MQTTPubrelMessage pubrel_r;
			interpret_pubrel_packet(&pubrel_r, packet.data());
			//cout << "pubrel sent...." << pubrel_r.get_vHeader().get_packet_identifier().get_data() << endl;
			send_flag = true;
		}
		else if (message_type == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBCOMP)
		{
			MQTTPubcompMessage pubcomp;
			interpret_pubcomp_packet(&pubcomp, recv_data);
			//cout << "pubcomp received..." << pubcomp.get_vHeader().get_packet_identifier().get_data() << endl;
			send_flag = false;
		}

		//if (send_flag) do_write(packet.data(), packet.size());
		if (send_flag) boost_sync_write(packet.data(), packet.size());
	}
}