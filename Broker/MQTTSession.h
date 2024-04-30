#ifndef MQTT_SESSION_H
#define MQTT_SESSION_H
#include <boost/asio.hpp>
#include <set>
#include <memory>
#include <mutex>
#include <queue>

#include <SESSolutionX/Broker/BrokerTask.h>
#include <SESSolutionX/SBDP/ConnectedClient.h>
#include <boost/lockfree/queue.hpp>
#include <SESSolutionX/moodycamel/blockingconcurrentqueue.h>

class MQTTSession : public std::enable_shared_from_this<MQTTSession> {
public:
    MQTTSession(boost::asio::ip::tcp::socket socket, std::set<std::shared_ptr<MQTTSession>>& mqtt_clients, std::mutex& mqtt_clients_mutex, moodycamel::BlockingConcurrentQueue<class MQTT_task>& MQTT_broker_task_queue, moodycamel::BlockingConcurrentQueue<class Broker_task>& DDS_task_queue, std::mutex& mqtt_messages_mutex, std::condition_variable& message_cv)
        : socket_(std::move(socket)), mqtt_clients_(mqtt_clients), mqtt_clients_mutex_(mqtt_clients_mutex), MQTT_broker_task_queue(MQTT_broker_task_queue), DDS_task_queue(DDS_task_queue), mqtt_messages_mutex_(mqtt_messages_mutex), message_cv_(message_cv){}

    void start();
    void send(const std::string& message);
    int send(std::vector<char> message, int task_id);
    void read();
    std::string get_topic() 
    {
        return topic;
    }

    void set_topic(std::string topic_) 
    {
        topic = topic_;
    }

    ConnectedClient get_connected_client() 
    {
        return connected_client;
    }

    ~MQTTSession() 
    {
        std::cout << "client " << name << " session is closed." << std::endl;
    }

    bool sub_requested() { return sub_requested_; }
    void set_sub_requeseted(bool sub_request) { sub_requested_ = sub_request; }

private:
    void MQTTSession::message_process(char* recv_data);
    std::condition_variable& message_cv_;
    boost::asio::ip::tcp::socket socket_;
    std::set<std::shared_ptr<MQTTSession>>& mqtt_clients_;
    std::mutex& mqtt_clients_mutex_;
    //std::priority_queue<class MQTT_task>& mqtt_messages_;
    //boost::lockfree::queue<class MQTT_task>& MQTT_broker_task_queue;
    moodycamel::BlockingConcurrentQueue<class MQTT_task>& MQTT_broker_task_queue;
    moodycamel::BlockingConcurrentQueue<class Broker_task>& DDS_task_queue;
    std::map<int, std::vector<char>> MQTT_publish_map;

    std::mutex& mqtt_messages_mutex_;
    char packet_[1024];
    const int MAX_PACKET_SIZE = 1024;
    ConnectedClient connected_client;
    std::queue <std::string> retain_messages;
    //boost::lockfree::queue<std::string> retain_messages;
    std::string name;
    int client_n = 0;
    std::string topic;
    bool sub_requested_ = false;
};
#endif MQTT_SESSION_H
