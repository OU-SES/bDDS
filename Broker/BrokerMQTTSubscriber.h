#include <boost/asio.hpp>
#include <queue>
#include "MQTTSession.h"

#include <SESSolutionX/Broker/BrokerTask.h>
#include <SESSolutionX/moodycamel/blockingconcurrentqueue.h>
#include <boost/lockfree/queue.hpp>
#include <SESSolutionX/MessengerC.h>

class BrokerMQTTSubscriber {
public:
    BrokerMQTTSubscriber (boost::asio::io_service& io_service, short port, moodycamel::BlockingConcurrentQueue<class MQTT_task>& MQTT_broker_task_queue, moodycamel::BlockingConcurrentQueue<class Broker_task>& DDS_message_queue, std::mutex& messages_mutex, std::set<std::shared_ptr<class MQTTSession>>& mqtt_clients, std::mutex& mqtt_clients_mutex, std::condition_variable& message_cv)
        : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), socket_(io_service), MQTT_broker_task_queue(MQTT_broker_task_queue), DDS_message_queue(DDS_message_queue), messages_mutex_(messages_mutex), mqtt_clients_(mqtt_clients), mqtt_clients_mutex_(mqtt_clients_mutex), message_cv_(message_cv)
    {
        do_accept();
    }

    void queue_message(MQTT_task mqtt_task);

private:
    void do_accept();

    std::condition_variable& message_cv_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;

    moodycamel::BlockingConcurrentQueue<class MQTT_task>& MQTT_broker_task_queue;
    moodycamel::BlockingConcurrentQueue<class Broker_task>& DDS_message_queue;

    std::mutex& messages_mutex_;
    std::set<std::shared_ptr<class MQTTSession>>& mqtt_clients_;
    std::mutex& mqtt_clients_mutex_;
};

