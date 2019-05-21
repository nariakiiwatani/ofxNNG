#pragma once

#include "nng.h"

namespace ofx {
namespace nng {
class Node
{
public:
	bool dial(const std::string &url, bool blocking=false) {
		int flags = 0;
		if(!blocking) flags |= NNG_FLAG_NONBLOCK;
		auto result = nng_dial(socket_, url.data(), dialer_, flags);
		if(result != 0) {
			return false;
		}
		return true;
	}
	bool listen(const std::string &url, bool blocking=false) {
		int flags = 0;
		if(!blocking) flags |= NNG_FLAG_NONBLOCK;
		auto result = nng_listen(socket_, url.data(), listener_, flags);
		if(result != 0) {
			return false;
		}
		return true;
	}
protected:
	nng_socket socket_;
	nng_dialer *dialer_=nullptr;
	nng_listener *listener_=nullptr;
};
}}
