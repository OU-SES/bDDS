/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdlib.h>

#include "DataReaderListenerImpl.h"
#include "MessengerTypeSupportC.h"
#include "MessengerTypeSupportImpl.h"
#include "Broker/BrokerTaskQueue.h"

#include <iostream>
#include <MQTTPacket/MQTTPublishMessage.h>
#include <MQTTPacket/PacketGenerator.h>
#include <MQTTPacket/MQTTMessageFactory.h>
#include <SESSolutionX\DDSThread\DDSThread.h>
#include <SESSolutionX\BrokerClient.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <MQTTPacket/PacketInterpreter.h>
#include "Broker/BrokerTask.h"
#include <SESSolutionX/BrokerThreadMutex.h>
using namespace std;
int message_count=0;

void
DataReaderListenerImpl::on_requested_deadline_missed(
  DDS::DataReader_ptr /*reader*/,
  const DDS::RequestedDeadlineMissedStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_requested_incompatible_qos(
  DDS::DataReader_ptr /*reader*/,
  const DDS::RequestedIncompatibleQosStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_sample_rejected(
  DDS::DataReader_ptr /*reader*/,
  const DDS::SampleRejectedStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_liveliness_changed(
  DDS::DataReader_ptr /*reader*/,
  const DDS::LivelinessChangedStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_data_available(DDS::DataReader_ptr reader)
{
  Messenger::MessageDataReader_var reader_i =
    Messenger::MessageDataReader::_narrow(reader);

  if (!reader_i) {
    ACE_ERROR((LM_ERROR,
               ACE_TEXT("ERROR: %N:%l: on_data_available() -")
               ACE_TEXT(" _narrow failed!\n")));
    ACE_OS::exit(1);
  }

  Messenger::Message message;
  DDS::SampleInfo info;

  DDS::ReturnCode_t error = reader_i->take_next_sample(message, info);

  if (error == DDS::RETCODE_OK) {
    //std::cout << "SampleInfo.sample_rank = " << info.sample_rank << std::endl;
    //std::cout << "SampleInfo.instance_state = " << info.instance_state << std::endl;
    
    /*if (info.valid_data) {

        //std::cout.precision(17);
        std::cout << "      subject_id = " << message.subject_id << std::endl
            << "        utc = " << message.utc << std::endl
            << "        posstatus = " << message.posstatus << std::endl
            << "        lat = " << message.lat << std::endl
            << "        lat_dir = " << message.lat_dir << std::endl
            << "        lon = " << message.lon << std::endl
            << "        lon_dir = " << message.lon_dir << std::endl
            << "        speed = " << message.speed << std::endl
            << "        track = " << message.track << std::endl
            << "        date = " << message.date << std::endl
            << "        mag = " << message.mag << std::endl
            << "        mag_dir = " << message.mag_dir << std::endl
            << "        mode = " << message.mode << std::endl;
    }*/ 

    boost::property_tree::ptree output;
    output.put("Vehicle_GPS.subject_id", message.subject_id);
    output.put("Vehicle_GPS.utc", message.utc);
    output.put("Vehicle_GPS.posstatus", message.posstatus);
    output.put("Vehicle_GPS.lat", message.lat);
    output.put("Vehicle_GPS.lat_dir", message.lat_dir);
    output.put("Vehicle_GPS.lon", message.lon);
    output.put("Vehicle_GPS.lon_dir", message.lon_dir);
    output.put("Vehicle_GPS.speed", message.speed);
    output.put("Vehicle_GPS.track", message.track);
    output.put("Vehicle_GPS.date", message.date);
    output.put("Vehicle_GPS.mag", message.mag);
    output.put("Vehicle_GPS.mag_dir", message.mag_dir);
    output.put("Vehicle_GPS.mode", message.mode);
    output.put("Vehicle_GPS.check", message.check);
    std::stringstream string_st;
    boost::property_tree::json_parser::write_json(string_st, output);
    
    Broker_task BK_task(task_count, this->topic_name_, string_st.str());
    DDS_task_queue_.enqueue(std::move(BK_task));
    //std::ofstream myfile;you
    //myfile.open("broker_receive_event.txt", ios::app);
    //myfile << string_st.str() << "\n";
    //myfile.close();::task_mutex_wlock();
    /*broker_cache.push(string_st.str());
    {
        auto msg_num = broker_cache.size();
        if (msg_num > 50)
        {
            broker_cache.pop();
        }
    }
    DTHREAD::task_mutex_unlock();
    */

    // Lock free version

    //MQTT_broker_task_queue_.push(string_st.str());
    //create mqtt publish packet
    

    //if (broker_trigger_)
    //{
        MQTTPublishMessage p_message;
        std::vector<char> packet;
        generate_default_publish(p_message, message.subject_id, "GPS", string_st.str(), global_qos);
        convert_publish_into_packet(&p_message, &packet);

        //DTHREAD::task_mutex_rlock();
        Broker_topic topic = BROKER_CONFIGURATION::find_topic("GPS");
        //DTHREAD::task_mutex_unlock();
        
        //MQTT_task task(task_count++, 0, packet, "-99");
        if (topic.get_sub_topic_str() != "null")
        {
            for (const auto& client : topic.mqtt_client_sessions)
            {
                //std::unique_lock<std::mutex> lock(messages_mutex_);
                //BROKER_THREAD::task_mutex_wlock(&BROKER_THREAD::mutex);
                //MQTT_task task(0, packet, client->get_connected_client().get_id());
                MQTT_task task(task_count++, 0, packet, client->get_connected_client().get_id());

                //myfile.open("rec_evednt.txt", ios::app);
                //myfile << "Distribute_Meesage_added_[" << task.get_client_id() << "][task id:" << task.get_task_id() << "][option:" << task.get_task_option() << "][size" << task.get_data().size() << "messaage " << std::endl;

                //myfile << "\n";
                //myfile.close();





                // Previous version
                //message_queue_.push(std::move(task));

                // moodycamel's queue
                MQTT_broker_task_queue_.enqueue(std::move(task));

                //lock.unlock();
                //BROKER_THREAD::task_mutex_unlock(&BROKER_THREAD::mutex);
                //message_cv_.notify_one();
                //message_cv_.notify_all();

            }
        }
        if (retained_message_count >= max_retain_queue_message_count) {
            boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
            exit(0);
        }
    //}
    
    
  } else {
    ACE_ERROR((LM_ERROR,
               ACE_TEXT("ERROR: %N:%l: on_data_available() -")
               ACE_TEXT(" take_next_sample failed!\n")));
  }
  retained_message_count++;
  
}

void
DataReaderListenerImpl::on_subscription_matched(
  DDS::DataReader_ptr /*reader*/,
  const DDS::SubscriptionMatchedStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_sample_lost(
  DDS::DataReader_ptr /*reader*/,
  const DDS::SampleLostStatus& /*status*/)
{
}
