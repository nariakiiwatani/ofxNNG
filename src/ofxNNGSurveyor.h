#pragma once

#include <stddef.h>
#include "nng.h"
#include "survey0/survey.h"
#include "supplemental/util/platform.h"
#include "ofLog.h"
#include "ASyncWork.h"
#include "ofxNNGConvertFunctions.h"
#include "ofxNNGParseFunctions.h"
#include "ofxNNGNode.h"

namespace ofx {
namespace nng {
class Surveyor : public Node
{
public:
	struct Settings {
		int max_queue=16;
		bool allow_callback_from_other_thread=false;
	};
	bool setup(const Settings &s) {
		int result;
		result = nng_surveyor0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGSurveyor") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		result = nng_mtx_alloc(&work_mtx_);
		if(result != 0) {
			ofLogError("ofxNNGSurveyor") << "failed to create mutex; " << nng_strerror(result);
			return false;
		}
		result = nng_mtx_alloc(&callback_mtx_);
		if(result != 0) {
			ofLogError("ofxNNGSurveyor") << "failed to create mutex; " << nng_strerror(result);
			return false;
		}
		async_ = s.allow_callback_from_other_thread;
		if(!async_) {
			ofAddListener(ofEvents().update, this, &Surveyor::update);
		}
		work_.initialize(s.max_queue, &Surveyor::async, this);
		return true;
	}
	template<typename Request, typename Response>
	bool send(const Request &req, std::function<void(const Response&)> callback) {
		aio::Work *work = nullptr;
		nng_mtx_lock(work_mtx_);
		work = work_.getUnused();
		nng_mtx_unlock(work_mtx_);
		if(!work) {
			ofLogWarning("ofxNNGSurveyor") << "no unused work";
			return false;
		}
		nng_ctx_open(&work->ctx, socket_);
		nng_msg *msg;
		nng_msg_alloc(&msg, 0);
		if(!util::convert(req, msg)) {
			ofLogError("ofxNNGSurveyor") << "failed to convert data";
			nng_mtx_lock(work_mtx_);
			work->release();
			nng_mtx_unlock(work_mtx_);
			return false;
		}
		nng_aio_set_msg(work->aio, msg);
		nng_mtx_lock(callback_mtx_);
		callback_[work] = [callback](nng_msg *msg) {
			callback(util::parse<ofBuffer>(msg));
		};
		nng_mtx_unlock(callback_mtx_);
		work->state = aio::SEND;
		nng_ctx_send(work->ctx, work->aio);
		return true;
	}
private:
	aio::WorkPool work_;
	std::map<aio::Work*, std::function<void(nng_msg*)>> callback_;
	nng_mtx *work_mtx_, *callback_mtx_;
	bool async_;
	struct AsyncWork {
		AsyncWork() {}
		AsyncWork(aio::Work *w):work(w) {
			auto src = nng_aio_get_msg(work->aio);
			nng_msg_dup(&msg, src);
		}
		aio::Work *work;
		nng_msg *msg;
	};
	ofThreadChannel<AsyncWork> channel_;
	
	static void async(void *arg) {
		auto work = (aio::Work*)arg;
		auto me = (Surveyor*)work->userdata;
		switch(work->state) {
			case aio::SEND: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGSurveyor") << "failed to send message; " << nng_strerror(result);
					nng_mtx_lock(me->work_mtx_);
					work->release();
					nng_mtx_unlock(me->work_mtx_);
					break;
				}
				work->state = aio::RECV;
				nng_ctx_recv(work->ctx, work->aio);
			}	break;
			case aio::RECV: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGSurveyor") << "failed to receive message; " << nng_strerror(result);
					nng_mtx_lock(me->work_mtx_);
					work->release();
					nng_mtx_unlock(me->work_mtx_);
					return;
				}
				if(me->async_) {
					me->onReceiveReply(work);
				}
				else {
					me->channel_.send({work});
				}
				nng_msg_free(nng_aio_get_msg(work->aio));
				nng_ctx_recv(work->ctx, work->aio);
			}	break;
		}
	}
	void update(ofEventArgs&) {
		AsyncWork work;
		while(channel_.tryReceive(work)) {
			onReceiveReply(work);
			nng_msg_free(work.msg);
		}
	}
	void onReceiveReply(const AsyncWork &work) {
		nng_mtx_lock(callback_mtx_);
		callback_[work.work](work.msg);
		nng_mtx_unlock(callback_mtx_);
	}
};
}}
