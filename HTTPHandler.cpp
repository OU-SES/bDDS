
#include "HTTPHandler.h"
#include <iostream>
#include <SESSolutionX\Broker\BrokerConfigurationSet.h>

void HTTPHandler::initialize(bool broker_alt_flag) 

{
	tcp::resolver resolver(io_context);
	tcp::socket g_socket(io_context);
	tcp::resolver::query query(serverIp.c_str(), serverPort.c_str());
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

	boost::asio::connect(g_socket.lowest_layer(), endpoint_iterator);
	g_socket.lowest_layer().set_option(tcp::no_delay(true));
	boost::asio::streambuf request;
	std::ostream request_stream(&request);

	//embedded type code should be modified later
	boost::property_tree::ptree root, info, location, lat, log;
	lat.put("", "40.757464633923644");
	log.put("", "-73.98979872897577");
	location.push_back(std::make_pair("",lat));
	location.push_back(std::make_pair("", log));

	if (broker_alt_flag)
	{
		info.put("id", "Broker120");
		//std::cout << "broker_alt_flag:TRUE" << std::endl;
	}
	else 
	{
		info.put("id", "Broker119");
		//std::cout << "broker_alt_flag:FALSE" << std::endl;
	}
	
	//info.put("ipAddress", "10.0.2.2");
	info.put("ipAddress", BROKER_CONFIG::BROKER_IP);
	if (broker_alt_flag) 
	{
		info.put("port", "32889"); //ori_broker : 32888, alt_broker : 32889
		//std::cout << "broker_alt_flag:TRUE" << std::endl;
	}
	else 
	{
		info.put("port", "32888"); //ori_broker : 32888, alt_broker : 32889
		//std::cout << "broker_alt_flag:False" << std::endl;
	}
	info.put("numSat", "4");
	info.put("HDOP", "1.0");
	info.put("altitude", "10.0");
	info.put("height","100.0");
	info.add_child("location", location);
	
	//root.put_child("broker", info);
	std::ostringstream buf;
	//write_json(buf, root);
	write_json(buf, info);
	std::string json = buf.str();

	request_stream << "POST /brokers/ HTTP/1.1\r\n";
	request_stream << "Host:" << serverDomain << ":" << serverPort << "\r\n";
	request_stream << "User-Agent: C/1.0\r\n";
	request_stream << "Content-Type: application/json; charset=utf-8 \r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Content-Length: " << json.length() << "\r\n";
	request_stream << "Connection: close\r\n\r\n";  //NOTE THE Double line feed
	request_stream << json;
	//end code that will be modified

	int a = boost::asio::write(g_socket, request);
	//std::cout << a << " byte sent\n";
	std::vector<char> response(10 * 1024);
	boost::system::error_code ec;
	auto bytes_received = boost::asio::read(g_socket, boost::asio::buffer(response), boost::asio::transfer_all(), ec);
	response.resize(a);

	//std::cout << "Response: " << ec.message() << "\n----------------\n";
	std::cout.write(response.data(), bytes_received) << std::flush;
}
