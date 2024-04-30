#include <boost/asio.hpp>
#include <iostream>
#include <set>
#include <mutex>
#include <queue>
#include "MQTTSession.h"
#include <SESSolutionX/Broker/BrokerTask.h>
#include <SESSolutionX/BrokerClient.h>
#include <MQTTPacket/MQTTControlPacketList.h>
#include <SESSolutionX/BrokerThreadMutex.h>
#include <SESSolutionX/moodycamel/blockingconcurrentqueue.h>
using namespace std::chrono;

//using mqtt_message_type = std::deque<class MQTT_task>;
using mqtt_message_type = std::priority_queue<class MQTT_task>;

class BrokerMQTTPublisher 
{
public:
    BrokerMQTTPublisher(mqtt_message_type& messages, moodycamel::BlockingConcurrentQueue<class MQTT_task>& MQTT_broker_task_queue, std::mutex& messages_mutex, std::set<std::shared_ptr<class MQTTSession>>& clients, std::mutex& clients_mutex, const int NUM_END_POINT_WRITERS, std::condition_variable& message_cv)
        : messages_(messages), messages_mutex_(messages_mutex), clients_(clients), clients_mutex_(clients_mutex) , message_cv_(message_cv), MQTT_broker_task_queue_(MQTT_broker_task_queue)
    {
        boost::thread::attributes boost_thread_attr;
        boost_thread_attr.set_stack_size(10 * 1024 * 1024);
        for (int i = 0; i < NUM_END_POINT_WRITERS; i++) 
        {
            broker_endpoint_writer.emplace_back(boost_thread_attr , boost::bind(&BrokerMQTTPublisher::MQTTEndPointWriter, this));
        }
    }

