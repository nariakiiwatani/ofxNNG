#pragma once

#include <stddef.h>
#include "nng.h"
#include "pipeline0/push.h"
#include "ASyncWork.h"
#include "ofxNNGMessage.h"
#include "ofxNNGNode.h"

namespace ofxNNG {
class Push : public Node
{
public:
	struct Settings {
		Settings(){}
	};
	bool setup(const Settings &s=Settings()) {
		int result;
		result = nng_push0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGPush") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		return true;
	}
	bool send(Message msg) {
		int result;
		result = nng_sendmsg(socket_, msg, 0);
		if(result != 0) {
			ofLogError("ofxNNGPush") << "failed to send message; " << nng_strerror(result);
			return false;
		}
		msg.setSentFlag();
		return true;
	}
};
}
