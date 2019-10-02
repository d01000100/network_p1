#pragma once

#include <string>
class SendBuffer
{
private:
	unsigned int size;
	unsigned int write_index;
	void growBuffer();
	unsigned char* buffer;
public:
	unsigned char* getBuffer(); // to expose to the socket
	SendBuffer(unsigned int size = 64);
	bool writeInt(int data);
	bool writeShort(short data);
	bool writeString(std::string data);
	// for debugging
	void printBuffer();
};

