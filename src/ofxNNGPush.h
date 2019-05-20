#pragma once

#include <stddef.h>
#include "nng.h"
#include "pipeline0/push.h"
#include "ASyncWork.h"
#include "ofxNNGConvertFunctions.h"

namespace ofx {
namespace nng {
class Push
{
public:
	struct Settings {
		std::string url;
		nng_dialer *dialer=nullptr;
		bool blocking=false;
	};
	bool setup(const Settings &s) {
		int result;
		result = nng_push0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGPush") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		int flags = 0;
		if(!s.blocking) flags |= NNG_FLAG_NONBLOCK;
		result = nng_dial(socket_, s.url.data(), s.dialer, flags);
		if(result != 0) {
			ofLogError("ofxNNGPush") << "failed to create dialer; " << nng_strerror(result);
			return false;
		}
		return true;
	}
	template<typename T>
	bool send(const T &data) {
		nng_msg *msg;
		nng_msg_alloc(&msg, 0);
		if(!util::convert(data, msg)) {
			ofLogError("ofxNNGPush") << "failed to convert message";
			return false;
		}
		int result;
		result = nng_sendmsg(socket_, msg, 0);
		if(result != 0) {
			ofLogError("ofxNNGPush") << "failed to send message; " << nng_strerror(result);
			nng_msg_free(msg);
			return false;
		}
		return true;
	}
private:
	nng_socket socket_;
};
}
}
