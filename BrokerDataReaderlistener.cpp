/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdlib.h>

#include "BrokerDataReaderListener.h"
#include "MessengerTypeSupportC.h"
#include "MessengerTypeSupportImpl.h"
#include <fstream>

#include <iostream>


void
BrokerDataReaderListener::on_requested_deadline_missed(
    DDS::DataReader_ptr /*reader*/,
    const DDS::RequestedDeadlineMissedStatus& /*status*/)
{
}

void
BrokerDataReaderListener::on_requested_incompatible_qos(
    DDS::DataReader_ptr /*reader*/,
    const DDS::RequestedIncompatibleQosStatus& /*status*/)
{
}

void
BrokerDataReaderListener::on_sample_rejected(
    DDS::DataReader_ptr /*reader*/,
    const DDS::SampleRejectedStatus& /*status*/)
{
}

void
BrokerDataReaderListener::on_liveliness_changed(
    DDS::DataReader_ptr /*reader*/,
    const DDS::LivelinessChangedStatus& /*status*/)
{
}

void
BrokerDataReaderListener::on_data_available(DDS::DataReader_ptr reader)
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
        std::cout << "SampleInfo.sample_rank = " << info.sample_rank << std::endl;
        std::cout << "SampleInfo.instance_state = " << info.instance_state << std::endl;
        
        if (info.valid_data) {
            std::cout << "      subject_id = " << message.subject_id << std::endl
                << "        vers = " << message.vers << std::endl;

            write_pcd_file("PLC_v1_output",message);
        }

    }
    else {
        ACE_ERROR((LM_ERROR,
            ACE_TEXT("ERROR: %N:%l: on_data_available() -")
            ACE_TEXT(" take_next_sample failed!\n")));
    }
}

void
BrokerDataReaderListener::on_subscription_matched(
    DDS::DataReader_ptr /*reader*/,
    const DDS::SubscriptionMatchedStatus& /*status*/)
{
}

void
BrokerDataReaderListener::on_sample_lost(
    DDS::DataReader_ptr /*reader*/,
    const DDS::SampleLostStatus& /*status*/)
{
}

void BrokerDataReaderListener::write_pcd_file(
    std::string file_name,
    Messenger::Message message)
{
    //Name : file_name + subject_id + .pcd
    std::string temp, array_field = "";
    std::string file_name_with_format = file_name + std::to_string(message.subject_id) + ".pcd";

    std::fstream f(file_name_with_format, std::fstream::out);
    //header
    
    temp = "# .PCD v.7 - Point Cloud Data file format\n";
    f.write(temp.c_str(),temp.size());
    
    //version
    temp = "VERSION " + std::to_string(message.vers) +"\n";
    f.write(temp.c_str(), temp.size());
    
    //fields
    temp = "FIELDS " +
        std::to_string(message.fields[0]) + " " +
        std::to_string(message.fields[1]) + " " +
        std::to_string(message.fields[2]) + "\n";
    f.write(temp.c_str(), temp.size());
    
    //Size
    temp = "SIZE " +
        std::to_string(message.sizes[0]) + " " +
        std::to_string(message.sizes[1]) + " " +
        std::to_string(message.sizes[2]) + "\n";
    f.write(temp.c_str(), temp.size());

    //Type
    temp = "TYPE " +
        std::to_string(message.types[0]) + " " +
        std::to_string(message.types[1]) + " " +
        std::to_string(message.types[2]) + "\n";
    f.write(temp.c_str(), temp.size());

    //Count
    temp = "COUNT " +
        std::to_string(message.counts[0]) + " " +
        std::to_string(message.counts[1]) + " " +
        std::to_string(message.counts[2]) + "\n";
    f.write(temp.c_str(), temp.size());

    //Width
    temp = "WIDTH " + std::to_string(message.width) +"\n";
    f.write(temp.c_str(), temp.size());

    //Height
    temp = "HEIGHT " + std::to_string(message.height) +"\n";
    f.write(temp.c_str(), temp.size());

    //Points
    temp = "POINTS " + std::to_string(message.numOfPoints) +"\n";
    f.write(temp.c_str(), temp.size());

    //Data
    temp = "DATA ascii\n"; //+ message.data.operator const CORBA::Char *;
    f.write(temp.c_str(),temp.size());

    for (int i = 0; i < message.numOfPoints; i++) {
        // point x
        temp = std::to_string(message.points[i].x)+" ";
        f.write(temp.c_str(), temp.size());
        // point y
        temp = std::to_string(message.points[i].y)+" ";
        f.write(temp.c_str(), temp.size());
        // point z
        temp = std::to_string(message.points[i].z)+" ";
        f.write(temp.c_str(), temp.size());
        // point s
        temp = std::to_string(message.points[i].s)+"\n";
        f.write(temp.c_str(), temp.size());
    }
}
