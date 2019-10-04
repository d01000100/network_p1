#include "RoomManager.h"

void RoomManager::addMember(std::string room_name, ClientInfo* client) {
	if (!rooms[room_name].members) {
		rooms[room_name].members = new std::vector<ClientInfo*>;
	}
	rooms[room_name].members->push_back(client);
}

void RoomManager::removeMember(std::string room_name, ClientInfo* client) {
	std::vector<ClientInfo*>* members = rooms[room_name].members;
	for (std::vector<ClientInfo*>::iterator m = rooms[room_name].members->begin();
		m != members->end();
		m++)
	{
		if ((*m) == client) {
			members->erase(m);
			return;
		}
	}
}
	
void RoomManager::deleteMember(ClientInfo* client) {
	for (std::map<std::string, Room>::iterator r = rooms.begin();
		r != rooms.end();
		r++)
	{
		removeMember(r->first, client);
	}
}

std::vector<ClientInfo*>* RoomManager::getMembers(std::string room_name) {
	return rooms[room_name].members;
}

void RoomManager::printRooms() {
	printf("ROOMS:\n");
	for (std::map<std::string, Room>::iterator r = rooms.begin();
		r != rooms.end();
		r++)
	{
		printf("Room Name: %s\n", r->first.c_str());
		std::vector<ClientInfo*> *members = r->second.members;
		printf("Room members:\n");
		for (int m = 0; m < members->size(); m++) {
			printf("- %s", members->at(m)->name.c_str());
		}
		printf("\n");
	}
}