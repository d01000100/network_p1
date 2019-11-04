#include "AuthProtocol.h"

using namespace auth_protocol;

std::string writeSignUpRequest(std::string username, std::string plain_text_password) {
	Request req;

	req.set_username(username);
	req.set_action(SIGN_UP);
	req.set_plaintextpassword(plain_text_password);
	req.set_length(req.ByteSizeLong());

	return req.SerializeAsString();
}

std::string writeSignUpOK(std::string username) {
	ResponseOK response;

	response.set_username(username);
	response.set_action(SIGN_UP);
	response.set_length(response.ByteSizeLong());

	return response.SerializeAsString();

}

std::string writeSignUpError(std::string username, Error error) {
	ResponseError response;

	response.set_username(username);
	response.set_action(SIGN_UP);
	response.set_error(error);
	response.set_length(response.ByteSizeLong());

	return response.SerializeAsString();

}

std::string writeLoginRequest(std::string username, std::string plain_text_password) {
	Request req;

	req.set_username(username);
	req.set_action(LOGIN);
	req.set_plaintextpassword(plain_text_password);
	req.set_length(req.ByteSizeLong());

	return req.SerializeAsString();

}

std::string writeLoginOK(std::string username) {
	ResponseOK response;

	response.set_username(username);
	response.set_action(LOGIN);
	response.set_length(response.ByteSizeLong());

	return response.SerializeAsString();

}

std::string writeLoginError(std::string username, Error error) {
	ResponseError response;

	response.set_username(username);
	response.set_action(LOGIN);
	response.set_error(error);
	response.set_length(response.ByteSizeLong());

	return response.SerializeAsString();

}

google::protobuf::Message* readAuthMessage(std::string message) {
	Request *req = new Request();
	if (req->ParseFromString(message)) {
		return req;
	}

	ResponseError *error = new ResponseError(); 
	if (error->ParseFromString(message)) {
		return error;
	}

	ResponseOK *ok = new ResponseOK();
	if (ok->ParseFromString(message)) {
		return ok;
	}

	return NULL;
}