#include "BrokerPublisher.h"
#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <SESSolutionX/TCPHandler.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::asio::ip::tcp;
using namespace std;
namespace BROKER_PUB {

	bool Broker_Publisher::check_topic(std::string topic_n) {
		return true;
	}

	void BROKER_PUB::Broker_Publisher::work(DDS::DomainParticipant_var participant, DDS::Topic_var topic, moodycamel::BlockingConcurrentQueue<class Broker_task>& DDS_task_queue_) {

		// Create Publisher
		BROKER_PUB::Broker_Publisher::publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT,
				0,
				OpenDDS::DCPS::DEFAULT_STATUS_MASK);


		if (!publisher) {
			cout << "create publisher error" << endl;
		}

		// Create DataWriter
		DDS::DataWriter_var writer =
			publisher->create_datawriter(topic,
				DATAWRITER_QOS_DEFAULT,
				0,
				OpenDDS::DCPS::DEFAULT_STATUS_MASK);

		if (!writer) {
			cout << "create datawriter error" << endl;
		}

		Messenger::MessageDataWriter_var message_writer =
			Messenger::MessageDataWriter::_narrow(writer);

		if (!message_writer) {
			cout << "narrow failed" << endl;
		}

		// Block until Subscriber is available
		DDS::StatusCondition_var condition = writer->get_statuscondition();
		condition->set_enabled_statuses(DDS::PUBLICATION_MATCHED_STATUS);

		DDS::WaitSet_var ws = new DDS::WaitSet;
		ws->attach_condition(condition);

		while (running) {
			DDS::PublicationMatchedStatus matches;
			if (writer->get_publication_matched_status(matches) != ::DDS::RETCODE_OK) {
				cout << "get_publication_matched_status failed!" << endl;
			}

			if (matches.current_count >= 1) {
				cout << "[SYSTEM] DDSb Broker Pub detects subscribers with topic[" << topic->get_name() << "]" << endl;
				break;
			}

			DDS::ConditionSeq conditions;
			DDS::Duration_t timeout = { 60, 0 };
			if (ws->wait(conditions, timeout) != DDS::RETCODE_OK) {
				cout << "wait failed!" << endl;
			}
		}

		ws->detach_condition(condition);

		while(true)
		{
			Broker_task task;
			DDS_task_queue_.wait_dequeue(task);

			Messenger::Message dds_message;
			auto payload = task.get_data();
			std::stringstream stream_gps(payload);
			boost::property_tree::ptree tree_gps;
			boost::property_tree::read_json(stream_gps, tree_gps);

			dds_message.subject_id = tree_gps.get<int>("Vehicle_GPS.subject_id");
			dds_message.utc = tree_gps.get<long long>("Vehicle_GPS.utc");
			dds_message.posstatus = tree_gps.get<char>("Vehicle_GPS.posstatus");
			dds_message.lat = tree_gps.get<double>("Vehicle_GPS.lat");
			dds_message.lat_dir = tree_gps.get<char>("Vehicle_GPS.lat_dir");
			dds_message.lon = tree_gps.get<double>("Vehicle_GPS.lon");
			dds_message.lon_dir = tree_gps.get<char>("Vehicle_GPS.lon_dir");
			dds_message.speed = tree_gps.get<int>("Vehicle_GPS.speed");
			dds_message.track = tree_gps.get<int>("Vehicle_GPS.track");
			dds_message.date = tree_gps.get<int>("Vehicle_GPS.date");
			dds_message.mag = tree_gps.get<float>("Vehicle_GPS.mag");
			dds_message.mag_dir = tree_gps.get<char>("Vehicle_GPS.mag_dir");
			dds_message.mode = tree_gps.get<char>("Vehicle_GPS.mode");
			dds_message.check = tree_gps.get<int>("Vehicle_GPS.check");
			
			DDS::ReturnCode_t error = message_writer->write(dds_message, DDS::HANDLE_NIL);
			if (error != DDS::RETCODE_OK) {
				ACE_ERROR((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" write returned %d!\n"), error));
			}
		}
		
	}

}