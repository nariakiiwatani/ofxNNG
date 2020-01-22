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
		return send(nullptr, 0, data);
	}
	template<typename T>
	bool send(const std::string &topic, const T &data) {
		return send(topic.c_str(), topic.size(), data);
	}
	template<typename T>
	bool send(const void *topic, std::size_t topic_length, const T &data) {
		nng_msg *msg;
		nng_msg_alloc(&msg, 0);
		if(!util::convert(data, msg)) {
			ofLogError("ofxNNGPub") << "failed to convert message";
			return false;
		}
		if(topic != nullptr && topic_length > 0) {
			nng_msg_insert(msg, topic, topic_length);
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
