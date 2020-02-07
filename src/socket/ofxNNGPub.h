#pragma once

#include <stddef.h>
#include "nng.h"
#include "pubsub0/pub.h"
#include "ofxNNGNode.h"
#include "ofLog.h"
#include "ofxNNGMessage.h"

namespace ofxNNG {
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
	bool send(Message msg) {
		int result;
		result = nng_sendmsg(socket_, msg, 0);
		if(result != 0) {
			ofLogError("ofxNNGPub") << "failed to send message; " << nng_strerror(result);
			return false;
		}
		msg.setSentFlag();
		return true;
	}
	bool send(const std::string &topic, Message msg) {
		msg.prepend(topic);
		return send(std::move(msg));
	}
};
}

