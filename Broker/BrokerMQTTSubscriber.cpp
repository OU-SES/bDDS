#include "BrokerMQTTSubscriber.h"

void BrokerMQTTSubscriber::queue_message(MQTT_task mqtt_task)
{
    std::lock_guard<std::mutex> lock(messages_mutex_);
    //message_queue.push(mqtt_task);
}

void BrokerMQTTSubscriber::do_accept() 
{
    acceptor_.async_accept(socket_,
        [this](boost::system::error_code ec) 
        {
            if (!ec) {
                //std::cout << "Client is comming..." << std::endl;
                std::make_shared<MQTTSession>(std::move(socket_), mqtt_clients_, mqtt_clients_mutex_, MQTT_broker_task_queue, DDS_message_queue, messages_mutex_, message_cv_)->start();
            }
            do_accept();
        });
}

