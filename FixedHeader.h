class FixedHeader {
private :
	unsigned int control_packet_type;
	unsigned int flags;
	int length;

public:
	void set_control_packet_type(unsigned int value) { this->control_packet_type = value; }
	void set_flags(unsigned int value) { this->flags = value; }
	void set_length(int value) { this->length = value; }
	unsigned int get_control_packet_type() { return this->control_packet_type; }
	unsigned int get_flags() { return this->flags; }
	int get_length() { return this->length; }
};
