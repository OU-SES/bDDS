#pragma once
#include <string>
class ConnectedClient {
public:
	ConnectedClient(std::string protocol, int flags, int keep_alive, std::string id, std::string password) : client_protocol_kind{ protocol }, flags{ flags }, keep_alive{ keep_alive }, id{ id }, password{ password }{};
	ConnectedClient() : client_protocol_kind{ "" }, flags{ 0 }, keep_alive{ 0 }, id{ "" }, password{ "" }{};
	std::string get_protocol() { return client_protocol_kind; }
	void set_protocol(std::string protocol) { client_protocol_kind = protocol; }
	void set_flags(int flags) { this->flags = flags; }
	void set_keep_alive(int alive) { this->keep_alive = alive; }
	void set_id(std::string id) { this->id = id; }
	std::string get_id() { return id; }
	void set_password(std::string password) { this->password = password; }
private:
	std::string client_protocol_kind;
	int flags;
	int keep_alive;
	std::string id;
	std::string password;
};