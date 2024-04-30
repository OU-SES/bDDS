/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#ifndef DATAREADER_LISTENER_IMPL_H
#define DATAREADER_LISTENER_IMPL_H

#include <ace/Global_Macros.h>

#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DCPS/LocalObject.h>
#include <dds/DCPS/Definitions.h>
#include <queue>
#include <mutex>
#include <SESSolutionX/Broker/BrokerTask.h>
#include <SESSolutionX/moodycamel/blockingconcurrentqueue.h>

class DataReaderListenerImpl : public virtual OpenDDS::DCPS::LocalObject<DDS::DataReaderListener> {
public:
    std::priority_queue<MQTT_task>& message_queue_;
    std::mutex& messages_mutex_;
    int retained_message_count = 0;
    std::condition_variable& message_cv_;
    std::string topic_name_;
    //boost::lockfree::queue<class MQTT_task>& MQTT_broker_task_queue_;
    moodycamel::BlockingConcurrentQueue<class MQTT_task>& MQTT_broker_task_queue_;
    moodycamel::BlockingConcurrentQueue<class Broker_task>& DDS_task_queue_;

  virtual void on_requested_deadline_missed(
    DDS::DataReader_ptr reader,
    const DDS::RequestedDeadlineMissedStatus& status);

  virtual void on_requested_incompatible_qos(
    DDS::DataReader_ptr reader,
    const DDS::RequestedIncompatibleQosStatus& status);

  virtual void on_sample_rejected(
    DDS::DataReader_ptr reader,
    const DDS::SampleRejectedStatus& status);

  virtual void on_liveliness_changed(
    DDS::DataReader_ptr reader,
    const DDS::LivelinessChangedStatus& status);

  virtual void on_data_available(
    DDS::DataReader_ptr reader);

  virtual void on_subscription_matched(
    DDS::DataReader_ptr reader,
    const DDS::SubscriptionMatchedStatus& status);

  virtual void on_sample_lost(
    DDS::DataReader_ptr reader,
    const DDS::SampleLostStatus& status);
  
  DataReaderListenerImpl(moodycamel::BlockingConcurrentQueue<class MQTT_task>& MQTT_broker_task_queue, moodycamel::BlockingConcurrentQueue<class Broker_task>& DDS_task_queue ,std::priority_queue<MQTT_task>& message_queue, std::mutex& messages_mutex, std::condition_variable& message_cv, std::string topic_name):
      message_queue_(message_queue), messages_mutex_(messages_mutex), message_cv_(message_cv), MQTT_broker_task_queue_(MQTT_broker_task_queue), DDS_task_queue_(DDS_task_queue), topic_name_(topic_name){}
};

static int message_counter = 0;
#endif /* DATAREADER_LISTENER_IMPL_H */
