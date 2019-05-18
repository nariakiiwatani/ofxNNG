#pragma once

#include <stddef.h>
#include "nng.h"
#include "pubsub0/sub.h"
#include "ASyncWork.h"

namespace ofx {
namespace nng {
class Sub
{
public:
	struct Settings {
		std::string url;
		nng_dialer *dialer=nullptr;
		bool blocking=false;
		int max_queue=16;
		std::function<bool(nng_msg*)> onReceive=[](nng_msg *msg) { return true; };
	};
	bool setup(const Settings &s) {
		int result;
		result = nng_sub0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to open socket;" << nng_strerror(result);
			return false;
		}
		int flags = 0;
		if(!s.blocking) flags |= NNG_FLAG_NONBLOCK;
		result = nng_dial(socket_, s.url.data(), s.dialer, flags);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to create dialer; " << nng_strerror(result);
			return false;
		}
		result = nng_setopt(socket_, NNG_OPT_SUB_SUBSCRIBE, nullptr, 0);
		onReceive = s.onReceive;
		return true;
	}
	bool hasWaitingMessage() const {
	}
	void receive(int flags) {
		nng_msg *msg;
		int result;
		result = nng_recvmsg(socket_, &msg, flags);
		if(result != 0) {
			if(result != 8) {
				ofLogError("ofxNNGSub") << "failed to receive message; " << nng_strerror(result);
			}
			return;
		}
		onReceive(msg);
	}
private:
	nng_socket socket_;
	
	std::function<bool(nng_msg*)> onReceive;
};
}}
