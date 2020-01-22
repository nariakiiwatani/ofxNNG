#pragma once

#include <stddef.h>
#include "nng.h"
#include "pubsub0/pub.h"
#include "ofxNNGConvertFunctions.h"
#include "ofxNNGNode.h"
#include "ofLog.h"

namespace ofx {
namespace nng {
class Pub : public Node
{
public:
	struct Settings {
	};
	bool setup(const Settings &s) {
		int result;
		result = nng_pub0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGPub") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		return true;
	}
	template<typename T>
	bool send(const T &data) {
		nng_msg *msg;
		nng_msg_alloc(&msg, 0);
		if(!util::convert(data, msg)) {
			ofLogError("ofxNNGPub") << "failed to convert message";
			return false;
		}
		int result;
		result = nng_sendmsg(socket_, msg, 0);
		if(result != 0) {
			ofLogError("ofxNNGPub") << "failed to send message; " << nng_strerror(result);
			nng_msg_free(msg);
			return false;
		}
		return true;
	}
};
}
}
