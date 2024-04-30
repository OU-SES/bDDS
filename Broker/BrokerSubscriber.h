#pragma once
//=============================================================================
/**
 * @file    BrokerPublisher.h
 *
 *
 * This file contains value added Publisher functions that extend the
 * behavior of the DDS Publisher.
 *
 *
 * @author Hwimin Kim <hwiminkim@oakland.edu>
 */
 //=============================================================================

#ifndef BROKER_SUB_H
#define BROKER_SUB_H


#include "DdsBrokerImpl.h"
#include <fstream>
#include "boost/asio.hpp"
#include <iostream>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind/bind.hpp>
#include "BrokerConfigurationSet.h"
#include "../SBDP/ConnectedClient.h"
#include "BrokerTaskQueue.h"
#include <boost/thread.hpp>
using boost::asio::ip::tcp;

namespace BROKERSUB
{
	class Session
		: public boost::enable_shared_from_this<Session> {
	public:
		typedef boost::shared_ptr<Session> pointer;
		Session(tcp::socket socket)
			: socket_(std::move(socket)){}

		void start() {
			//std::cout << "Client is comming... address = :[IP:" << socket_.remote_endpoint().address().to_string()
			//	<< ",PORT:" << socket_.remote_endpoint().port() << "]" << std::endl;
			do_read();
		}

		static pointer create(boost::asio::io_context& io_context)
		{
			return pointer(new Session(io_context));
		}

        tcp::socket& socket()
        {
            return socket_;
        }

		int get_client_no() { return client_n; }
		void set_client_no(int no) { client_n = no; }
		void do_read();
		void do_write(const char packet[], int size);
		void just_do_write(const char packet[], int size);
		void just_do_sync_write(const char packet[], int size);
		void do_sync_write(std::vector<char> packet, int size);
		void message_process(std::vector<char> packet);
		std::string name;
		std::size_t recv_size;
	private:
		Session(boost::asio::io_context& io_context)
			: socket_(io_context)
		{
			//std::cout << "Session is created...." << std::endl;
		}

		void handle_write(const boost::system::error_code& /*error*/,
			size_t /*bytes_transferred*/)
		{
		}

		void boost_sync_read(std::vector<char>* packet);
		void boost_sync_write(const char packet[], int size);

		tcp::socket socket_;
		enum { max_length = 1024 };
		char data_[max_length];
		boost::asio::streambuf data_recv;
		int client_n=0;
		ConnectedClient connected_client;
		const char* recv_data;
		bool send_flag;
	};

	class Broker_Subscriber {
	public:

		Broker_Subscriber(boost::asio::io_context& io_context, short port)
			: io_context_(io_context),
			acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
		{
			//start_accept();
		}

		void service_session();
		void set_messenger_type();
		bool create_topic();
		DDS_BROKER::DataBrokerReader_ptr create_broker_reader();

		void start_accept();
	private:
		void handle_accept(Session::pointer new_sessions, const boost::system::error_code& error);
		void get_broker(Session::pointer new_sessions, const boost::system::error_code& error);
		int num_data_reader = 0;
		//tcp::socket socket_;
		//const char* serverIp = "127.0.0.1";
		const short serverPort = 32888;

		boost::asio::io_context& io_context_;
		//boost::asio::ip::tcp::socket socket_;
		tcp::acceptor acceptor_;
		

	};
}

#endif // !BROKER_SUB_H
