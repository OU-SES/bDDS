/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include <ace/Log_Msg.h>
#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsPublicationC.h>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>

#include <dds/DCPS/StaticIncludes.h>
#ifdef ACE_AS_STATIC_LIBS
#  include <dds/DCPS/RTPS/RtpsDiscovery.h>
#  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
#endif

#include <fstream>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "Broker/BrokerPublisher.h"
#include "Broker/BrokerSubscriber.h"
#include "DDSThread/DDSThread.h"
#include "DataReaderListenerImpl.h"
#include "BrokerDataReaderListener.h"
#include "MessengerTypeSupportImpl.h"
#include <SESSolutionX\HTTPHandler.h>
#include "Broker/BrokerConfigurationSet.h"
#include "SBDP/SBDPBuiltinEndpointDataReader.h"
#include "SBDPBuiltinEndpointDataWriter.h"
#include <SESSolutionX\SBDP\SBDPThreadMutex.h>
#include <SESSolutionX\BrokerClient.h>
#include <SESSolutionX\TCPHandler.h>
#include "Broker/BrokerConfigurationSet.h"
#include "Broker/BrokerTaskQueue.h"
#include <SESSolutionX/Broker/BrokerMQTTPublisher.h>
#include <SESSolutionX/Broker/BrokerMQTTSubscriber.h>
#include <SESSolutionX/moodycamel/blockingconcurrentqueue.h>

//JSON! serialize AlternativeBrokerInfo to Json file. and send it with SBDP
int client_no = 0;
std::vector<Broker_topic> topic_list;
std::vector<AltDiscoveredBrokerData> alt_broker_list;
std::queue <std::string> broker_cache;
int max_retain_queue_message_count;
bool broker_trigger_;
int global_qos=0;
//DDSThread::DDSThreadPool dds_task_pool;
boost::asio::thread_pool *dds_thread_pool = nullptr;
std::string log_name;
int task_count=0;

int testRecv(TCPHandler test_server) {
	char recv_buf[512] = {};
	test_server.recv_data(recv_buf, 512);
	running = false;
	//std::cout << "Test end" << std::endl;
	return 0;
}


