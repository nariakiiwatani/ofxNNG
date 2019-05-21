#pragma once

#include <stddef.h>
#include "nng.h"
#include "supplemental/util/platform.h"
#include "pipeline0/pull.h"
#include "ASyncWork.h"
#include "ofxNNGParseFunctions.h"
#include "ofxNNGNode.h"

namespace ofx {
namespace nng {
class Pull : public Node
{
public:
	struct Settings {
		bool allow_callback_from_other_thread=false;
	};
	template<typename T>
	bool setup(const Settings &s, const std::function<void(const T&)> &callback) {
		int result;
		result = nng_pull0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGPull") << "failed to open socket;" << nng_strerror(result);
			return false;
		}
		callback_ = [callback](nng_msg *msg) {
			callback(util::parse<T>(msg));
		};
		async_ = s.allow_callback_from_other_thread;
		if(!async_) {
			ofAddListener(ofEvents().update, this, &Pull::update);
		}
		nng_aio_alloc(&aio_, &Pull::receive, this);
		nng_recv_aio(socket_, aio_);
		return true;
	}
	~Pull() {
		if(aio_) nng_aio_free(aio_);
	}
private:
	nng_aio *aio_;
	std::function<void(nng_msg*)> callback_;
	bool async_;
	ofThreadChannel<nng_msg*> channel_;
	static void receive(void *arg) {
		auto me = (Pull*)arg;
		auto result = nng_aio_result(me->aio_);
		if(result != 0) {
			ofLogError("ofxNNGPull") << "failed to receive message; " << nng_strerror(result);
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
}}
