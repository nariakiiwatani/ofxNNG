#pragma once

#include <stddef.h>
#include "nng.h"
#include "reqrep0/req.h"
#include "supplemental/util/platform.h"
#include "ofLog.h"
#include "ASyncWork.h"
#include "ofxNNGMessage.h"
#include "ofxNNGNode.h"

namespace ofxNNG {
class Req : public Node
{
public:
	struct Settings {
		int max_queue=16;
		nng_duration timeout_milliseconds=NNG_DURATION_DEFAULT;
		bool allow_callback_from_other_thread=false;
	};
	bool setup(const Settings &s) {
		int result;
		result = nng_req0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGReq") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		result = nng_mtx_alloc(&work_mtx_);
		if(result != 0) {
			ofLogError("ofxNNGReq") << "failed to create mutex; " << nng_strerror(result);
			return false;
		}
		result = nng_mtx_alloc(&callback_mtx_);
		if(result != 0) {
			ofLogError("ofxNNGReq") << "failed to create mutex; " << nng_strerror(result);
			return false;
		}
		timeout_ = s.timeout_milliseconds;
		async_ = s.allow_callback_from_other_thread;
		if(!async_) {
			ofAddListener(ofEvents().update, this, &Req::update);
		}
		work_.initialize(s.max_queue, &Req::async, this);
		return true;
	}
	template<typename T>
	bool send(Message msg, std::function<void(T&&)> callback) {
		aio::Work *work = nullptr;
		nng_mtx_lock(work_mtx_);
		work = work_.getUnused();
		nng_mtx_unlock(work_mtx_);
		if(!work) {
			ofLogWarning("ofxNNGReq") << "no unused work";
			return false;
		}
		nng_ctx_open(&work->ctx, socket_);
		nng_aio_set_msg(work->aio, msg);
		nng_mtx_lock(callback_mtx_);
		callback_[nng_ctx_id(work->ctx)] = [callback](Message msg) {
			callback(msg.get<T>());
		};
		nng_mtx_unlock(callback_mtx_);
		nng_aio_set_timeout(work->aio, timeout_);
		work->state = aio::SEND;
		nng_ctx_send(work->ctx, work->aio);
		msg.setSentFlag();
		return true;
	}
private:
	aio::WorkPool work_;
	std::map<int, std::function<void(Message)>> callback_;
	nng_mtx *work_mtx_, *callback_mtx_;
	bool async_;
	ofThreadChannel<aio::Work*> channel_;
	nng_duration timeout_;
	
	static void async(void *arg) {
		auto work = (aio::Work*)arg;
		auto me = (Req*)work->userdata;
		switch(work->state) {
			case aio::SEND: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGReq") << "failed to send message; " << nng_strerror(result);
					me->closeWork(work);
					break;
				}
				work->state = aio::RECV;
				nng_ctx_recv(work->ctx, work->aio);
			}	break;
			case aio::RECV: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGReq") << "failed to receive message; " << nng_strerror(result);
					me->closeWork(work);
					return;
				}
				if(me->async_) {
					me->onReceiveReply(work);
				}
				else {
					me->channel_.send(work);
				}
			}	break;
		}
	}
	void update(ofEventArgs&) {
		aio::Work *work;
		while(channel_.tryReceive(work)) {
			onReceiveReply(work);
		}
	}
	void onReceiveReply(aio::Work *work) {
		Message msg(nng_aio_get_msg(work->aio));
		nng_mtx_lock(callback_mtx_);
		callback_[nng_ctx_id(work->ctx)](msg);
		callback_.erase(nng_ctx_id(work->ctx));
		nng_mtx_unlock(callback_mtx_);
		closeWork(work);
	}
	void closeWork(aio::Work *work) {
		nng_ctx_close(work->ctx);
		nng_mtx_lock(work_mtx_);
		work->release();
		nng_mtx_unlock(work_mtx_);
	}
};
}
