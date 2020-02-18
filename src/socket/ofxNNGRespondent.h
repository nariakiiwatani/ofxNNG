#pragma once

#include <stddef.h>
#include "nng.h"
#include "survey0/respond.h"
#include "ofLog.h"
#include "ASyncWork.h"
#include "ofxNNGMessage.h"
#include "ofThreadChannel.h"
#include "ofxNNGNode.h"

namespace ofxNNG {
class Respondent : public Node
{
public:
	struct Settings {
		Settings(){}
		int max_queue=16;
		bool allow_callback_from_other_thread=false;
	};
	bool setup(const Settings &s=Settings()) {
		int result;
		result = nng_respondent0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGRespondent") << "failed to open socket;" << nng_strerror(result);
			return false;
		}
		setEnabledAutoUpdate(!s.allow_callback_from_other_thread);
		work_.initialize(s.max_queue, &Respondent::async, this);
		while(auto work = work_.getUnused()) {
			nng_ctx_open(&work->ctx, socket_);
			work->state = aio::RECV;
			nng_ctx_recv(work->ctx, work->aio);
		}
		return true;
	}
	template<typename Request, typename Response>
	void setCallback(const std::function<bool(const Request&, Response&)> &callback) {
		callback_ = [callback](Message &msg) {
			Response res;
			if(!callback(msg.get<Request>(), res)) {
				return false;
			}
			msg.set(res);
			return true;
		};
	}
private:
	aio::WorkPool work_;
	std::function<bool(Message&)> callback_;
	ofThreadChannel<aio::Work*> channel_;
	
	static void async(void *arg) {
		auto work = (aio::Work*)arg;
		auto me = (Respondent*)work->userdata;
		switch(work->state) {
			case aio::RECV: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGRespondent") << "failed to receive message; " << nng_strerror(result);
					work->state = aio::RECV;
					nng_ctx_recv(work->ctx, work->aio);
					return;
				}
				if(me->isEnabledAutoUpdate()) {
					me->channel_.send(work);
				}
				else {
					me->reply(work);
				}
			}	break;
			case aio::SEND: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGRespondent") << "failed to send message; " << nng_strerror(result);
					work->state = aio::RECV;
					nng_ctx_recv(work->ctx, work->aio);
					break;
				}
				work->state = aio::RECV;
				nng_ctx_recv(work->ctx, work->aio);
			}	break;
		}
	}
	void update() {
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
