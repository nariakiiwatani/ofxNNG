#pragma once

#include <stddef.h>
#include "nng.h"
#include "pubsub0/pub.h"
#include "ASyncWork.h"

namespace ofx {
namespace nng {
class Pub
{
public:
	struct Settings {
		std::string url;
		nng_listener *listener=nullptr;
		bool blocking=false;
	};
	bool setup(const Settings &s) {
		int result;
		result = nng_pub0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGPub") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		int flags = 0;
		if(!s.blocking) flags |= NNG_FLAG_NONBLOCK;
		result = nng_listen(socket_, s.url.data(), s.listener, flags);
		if(result != 0) {
			ofLogError("ofxNNGPub") << "failed to create listener; " << nng_strerror(result);
			return false;
		}
		return true;
	}
	void send(void *data, size_t len, int flags) {
		int result;
		result = nng_send(socket_, data, len, flags);
		if(result != 0) {
			ofLogError("ofxNNGPub") << "failed to send message; " << nng_strerror(result);
			return;
		}
	}
private:
	nng_socket socket_;
};
}
}
