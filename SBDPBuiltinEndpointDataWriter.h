#pragma once
#include <string>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <SESSolutionX/SBDP/SBDPThreadMutex.h>
#include "BrokerClient.h"
class SBDPBuiltinEndpointDataWriter
{
private:
    boost::asio::ip::udp::endpoint endpoint_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::deadline_timer timer_;
    int message_count_;
    std::string message_;
    AltDiscoveredBrokerData SBDP_DATA;
    int* shared_client_num;
public:
	SBDPBuiltinEndpointDataWriter(boost::asio::io_service& io_service,
        const boost::asio::ip::address& multicast_address, bool broker_flag)
        : endpoint_(multicast_address, BROKER_CONFIG::SBDP_MULTICAST_PORT),
        socket_(io_service, endpoint_.protocol()),
        timer_(io_service),
        message_count_(0) 
    {

        socket_.set_option(boost::asio::ip::multicast::join_group(endpoint_.address()));
        //added end
        std::ostringstream os;
        os << "Message " << message_count_++;
        
        SBDP_THREAD::task_mutex_rlock(&SBDP_THREAD::mutex);
        if (!broker_flag) 
        {
            SBDP_DATA.set_broker_id(BROKER_ALT_CONFIG::BROKER_ID);
        }
        SBDP_DATA.set_connected_client_count(BROKER_CONFIGURATION::get_count());
        message_ = SBDP_DATA.to_json();
        SBDP_THREAD::task_mutex_unlock(&SBDP_THREAD::mutex);
        
       
        socket_.async_send_to(
            boost::asio::buffer(message_), endpoint_,
            boost::bind(&SBDPBuiltinEndpointDataWriter::handle_send_to, this,
                boost::asio::placeholders::error));
        
    };

    void handle_send_to(const boost::system::error_code& error)
    {
        if (!error && message_count_ < 50)
        {
            timer_.expires_from_now(boost::posix_time::seconds(2));
            timer_.async_wait(
                boost::bind(&SBDPBuiltinEndpointDataWriter::handle_timeout, this,
                    boost::asio::placeholders::error));
        }
    }

    void handle_timeout(const boost::system::error_code& error)
    {
        if (!error)
        {
            std::ostringstream os;
            os << "Message " << message_count_++;
            SBDP_THREAD::task_mutex_rlock(&SBDP_THREAD::mutex);
            SBDP_DATA.set_connected_client_count(BROKER_CONFIGURATION::get_count());
            message_ = SBDP_DATA.to_json();
            SBDP_THREAD::task_mutex_unlock(&SBDP_THREAD::mutex);

            socket_.async_send_to(
                boost::asio::buffer(message_), endpoint_,
                boost::bind(&SBDPBuiltinEndpointDataWriter::handle_send_to, this,
                    boost::asio::placeholders::error));
        }
    }
};

