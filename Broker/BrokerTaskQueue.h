#ifndef BROKER_TASK_QUEUE_H
#define BROKER_TASK_QUEUE_H
#include <boost/lockfree/queue.hpp>
#include <iostream>
#include <queue>
#include "BrokerTask.h"
#include "BrokerConfigurationSet.h"
#include <boost/bind/bind.hpp>
using namespace boost::placeholders;

// Define lock free MQTT task queue
//boost::lockfree::queue<class MQTT_task> MQTT_broker_task_queue(2048);
//boost::lockfree::queue<std::string> MQTT_broker_retain_queue(30);

// Atomic flag to control the threads
//std::atomic<bool> shut_down(false);
#endif //BROKER_TASK_H
