#pragma once
#include <string>
class SendBuffer
{
private:
	unsigned int size;
	unsigned int write_index;
	void growBuffer();
public:
	unsigned char* buffer; // made public for debugging TODO: make private
	SendBuffer(unsigned int size = 64);
	bool writeInt(int data);
	bool writeShort(short data);
	bool writeString(std::string data);
	// for debugging
	void printBuffer();
};

