#pragma once

#include <stddef.h>
#include "nng.h"
#include "reqrep0/rep.h"
#include "ofLog.h"
#include "ASyncWork.h"
#include "ofxNNGMessage.h"
#include "ofThreadChannel.h"
#include "ofxNNGNode.h"

namespace ofxNNG {
class Rep : public Node
{
public:
	struct Settings {
		int max_queue=16;
		bool allow_callback_from_other_thread=false;
	};
	template<typename Request, typename Response>
	bool setup(const Settings &s, const std::function<bool(const Request&, Response&)> &callback) {
		int result;
		result = nng_rep0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGRep") << "failed to open socket;" << nng_strerror(result);
			return false;
		}
		callback_ = [callback](Message &msg) {
			Response res;
			if(!callback(msg.get<Request>(), res)) {
				return false;
			}
			msg.set(res);
			return true;
		};
		async_ = s.allow_callback_from_other_thread;
		if(!async_) {
			ofAddListener(ofEvents().update, this, &Rep::update);
		}
		work_.initialize(s.max_queue, &Rep::async, this);
		while(auto work = work_.getUnused()) {
			nng_ctx_open(&work->ctx, socket_);
			work->state = aio::RECV;
			nng_ctx_recv(work->ctx, work->aio);
		}
		return true;
	}
private:
	aio::WorkPool work_;
	std::function<bool(Message&)> callback_;
	bool async_;
	ofThreadChannel<aio::Work*> channel_;
	
	static void async(void *arg) {
		auto work = (aio::Work*)arg;
		auto me = (Rep*)work->userdata;
		switch(work->state) {
			case aio::RECV: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGRep") << "failed to receive message; " << nng_strerror(result);
					return;
				}
				if(me->async_) {
					me->reply(work);
				}
				else {
					me->channel_.send(work);
				}
			}	break;
			case aio::SEND: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGRep") << "failed to send message; " << nng_strerror(result);
					break;
				}
				work->state = aio::RECV;
				nng_ctx_recv(work->ctx, work->aio);
			}	break;
		}
	}
	void update(ofEventArgs&) {
		aio::Work *work;
		while(channel_.tryReceive(work)) {
			reply(work);
		}
	}
	void reply(aio::Work *work) {
		Message msg(nng_aio_get_msg(work->aio));
		if(!callback_(msg)) {
			return;
		}
		nng_aio_set_msg(work->aio, msg);
		work->state = aio::SEND;
		nng_ctx_send(work->ctx, work->aio);
		msg.setSentFlag();
	}
};
}
