#pragma once
#include <string>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <SESSolutionX\Broker\BrokerConfigurationSet.h>
#include "AltDiscoveredBrokerData.h"
#include "AlternativeServerInfo.h"
#include "SBDPThreadMutex.h"
#include <iostream>

class SBDPBuiltinEndpointDataReader
{
private:
	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint sender_endpoint_;
	std::string broker_id;
	std::vector<AltDiscoveredBrokerData> local_storage;
	int temp_output;
	enum { max_length = 1024 };
	char data_[max_length];

	int find_broker(std::string b_id) 
	{
		int i = 0;
		if (local_storage.size() > 0)
		{
			for (AltDiscoveredBrokerData temp : local_storage)
			{
				if (b_id.compare(temp.get_broker_id()) == 0) return i;
				i++;
			}
		}
		return -1;
	}

	void add_broker(AltDiscoveredBrokerData alt_broker) 
	{
		local_storage.push_back(alt_broker);
	}

	void delete_broker(int i) 
	{
		local_storage.erase(local_storage.begin()+i);
	}

public:
	SBDPBuiltinEndpointDataReader(boost::asio::io_context& io_context,
		const boost::asio::ip::address & listen_address,
		const boost::asio::ip::address & multicast_address,
		std::string _broker_id)
		: socket_(io_context) , broker_id(_broker_id)
	{
		boost::asio::ip::udp::endpoint listen_endpoint(
		listen_address, BROKER_CONFIG::SBDP_MULTICAST_PORT);
		socket_.open(listen_endpoint.protocol());
		socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		socket_.bind(listen_endpoint);
		
		// Join the multicast group.
		socket_.set_option(boost::asio::ip::multicast::join_group(multicast_address));
		
		socket_.async_receive_from(
			boost::asio::buffer(data_, max_length), sender_endpoint_,
			boost::bind(&SBDPBuiltinEndpointDataReader::handle_receive_from, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	};
	
	void handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd)
	{
		if (!error)
		{
			//std::
			//  "SBDP reader receieve:";
			//std::cout.write(data_, bytes_recvd);
			data_[bytes_recvd] = '\0';
			boost::property_tree::ptree pt;
			
			std::stringstream str_stream;
			str_stream << data_;
			boost::property_tree::read_json(str_stream, pt);

			std::string broker_id = pt.get<std::string>("BROKER.BrokerID");

			

			if (broker_id.compare(BROKER_ID)!=0) {
				//std::cout << "received broker[id:" << broker_id << "]" << std::endl;
				//std::cout << "This broker[id:" << broker_id << "]" << std::endl;
				temp_output = find_broker(broker_id);
				if (temp_output == -1)
				{
					//std::cout << "received broker[id:" << broker_id << "]" << std::endl;
					//std::cout << "SBDP reader receieve:";
					std::cout.write(data_, bytes_recvd);
					int broker_expected_client_num = std::stoi(pt.get<std::string>("BROKER.ExpectedClientNum"));
					int broker_connected_client_num = std::stoi(pt.get<std::string>("BROKER.ConnectedClientNum"));
					int broker_maximum_client_num = std::stoi(pt.get<std::string>("BROKER.MaximumClientNum"));
					boost::property_tree::ptree::iterator protocols_i = pt.get_child("BROKER.Protocols").begin();
					boost::property_tree::ptree protocols = protocols_i->second;
					std::string broker_protocol = protocols.get<std::string>("");
					std::string broker_ip = pt.get<std::string>("BROKER.Locators.ip");
					int broker_port = std::stoi(pt.get<std::string>("BROKER.Locators.port"));

					AltDiscoveredBrokerData recieved_broker(broker_id, broker_expected_client_num, broker_connected_client_num, broker_maximum_client_num, broker_protocol, broker_ip, broker_port);

					add_broker(recieved_broker);
					SBDP_THREAD::task_mutex_wlock(&SBDP_THREAD::mutex);
					AlternativeServerInfo::add_broker(recieved_broker);
					SBDP_THREAD::task_mutex_unlock(&SBDP_THREAD::mutex);
				}
			}
			socket_.async_receive_from(
				boost::asio::buffer(data_, max_length), sender_endpoint_,
				boost::bind(&SBDPBuiltinEndpointDataReader::handle_receive_from, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}


};

