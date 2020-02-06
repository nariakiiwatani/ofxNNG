#pragma once

#include <stddef.h>
#include "nng.h"
#include "supplemental/util/platform.h"
#include "pubsub0/sub.h"
#include "ASyncWork.h"
#include "ofxNNGNode.h"

#include "ofLog.h"
#include "ofEventUtils.h"
#include "ofEvents.h"
#include "ofThreadChannel.h"
#include <map>

namespace ofxNNG {
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
	bool subscribe(const std::string &topic, std::function<void(const std::string&, T&&)> callback) {
		return subscribe<ofBuffer>(topic.data(), topic.size(), [=](const ofBuffer &topic, T &&message) {
			callback(topic.getText(), std::forward<T>(message));
		});
	}
	template<typename T>
	bool subscribe(const void *topic_data, std::size_t topic_size, std::function<void(const ofBuffer&, T&&)> callback) {
		int result = nng_setopt(socket_, NNG_OPT_SUB_SUBSCRIBE, topic_data, topic_size);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to subscribe topic; " << nng_strerror(result);
			return false;
		}
		callback_.emplace_back(ofBuffer{(const char*)topic_data, topic_size}, [=](const ofBuffer &topic, Message &&msg) {
			callback(topic, std::move(msg.get<T>(topic.size())));
		});
		return true;
	}
	template<typename T>
	bool subscribe(std::function<void(T&&)> callback) {
		int result = nng_setopt(socket_, NNG_OPT_SUB_SUBSCRIBE, nullptr, 0);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to subscribe topic; " << nng_strerror(result);
			return false;
		}
		callback_.emplace_back(ofBuffer{nullptr, 0}, [=](const ofBuffer &topic, Message &&msg) {
			callback(std::move(msg.get<T>()));
		});
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
		callback_.erase(std::remove_if(std::begin(callback_), std::end(callback_), [data](std::pair<ofBuffer, std::function<void(const ofBuffer&, Message&&)>> &kv) {
			return kv.first.size() == data.size() && memcmp(kv.first.getData(), data.getData(), data.size()) == 0;
		}), std::end(callback_));
		return true;
	}
private:
	nng_aio *aio_;
	std::vector<std::pair<ofBuffer, std::function<void(const ofBuffer&, Message&&)>>> callback_;
	bool async_;
	ofThreadChannel<Message> channel_;
	static void receive(void *arg) {
		auto me = (Sub*)arg;
		auto result = nng_aio_result(me->aio_);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to receive message; " << nng_strerror(result);
			return;
		}
		auto msg = Message(nng_aio_get_msg(me->aio_));
		if(me->async_) {
			me->dispatch(msg);
//			nng_msg_free(msg);
		}
		else {
			me->channel_.send(std::move(msg));
		}
		nng_recv_aio(me->socket_, me->aio_);
	}
	void update(ofEventArgs&) {
		Message msg;
		while(channel_.tryReceive(msg)) {
			dispatch(msg);
//			nng_msg_free(msg);
		}
	}
	void dispatch(Message &msg) {
		std::for_each(std::begin(callback_), std::end(callback_), [&](const std::pair<ofBuffer, std::function<void(const ofBuffer&, Message&&)>> &kv) {
			if(memcmp(msg.data(), kv.first.getData(), kv.first.size())==0) {
				kv.second(kv.first, std::move(msg.clone()));
			}
		});
	}
};
}