int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{

	try {
		//std::cout << "thred:" << id << "is running...." << std::endl;
		HTTPHandler http_handler;
		int broker_trigger = std::stoi(argv[3]);
		max_retain_queue_message_count = std::stod(argv[4]);
		global_qos = std::stod(argv[5]);
		dds_thread_pool = new boost::asio::thread_pool(std::stoi(argv[6]));
		std::string recv_topic_name = argv[7];	// GPS as default
		std::string send_topic_name = argv[8];	// Output
		bool broker_alt_flag = false;
		if (broker_trigger)
		{
			broker_alt_flag = false;
			broker_trigger_ = true;
			std::cout << "Broker start" << std::endl;
			log_name = "broker_log.txt";
		}
		else
		{
			broker_alt_flag = true;
			std::cout << "Broker Alt start" << std::endl;
			log_name = "broker_alt_log.txt";
		}

		//http_handler.initialize(broker_alt_flag);

		const char* serverIp = BROKER_CONFIG::BROKER_IP.c_str();
		short serverPort;
		if (broker_trigger)
		{
			serverPort = 32888;
		}
		else if (broker_trigger == 2)
		{
			serverPort = 32900;
		}
		else
		{
			serverPort = 32889;
		}

		//boost::asio::io_context io_context;
		//boost::asio::ip::tcp::socket socket(io_context);
		//BROKERSUB::Broker_Subscriber subscriber_b(io_context, serverPort);

		//set lock for thread task
		DTHREAD::task_lock_init();
		SBDP_THREAD::task_lock_init();
		//BROKER_THREAD::task_lock_init();
		//run broker subscriber
		//boost::thread subscriber_thrd(boost::bind(&BROKERSUB::Broker_Subscriber::start_accept, &subscriber_b));

		boost::asio::io_service io_service;
		boost::asio::io_service::work work(io_service);

		std::priority_queue<MQTT_task> mqtt_task_queue;
		std::mutex queue_mutex;

		std::set<std::shared_ptr<class MQTTSession>> mqtt_clients;
		//boost::lockfree::queue<class MQTT_task> MQTT_broker_task_queue;
		moodycamel::BlockingConcurrentQueue<class MQTT_task> MQTT_broker_task_queue;
		moodycamel::BlockingConcurrentQueue<class Broker_task> DDS_message_queue;
		
		std::mutex mqtt_client_mutex;

		std::condition_variable message_cv;
		
		BrokerMQTTSubscriber mqtt_subscriber(io_service, serverPort, MQTT_broker_task_queue, DDS_message_queue, queue_mutex, mqtt_clients, mqtt_client_mutex, message_cv);
		BrokerMQTTPublisher mqtt_publisher(mqtt_task_queue, MQTT_broker_task_queue, queue_mutex, mqtt_clients, mqtt_client_mutex, std::stoi(argv[6]), message_cv);
		const int NUM_ENDPOINTREADER = std::stoi(argv[6]);
		std::vector<std::thread> subscribe_endpoint_writer_pool;
		for (int i = 0; i < NUM_ENDPOINTREADER; i++) 
		{
			subscribe_endpoint_writer_pool.push_back(std::thread([&io_service] { io_service.run(); }));
		}

		boost::asio::io_service read_io;
		boost::asio::io_service write_io;
		std::string broker_id;
		if (broker_trigger)
		{
			broker_id = BROKER_CONFIG::BROKER_ID;
		}
		else
		{
			broker_id = BROKER_ALT_CONFIG::BROKER_ID;
		}
		SBDPBuiltinEndpointDataReader SBDP_reader(
			read_io,
			boost::asio::ip::make_address("0.0.0.0"),
			boost::asio::ip::make_address(BROKER_CONFIG::SBDP_MULTICAST_IP),
			broker_id
		);
		SBDPBuiltinEndpointDataWriter SBDP_writer(
			write_io,
			boost::asio::ip::make_address(BROKER_CONFIG::SBDP_MULTICAST_IP),
			broker_trigger
		);
		std::thread SBDPreader_thrd([&] { read_io.run(); });
		std::thread SBDPwriter_thrd([&] { write_io.run(); });

		//thrd1.join();
		// Initialize DomainParticipantFactory

		DDS::DomainParticipantFactory_var dpf =
			TheParticipantFactoryWithArgs(argc, argv);

		// Create DomainParticipant
		DDS::DomainParticipant_var participant =

			dpf->create_participant(42,
				PARTICIPANT_QOS_DEFAULT,
				0,
				OpenDDS::DCPS::DEFAULT_STATUS_MASK);

		if (!participant) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" create_participant failed!\n")),
				1);
		}

		// Register TypeSupport (Messenger::Message)
		Messenger::MessageTypeSupport_var ts =
			new Messenger::MessageTypeSupportImpl;

		if (ts->register_type(participant, "") != DDS::RETCODE_OK) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" register_type failed!\n")),
				1);
		}

		// Create Topic (Vehicle to client)
		CORBA::String_var type_name = ts->get_type_name();
		DDS::Topic_var sub_topic =
			//participant->create_topic("GPS",
			participant->create_topic(recv_topic_name.c_str(),
				type_name,
				TOPIC_QOS_DEFAULT,
				0,
				OpenDDS::DCPS::DEFAULT_STATUS_MASK);

		DDS::Topic_var pub_topic =
			//participant->create_topic("GPS",
			participant->create_topic(send_topic_name.c_str(),
				type_name,
				TOPIC_QOS_DEFAULT,
				0,
				OpenDDS::DCPS::DEFAULT_STATUS_MASK);

		if (!sub_topic) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" sub_create_topic failed!\n")),
				1);
		}

		if (!pub_topic) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" pub_create_topic failed!\n")),
				1);
		}

		// Create Subscriber
		DDS::Subscriber_var subscriber =
			participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT,
				0,
				OpenDDS::DCPS::DEFAULT_STATUS_MASK);

		if (!subscriber) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" create_subscriber failed!\n")),
				1);
		}

		BROKER_PUB::Broker_Publisher publisher = BROKER_PUB::Broker_Publisher();
		//std::thread publisher_thd(&BROKER_PUB::Broker_Publisher::work, &publisher, std::ref(participant), pub_topic, DDS_message_queue);
		std::thread publisher_thd([&publisher, &participant, &pub_topic, &DDS_message_queue]() {
			publisher.work(participant, pub_topic, DDS_message_queue);
		});

		// Create DataReader
		//DDS::DataReaderListener_var listener(new DataReaderListenerImpl);

		DDS::DataReaderListener_var listener(new DataReaderListenerImpl(MQTT_broker_task_queue, DDS_message_queue, mqtt_task_queue, queue_mutex, message_cv, send_topic_name));

		DDS::DataReaderQos reader_qos;
		subscriber->get_default_datareader_qos(reader_qos);
		reader_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;

		DDS::DataReader_var reader =
			subscriber->create_datareader(sub_topic,
				reader_qos,
				listener,
				OpenDDS::DCPS::DEFAULT_STATUS_MASK);

		if (!reader) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" create_datareader failed!\n")),
				1);
		}

		Messenger::MessageDataReader_var reader_i =
			Messenger::MessageDataReader::_narrow(reader);

		if (!reader_i) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" _narrow failed!\n")),
				1);
		}

		// Block until Publisher completes
		DDS::StatusCondition_var condition = reader->get_statuscondition();
		condition->set_enabled_statuses(DDS::SUBSCRIPTION_MATCHED_STATUS);

		DDS::WaitSet_var ws = new DDS::WaitSet;
		ws->attach_condition(condition);

		//TCPHandler simulator_client("broker_alt");
		//simulator_client.connect_server("192.168.0.220", 7777);
		//simulator_client.connect_server("10.0.0.91", 7777);

		while (running) {
			DDS::SubscriptionMatchedStatus matches;
			if (reader->get_subscription_matched_status(matches) != DDS::RETCODE_OK) {
				ACE_ERROR_RETURN((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" get_subscription_matched_status failed!\n")),
					1);
			}

			if (matches.current_count == 0 && matches.total_count > 0) {
				break;
			}

			//reader->get_matched_publication_data();
			DDS::ConditionSeq conditions;
			DDS::Duration_t timeout = { 180, 0 };
			if (ws->wait(conditions, timeout) != DDS::RETCODE_OK) {
				ACE_ERROR_RETURN((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" wait failed!\n")),
					1);
			}
		}

		for (auto& thread : subscribe_endpoint_writer_pool) {
			thread.join();
		}

		while (true) {
			std::string cc;
			std::cin >> cc;
			if (cc == "exit") break;
		}


		// Clean-up!
		participant->delete_contained_entities();
		dpf->delete_participant(participant);

		TheServiceParticipant->shutdown();

	}
	catch (const CORBA::Exception& e) {
		e._tao_print_exception("Exception caught in main():");
		return 1;
	}


	return 0;
}

