#pragma once

#include "nng.h"
#include "protocol/bus0/bus.h"
#include "ofxNNGNode.h"
#include "ofxNNGMessage.h"

namespace ofxNNG {
class Bus : public Node
{
public:
	struct Settings {
		Settings(){}
		bool allow_callback_from_other_thread=false;
	};
	bool setup(const Settings &s=Settigs()) {
		int result;
		result = nng_bus_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGBus") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		async_ = s.allow_callback_from_other_thread;
		if(!async_) {
			ofAddListener(ofEvents().update, this, &Bus::update);
		}
		nng_aio_alloc(&aio_, &Bus::receive, this);
		nng_recv_aio(socket_, aio_);
		return true;
	}
	template<typename T>
	void setCallback(const std::function<void(T&&)> &callback) {
		callback_ = [callback](Message msg) {
			callback(msg.get<T>());
		};
	}
	bool send(Message msg, bool blocking=false) {
		int result;
		int flags = 0;
		if(!blocking) flags |= NNG_FLAG_NONBLOCK;
		result = nng_sendmsg(socket_, msg, flags);
		if(result != 0) {
			ofLogError("ofxNNGBus") << "failed to send message; " << nng_strerror(result);
			return false;
		}
		msg.setSentFlag();
		return true;
	}
	~Bus() {
		if(aio_) nng_aio_free(aio_);
	}
private:
	nng_aio *aio_;
	std::function<void(Message)> callback_;
	bool async_;
	ofThreadChannel<Message> channel_;
	static void receive(void *arg) {
		auto me = (Bus*)arg;
		auto result = nng_aio_result(me->aio_);
		if(result != 0) {
			ofLogError("ofxNNGBus") << "failed to receive message; " << nng_strerror(result);
			return;
		}
		Message msg(nng_aio_get_msg(me->aio_));
		if(me->async_) {
			me->callback_(std::move(msg));
		}
		else {
			me->channel_.send(std::move(msg));
		}
		nng_recv_aio(me->socket_, me->aio_);
	}
	void update(ofEventArgs&) {
		Message msg;
		while(channel_.tryReceive(msg)) {
			callback_(std::move(msg));
		}
	}
};
}
