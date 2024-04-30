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

#ifndef BROKER_PUB_H
#define BROKER_PUB_H
#include <ace/Log_Msg.h>
#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsPublicationC.h>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include <SESSolutionX/MessengerTypeSupportC.h>
#include <dds/DCPS/StaticIncludes.h>
#include <SESSolutionX/moodycamel/blockingconcurrentqueue.h>
#ifdef ACE_AS_STATIC_LIBS
#  include <dds/DCPS/RTPS/RtpsDiscovery.h>
#  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
#endif

#include "DdsBrokerImpl.h"

//=============================================================================
/**
 * @file    BrokerSubscriber.h
 *
 *
 * This file contains value added Subscriber functions that extend the
 * behavior of the DDS Subscriber.
 *
 *
 * @author Hwimin Kim <hwiminkim@oakland.edu>
 */
 //=============================================================================

#include <fstream>

using namespace boost::container;

namespace BROKER_PUB
{
    class Broker_Publisher{
    public:
        int num_data_writer=0;
        void work(DDS::DomainParticipant_var participant, DDS::Topic_var topic, moodycamel::BlockingConcurrentQueue<class Broker_task>& DDS_task_queue_);

    private:
        boost::container::vector<std::string> broker_topic_list;
        bool check_topic(std::string topic_n);
        DDS::Publisher_var publisher;
    };

}

#endif // !BROKER_PUB_H
