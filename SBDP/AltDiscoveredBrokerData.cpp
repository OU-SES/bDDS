#include "AltDiscoveredBrokerData.h"
#include <vector>
std::string AltDiscoveredBrokerData::to_json()
{
    boost::property_tree::ptree output;
    output.put("BROKER.BrokerID", this->get_broker_id());
    output.put("BROKER.ExpectedClientNum", this->get_expected_client_num());
    output.put("BROKER.MaximumClientNum", this->get_maximum_num_client());
    output.put("BROKER.ConnectedClientNum", this->get_connected_client_count());
    boost::property_tree::ptree protocol, locator;
    std::vector<std::string> temp_list = this->get_protocol_list();
    for (std::string temp_protocol : temp_list) 
    {
        boost::property_tree::ptree item;
        item.put("", temp_protocol);
        protocol.push_back(std::make_pair("",item));
    }
    Locator broker_locator = this->get_broker_locator()[0];
    locator.put("ip",broker_locator.ip);
    locator.put("port", broker_locator.port);
    
    output.add_child("BROKER.Protocols", protocol);
    output.add_child("BROKER.Locators", locator);
    std::stringstream string_st;
    boost::property_tree::json_parser::write_json(string_st, output);
    return string_st.str();
}
