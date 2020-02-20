#pragma once

#include <stddef.h>
#include "nng.h"
#include "supplemental/util/platform.h"
#include "pipeline0/pull.h"
#include "ASyncWork.h"
#include "ofxNNGMessage.h"
#include "ofxNNGNode.h"
#include "ofThreadChannel.h"
#include "detail/apply.h"

namespace ofxNNG {
class Pull : public Node
{
public:
	struct Settings {
		Settings(){}
		bool allow_callback_from_other_thread=false;
	};
	bool setup(const Settings &s=Settings()) {
		int result;
		result = nng_pull0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGPull") << "failed to open socket;" << nng_strerror(result);
			return false;
		}
		setEnabledAutoUpdate(!s.allow_callback_from_other_thread);
		nng_aio_alloc(&aio_, &Pull::receive, this);
		nng_recv_aio(socket_, aio_);
		return true;
	}
	~Pull() {
		if(aio_) nng_aio_free(aio_);
	}
	template<typename ...Args, typename F>
	auto setCallback(F &&func)
	-> decltype(func(declval<Args>()...), void()) {
		callback_ = [func](Message msg) {
			apply<Args...>(func, msg);
		};
	}
	template<typename ...Ref>
	void setCallback(Ref &...ref) {
		callback_ = [&ref...](Message msg) {
			msg.to(ref...);
		};
	}
private:
	nng_aio *aio_;
	std::function<void(Message)> callback_;
	ofThreadChannel<Message> channel_;
	static void receive(void *arg) {
		auto me = (Pull*)arg;
		auto result = nng_aio_result(me->aio_);
		if(result != 0) {
			ofLogError("ofxNNGPull") << "failed to receive message; " << nng_strerror(result);
			return;
		}
		Message msg(nng_aio_get_msg(me->aio_));
		if(me->isEnabledAutoUpdate()) {
			me->channel_.send(std::move(msg));
		}
		else {
			me->callback_(std::move(msg));
		}
		nng_recv_aio(me->socket_, me->aio_);
	}
	void update() {
		Message msg;
		while(channel_.tryReceive(msg)) {
			callback_(std::move(msg));
		}
	}
};
}
