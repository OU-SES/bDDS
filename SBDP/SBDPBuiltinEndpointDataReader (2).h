#pragma once
#include <string>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <SESSolutionX\Broker\BrokerConfigurationSet.h>
#include "AltDiscoveredBrokerData.h"

class SBPDBuiltinEndpointDataReader
{
private:
	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint sender_endpoint_;
	enum { max_length = 1024 };
	char data_[max_length];
public:
	SBPDBuiltinEndpointDataReader(boost::asio::io_context& io_context,
		const boost::asio::ip::address & listen_address,
		const boost::asio::ip::address & multicast_address)
		: socket_(io_context) 
	{
		boost::asio::ip::udp::endpoint listen_endpoint(
		listen_address, BROKER_CONFIG::SBDP_MULTICAST_PORT);
		socket_.open(listen_endpoint.protocol());
		socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		socket_.bind(listen_endpoint);
		
		// Join the multicast group.
		socket_.set_option(
			boost::asio::ip::multicast::join_group(multicast_address));
		
		socket_.async_receive_from(
			boost::asio::buffer(data_, max_length), sender_endpoint_,
			boost::bind(&SBPDBuiltinEndpointDataReader::handle_receive_from, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	};
	
	void handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd)
	{
		if (!error)
		{
			std::cout << "SBPD reader receieve:";
			std::cout.write(data_, bytes_recvd);
			std::cout << std::endl;

			AltDiscoveredBrokerData *recieved_broker = reinterpret_cast<AltDiscoveredBrokerData *>(data_);
			std::cout << "broker[id:" << recieved_broker->get_broker_id() << "][connected client num:" << recieved_broker->get_connected_client_count() << "]" << std::endl;
			
			socket_.async_receive_from(
				boost::asio::buffer(data_, max_length), sender_endpoint_,
				boost::bind(&SBPDBuiltinEndpointDataReader::handle_receive_from, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}
};

