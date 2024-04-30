#pragma once
#include "HTTPService.h"
#include <boost\asio\io_context.hpp>
#include <boost\asio.hpp>
#include <boost\property_tree\ptree.hpp>
#include <boost\property_tree\json_parser.hpp>
#include "Broker/BrokerConfigurationSet.h"

using boost::asio::ip::tcp;

class HTTPHandler
{
public:
	void initialize(bool broker_alt_flag);
private:
	HTTPService http_service;
	std::string serverIp = "127.0.0.1";
	std::string serverDomain = "localhost";
	std::string serverPort = "8080";
	boost::asio::io_context io_context;
};

