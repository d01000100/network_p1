#pragma once
#include <string>

#include "auth.pb.h"

std::string writeSignUpRequest(std::string username, std::string clear_password);
std::string writeSignUpOK(std::string username);
std::string writeSignUpError(std::string username, auth_protocol::Error error);
std::string writeLoginRequest(std::string username, std::string clear_password);
std::string writeLoginOK(std::string username);
std::string writeLoginError(std::string username, auth_protocol::Error error);

/* Returns a generic protobuf::Message pointer.
   The client must use GetTypeName() to get the proper message type
   and to cast it to get the data inside.

   Possible message types:
   - "auth_protocol.Request"
   - "auth_protocol.ResponseOK"
   - "auth_protocol.ResponsError"

   Returns NULL if the serialized message does not parse to any known type.*/
google::protobuf::Message* readAuthMessage(std::string message);

