#pragma once

#include "nng.h"
#include "protocol/bus0/bus.h"
#include "ofxNNGNode.h"
#include "ofxNNGParseFunctions.h"
#include "ofxNNGConvertFunctions.h"

namespace ofxNNG {
class Bus : public Node
{
public:
	struct Settings {
		bool allow_callback_from_other_thread=false;
	};
	template<typename T>
	bool setup(const Settings &s, const std::function<void(const T&)> &callback) {
		int result;
		result = nng_bus_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGBus") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		callback_ = [callback](nng_msg *msg) {
			callback(util::parse<T>(msg));
		};
		async_ = s.allow_callback_from_other_thread;
		if(!async_) {
			ofAddListener(ofEvents().update, this, &Bus::update);
		}
		nng_aio_alloc(&aio_, &Bus::receive, this);
		nng_recv_aio(socket_, aio_);
		return true;
	}
	template<typename T>
	bool send(const T &data, bool blocking=false) {
		nng_msg *msg;
		nng_msg_alloc(&msg, 0);
		if(!util::convert(data, msg)) {
			ofLogError("ofxNNGBus") << "failed to convert message";
			return false;
		}
		int result;
		int flags = 0;
		if(!blocking) flags |= NNG_FLAG_NONBLOCK;
		result = nng_sendmsg(socket_, msg, flags);
		if(result != 0) {
			ofLogError("ofxNNGBus") << "failed to send message; " << nng_strerror(result);
			nng_msg_free(msg);
			return false;
		}
		return true;
	}
	~Bus() {
		if(aio_) nng_aio_free(aio_);
	}
private:
	nng_aio *aio_;
	std::function<void(nng_msg*)> callback_;
	bool async_;
	ofThreadChannel<nng_msg*> channel_;
	static void receive(void *arg) {
		auto me = (Bus*)arg;
		auto result = nng_aio_result(me->aio_);
		if(result != 0) {
			ofLogError("ofxNNGBus") << "failed to receive message; " << nng_strerror(result);
			return;
		}
		auto msg = nng_aio_get_msg(me->aio_);
		if(me->async_) {
			me->callback_(msg);
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
			callback_(msg);
			nng_msg_free(msg);
		}
	}
};
}
