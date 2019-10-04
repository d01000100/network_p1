#pragma once
#include <vector>
#include <map>
#include "data_structs.h"

class RoomManager
{
private:
	std::map<std::string, Room> rooms;

public:
	void addMember(std::string room_name, ClientInfo* client);
	void removeMember(std::string room_name, ClientInfo* client);
	// Remove member from all rooms;
	void deleteMember(ClientInfo* client);
	std::vector<ClientInfo*>* getMembers(std::string room_name);
	void printRooms();
	bool broadcastMessage(RecieveMessage* message);
};

