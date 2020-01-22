#pragma once

#include <stddef.h>
#include "nng.h"
#include "supplemental/util/platform.h"
#include "pubsub0/sub.h"
#include "ASyncWork.h"
#include "ofxNNGParseFunctions.h"
#include "ofxNNGNode.h"

#include "ofLog.h"
#include "ofEventUtils.h"
#include "ofEvents.h"
#include "ofThreadChannel.h"
#include <map>

namespace ofx {
namespace nng {
class Sub : public Node
{
public:
	struct Settings {
		bool allow_callback_from_other_thread=false;
	};
	bool setup(const Settings &s) {
		int result;
		result = nng_sub0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to open socket;" << nng_strerror(result);
			return false;
		}
		async_ = s.allow_callback_from_other_thread;
		if(!async_) {
			ofAddListener(ofEvents().update, this, &Sub::update);
		}
		nng_aio_alloc(&aio_, &Sub::receive, this);
		nng_recv_aio(socket_, aio_);
		return true;
	}
	~Sub() {
		if(aio_) nng_aio_free(aio_);
	}
	template<typename T>
	bool subscribe(const std::string &topic, const std::function<void(const T&)> &callback) {
		return subscribe(topic.data(), topic.length(), [callback](nng_msg *msg) {
			callback(util::parse<T>(msg));
		});
	}
	template<typename T>
	bool subscribe(const std::string &topic, const std::function<void(const std::string &topic, const T&)> &callback) {
		return subscribe(topic.data(), topic.length(), [callback, topic](nng_msg *msg) {
			nng_msg_trim(msg, topic.size());
			callback(topic, util::parse<T>(msg));
		});
	}
	template<typename T>
	bool subscribe(const void *topic, std::size_t topic_length, const std::function<void(const T&)> &callback) {
		return subscribe(topic, topic_length, [callback](nng_msg *msg) {
			callback(util::parse<T>(msg));
		});
	}
	template<typename T>
	bool subscribe(const void *topic, std::size_t topic_length, const std::function<void(const void *topic, const T&)> &callback) {
		return subscribe(topic, topic_length, [callback, topic, topic_length](nng_msg *msg) {
			nng_msg_trim(msg, topic_length);
			callback(topic, util::parse<T>(msg));
		});
	}
	bool subscribe(const void *topic, std::size_t topic_length, std::function<void(nng_msg*)> callback) {
		int result = nng_setopt(socket_, NNG_OPT_SUB_SUBSCRIBE, topic, topic_length);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to subscribe topic; " << nng_strerror(result);
			return false;
		}
		ofBuffer data((const char*)topic, topic_length);
		callback_.push_back(std::make_pair(data, callback));
		return true;
	}
	bool unsubscribe(const std::string &topic) {
		return unsubscribe(topic.data(), topic.length());
	}
	bool unsubscribe(const void *topic, std::size_t topic_length) {
		int result = nng_setopt(socket_, NNG_OPT_SUB_UNSUBSCRIBE, topic, topic_length);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to unsubscribe topic; " << nng_strerror(result);
			return false;
		}
		ofBuffer data((const char*)topic, topic_length);
		callback_.erase(std::remove_if(std::begin(callback_), std::end(callback_), [data](std::pair<ofBuffer, std::function<void(nng_msg*)>> &kv) {
			return kv.first.size() == data.size() && memcmp(kv.first.getData(), data.getData(), data.size()) == 0;
		}), std::end(callback_));
		return true;
	}
private:
	nng_aio *aio_;
	std::vector<std::pair<ofBuffer, std::function<void(nng_msg*)>>> callback_;
	bool async_;
	ofThreadChannel<nng_msg*> channel_;
	static void receive(void *arg) {
		auto me = (Sub*)arg;
		auto result = nng_aio_result(me->aio_);
		if(result != 0) {
			ofLogError("ofxNNGRep") << "failed to receive message; " << nng_strerror(result);
			return;
		}
		auto msg = nng_aio_get_msg(me->aio_);
		if(me->async_) {
			me->dispatch(msg);
			nng_msg_free(msg);
		}
		else {
			me->channel_.send(msg);
		}
		nng_recv_aio(me->socket_, me->aio_);
	}
	void update(ofEventArgs&) {
		nng_msg *msg;
		while(channel_.tryReceive(msg)) {
			dispatch(msg);
			nng_msg_free(msg);
		}
	}
	void dispatch(nng_msg *msg) {
		auto body = nng_msg_body(msg);
		std::for_each(std::begin(callback_), std::end(callback_), [body,msg](const std::pair<ofBuffer, std::function<void(nng_msg*)>> &kv) {
			if(memcmp(body, kv.first.getData(), kv.first.size())==0) {
				kv.second(msg);
			}
		});
	}
};
}}
