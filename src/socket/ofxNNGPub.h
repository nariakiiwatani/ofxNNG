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
	template<typename T>
	bool send(T &&msg) {
		return send(std::forward<Message>(to_msg(msg)));
	}
	bool send(Message &&msg) {
		int result;
		result = nng_sendmsg(socket_, msg, 0);
		if(result != 0) {
			ofLogError("ofxNNGPub") << "failed to send message; " << nng_strerror(result);
			return false;
		}
		msg.setSentFlag();
		return true;
	}
	template<typename T>
	bool send(const std::string &topic, T &&msg) {
		return send(topic, std::move(adl_converter<T>::to_msg(msg)));
	}
	template<>
	bool send(const std::string &topic, Message &&msg) {
		return send(topic.data(), topic.length(), std::forward<Message>(msg));
	}
	bool send(const void *topic_data, std::size_t topic_size, Message &&msg) {
		nng_msg_insert(msg, topic_data, topic_size);
		return send(std::forward<Message>(msg));
	}
};
}

