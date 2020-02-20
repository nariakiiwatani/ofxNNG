#pragma once

#include <stddef.h>
#include "nng.h"
#include "supplemental/util/platform.h"
#include "pubsub0/sub.h"
#include "ASyncWork.h"
#include "ofxNNGNode.h"
#include "ofxNNGMessage.h"
#include "detail/apply.h"

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
		Settings(){}
		bool allow_callback_from_other_thread=false;
	};
	bool setup(const Settings &s=Settings()) {
		int result;
		result = nng_sub0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to open socket;" << nng_strerror(result);
			return false;
		}
		setEnabledAutoUpdate(!s.allow_callback_from_other_thread);
		nng_aio_alloc(&aio_, &Sub::receive, this);
		nng_recv_aio(socket_, aio_);
		return true;
	}
	~Sub() {
		if(aio_) nng_aio_free(aio_);
	}

	template<typename ...Args, typename F>
	auto subscribe(const std::string &topic, F &&func)
	-> decltype(func(declval<Args>()...), bool()) {
		return subscribe(topic.data(), topic.size(), [func](Message msg) {
			apply<Args...>(func, msg);
		});
	}
	template<typename ...Args, typename F>
	auto subscribe(const std::pair<const void*, std::size_t> &topic, F &&func)
	-> decltype(func(declval<Args>()...), bool()) {
		return subscribe(topic.first, topic.second, [func](Message msg) {
			apply<Args...>(func, msg);
		});
	}
	template<typename ...Ref>
	bool subscribe(const std::string &topic, Ref &...ref) {
		return subscribe(topic.data(), topic.size(), [&ref...](Message msg) {
			msg.to(ref...);
		});
	}
	template<typename ...Ref>
	bool subscribe(const std::pair<const void*, std::size_t> &topic, Ref &...ref) {
		return subscribe(topic.first, topic.second, [&ref...](Message msg) {
			msg.to(ref...);
		});
	}

	bool unsubscribe(const std::string &topic) {
		return unsubscribe(std::make_pair(topic.data(), topic.length()));
	}
	bool unsubscribe(const std::pair<const void*, std::size_t> &topic) {
		int result = nng_setopt(socket_, NNG_OPT_SUB_UNSUBSCRIBE, topic.first, topic.second);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to unsubscribe topic; " << nng_strerror(result);
			return false;
		}
		ofBuffer data((const char*)topic.first, topic.second);
		callback_.erase(std::remove_if(std::begin(callback_), std::end(callback_), [data](std::pair<ofBuffer, std::function<void(Message)>> &kv) {
			return kv.first.size() == data.size() && memcmp(kv.first.getData(), data.getData(), data.size()) == 0;
		}), std::end(callback_));
		return true;
	}
private:
	bool subscribe(const void *topic_data, std::size_t topic_size, const std::function<void(Message)> &func) {
		int result = nng_setopt(socket_, NNG_OPT_SUB_SUBSCRIBE, topic_data, topic_size);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to subscribe topic; " << nng_strerror(result);
			return false;
		}
		callback_.emplace_back(ofBuffer{(const char*)topic_data, topic_size}, func);
		return true;
	}
	nng_aio *aio_;
	std::vector<std::pair<ofBuffer, std::function<void(Message)>>> callback_;
	ofThreadChannel<Message> channel_;
	static void receive(void *arg) {
		auto me = (Sub*)arg;
		auto result = nng_aio_result(me->aio_);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to receive message; " << nng_strerror(result);
			return;
		}
		Message msg(nng_aio_get_msg(me->aio_));
		if(me->isEnabledAutoUpdate()) {
			me->channel_.send(std::move(msg));
		}
		else {
			me->dispatch(std::move(msg));
		}
		nng_recv_aio(me->socket_, me->aio_);
	}
	void update() {
		Message msg;
		while(channel_.tryReceive(msg)) {
			dispatch(std::move(msg));
		}
	}
	void dispatch(Message msg) {
		std::for_each(std::begin(callback_), std::end(callback_), [&](const std::pair<ofBuffer, std::function<void(Message)>> &kv) {
			if(memcmp(msg.data(), kv.first.getData(), kv.first.size())==0) {
				Message copy = msg;
				nng_msg_trim(copy, kv.first.size());
				kv.second(std::move(copy));
			}
		});
	}
};
}
