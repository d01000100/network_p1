#pragma once
#include "Message.h"
#include "SendBuffer.h"
#include "RecieveBuffer.h"

SendBuffer writeMessage(LoginMessage* message);
SendBuffer writeMessage(JoinMessage* message);
SendBuffer writeMessage(LeaveMessage* message);
SendBuffer writeMessage(SendMessage* message);
SendBuffer writeMessage(RecieveMessage* message);

Message* readMessage(RecieveBuffer buffer);