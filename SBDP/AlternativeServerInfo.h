#pragma once
#include <SESSolutionX\SBDP\AltDiscoveredBrokerData.h>
#include "ConnectedClient.h"
#include <vector>
extern std::vector<AltDiscoveredBrokerData> alt_broker_list;
namespace AlternativeServerInfo
{
	static int find_broker(std::string broker_id) 
	{
		int i = 0;
		for (AltDiscoveredBrokerData temp : alt_broker_list)
		{
			if (broker_id.compare(temp.get_broker_id()) == 0) return i;
			i++;
		}
		return -1;
	}

	static void add_broker(AltDiscoveredBrokerData broker) 
	{
		std::cout << "Broker[ " << broker.get_broker_id() << "] is added into the list" << std::endl;
		alt_broker_list.push_back(broker);
	}

	static void remove_broker(AltDiscoveredBrokerData broker) 
	{
		int result = find_broker(broker.get_broker_id());
		alt_broker_list.erase(alt_broker_list.begin() + result);
	}

	static Locator get_broker(ConnectedClient client) 
	{
		/*for (AltDiscoveredBrokerData temp : alt_broker_list)
		{
			
		}*/
		//this should be fixed!
		return alt_broker_list[0].get_broker_locator()[0];
	}
};

