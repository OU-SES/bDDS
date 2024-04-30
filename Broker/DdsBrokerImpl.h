#include "BrokerPublisher.h"
#include "BrokerSubscriber.h"
#include "boost/container/vector.hpp"
#ifndef DDS_BROKER
#define DDS_BROKER

class DataBrokerWriter;
typedef DataBrokerWriter* DataBrokerWriter_ptr;

class DataBrokerReader;
typedef DataBrokerReader* DataBrokerReader_ptr;

#endif // !DDS_BROKER