    ~BrokerMQTTPublisher() 
    {
        is_shutting_down_ = true;
        message_cv_.notify_all();
        for (auto& thread : broker_endpoint_writer) 
        {
            thread.join();
        }
    }

private:
    void send_message(std::string client_id, MQTT_task task_message)
    {
        auto client = get_client(client_id);

        auto packet_ = task_message.get_data();
        const int send_message_type_ = (packet_.at(0) >> 4) & 0x0f;

        try
        {

            if (client != NULL)
            {
                //std::unique_lock<std::mutex> lock(clients_mutex_);
                std::ofstream myfile;

                int result = -1;
                int count = 0;

                /*myfile.open("rec_evednt.txt", ios::app);
                myfile << "Task id:" << task_message.get_task_id() << "] has been sent[" << count << "th]to client" << std::endl;
                myfile.close();
                */
                if (!client->sub_requested()) {
                    if (send_message_type_ == mqttpacket::MQTT_CONTROL_PACKET_TYPE_SUBACK)
                    {
                        client->set_sub_requeseted(true);
                        /*myfile.open("rec_evednt.txt", ios::app);
                        myfile << "Client[" << client_id << " ] request SUBSCRIPTION. Task id:[" << task_message.get_task_id() << "] SUBACK is going to be trasnfered" << std::endl;
                        myfile.close();*/
                    }
                    if (send_message_type_ == mqttpacket::MQTT_CONTROL_PACKET_TYPE_PUBLISH)
                    {
                        /*std::unique_lock<std::mutex> lock_message(messages_mutex_);
                        messages_.push(std::move(task_message));
                        lock_message.unlock();
                        */
                        // lock free version
                        MQTT_broker_task_queue_.enqueue(std::move(task_message));

                        /*myfile.open("rec_evednt.txt", ios::app);
                        myfile << "Client[" << client_id << " ] does not request SUBSCRIPTION yet. Task id:[" << task_message.get_task_id() << "] will be stored again" << std::endl;
                        myfile.close();
                        */
                        //message_cv_.notify_one();
                        //message_cv_.notify_all();
                    }
                }
                result = client->send(task_message.get_data(), task_message.get_task_id());
                if (result < 0) {
                    /*std::unique_lock<std::mutex> lock_message(messages_mutex_);
                    messages_.push(std::move(task_message));
                    lock_message.unlock();
                    */
                    MQTT_broker_task_queue_.enqueue(std::move(task_message));

                    /*myfile.open("rec_evednt.txt", ios::app);
                    myfile << "Client[" << client_id << " ] fail to receive Task id:[" << task_message.get_task_id() << "], it will be stored again" << std::endl;
                    myfile.close();*/
                    //message_cv_.notify_one();
                    //message_cv_.notify_all();
                }
                //client->send(task_message.get_data(), task_message.get_task_id());

                //myfile.close();
                //lock.unlock();
            }
            else
            {
                //std::ofstream myfile;
                //myfile.open("rec_evednt.txt", ios::app);
                int arrived = duration_cast<std::chrono::milliseconds>(system_clock::now().time_since_epoch()).count();
                //myfile << "[MQTT_ENDPOINT_WRITER]: client [" << client_id << "] is not connected in the broker [task_id:" << task_message.get_task_id() << "][option:" << task_message.get_task_option() << "]" << std::endl;
                //myfile.close();
            }

        }
        catch (const std::exception& error)
        {
            printf("[SYSTEM] broker_mqtt_publisher :%s\n", error.what());
            std::ofstream myfile;
            myfile.open("rec_evednt.txt", ios::app);
            int arrived = duration_cast<std::chrono::milliseconds>(system_clock::now().time_since_epoch()).count();
            myfile << "[MQTT_ENDPOINT_WRITER]: client [" << client_id << "] is not connected in the broker [task_id:" << task_message.get_task_id() << "][option:" << task_message.get_task_option() << "]" << std::endl;
            myfile.close();
        }
    }
    void MQTTEndPointWriter() 
    {
        bool wait_trigger = true;
        //while (!is_shutting_down_) 
        while(!is_shutting_down_)
        {
            //std::unique_lock<std::mutex> lock(messages_mutex_);
            //BROKER_THREAD::task_mutex_wlock(&BROKER_THREAD::mutex);
            //message_cv_.wait(lock, [this]() { return !messages_.empty() || is_shutting_down_; });
            if (is_shutting_down_) return;
            //MQTT_task task_message = messages_.front();
            /*auto task_message = messages_.top();
            messages_.pop();
            lock.unlock();
            */

            MQTT_task task_message; 
            MQTT_broker_task_queue_.wait_dequeue(task_message);
            //BROKER_THREAD::task_mutex_unlock(&BROKER_THREAD::mutex);
            auto task_size = task_message.get_data().size();
            /* {
                std::ofstream myfile;
                myfile.open("rec_evednt.txt", ios::app);
                int arrived = duration_cast<std::chrono::milliseconds>(system_clock::now().time_since_epoch()).count();
                myfile << "About to send message_[" << task_message.get_client_id() << "][task id:" << task_message.get_task_id() << "][option:" << task_message.get_task_option() << "]messaage [size:" << task_size << "]" << std::endl;
                myfile.close();
            }*/

            if (task_size > 0)
            {
                //analyze task and do work.
                int task_option = task_message.get_task_option();
                //std::cout << "task received[" << task_option <<  "]" << std::endl;

                auto client_id = task_message.get_client_id();
                

                /*newly added here*/
                if (client_id == "-99")
                {
                    Broker_topic topic = BROKER_CONFIGURATION::find_topic("GPS");
                    for (const auto& client : topic.mqtt_client_sessions)
                    {
                        send_message(client->get_connected_client().get_id(), task_message);
                    }
                }
                else 
                {
                    send_message(client_id, task_message);
                }

            }
            
        }
    }

    std::shared_ptr<class MQTTSession> get_client(std::string client_id) 
    {
        for (auto client : clients_) 
        {
            if (client->get_connected_client().get_id() == client_id) return client;
        }
        return NULL;
    }

    std::vector<boost::thread> broker_endpoint_writer;
    mqtt_message_type& messages_;
    std::mutex& messages_mutex_;
    std::condition_variable& message_cv_;
    bool is_shutting_down_ = false;

    //boost::lockfree::queue<class MQTT_task>& MQTT_broker_task_queue_;
    moodycamel::BlockingConcurrentQueue<class MQTT_task>& MQTT_broker_task_queue_;

    std::set<std::shared_ptr<class MQTTSession>>& clients_;
    std::mutex& clients_mutex_;
};
