#pragma once

#include <stddef.h>
#include "nng.h"
#include "reqrep0/req.h"
#include "supplemental/util/platform.h"
#include "ofLog.h"
#include "ASyncWork.h"
#include "ofxNNGConvertFunctions.h"
#include "ofxNNGParseFunctions.h"

namespace ofx {
namespace nng {
class Req
{
public:
	struct Settings {
		std::string url;
		nng_dialer *dialer=nullptr;
		bool blocking=false;
		int max_queue=16;
	};
	bool setup(const Settings &s) {
		int result;
		result = nng_req0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGReq") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		int flags = 0;
		if(!s.blocking) flags |= NNG_FLAG_NONBLOCK;
		result = nng_dial(socket_, s.url.data(), s.dialer, flags);
		if(result != 0) {
			ofLogError("ofxNNGReq") << "failed to create dialer; " << nng_strerror(result);
			return false;
		}
		result = nng_mtx_alloc(&mtx_);
		if(result != 0) {
			ofLogError("ofxNNGReq") << "failed to create mutex; " << nng_strerror(result);
			return false;
		}
		work_.initialize(s.max_queue, &Req::async, this);
		return true;
	}
	template<typename Request, typename Response>
	bool send(const Request &req, std::function<void(const Response&)> callback) {
		aio::Work *work = nullptr;
		nng_mtx_lock(mtx_);
		work = work_.getUnused();
		nng_mtx_unlock(mtx_);
		if(!work) {
			ofLogWarning("ofxNNGReq") << "no unused work";
			return false;
		}
		nng_ctx_open(&work->ctx, socket_);
		nng_msg *msg;
		nng_msg_alloc(&msg, 0);
		if(!util::convert(req, msg)) {
			ofLogError("ofxNNGReq") << "failed to convert data";
			return false;
		}
		nng_aio_set_msg(work->aio, msg);
		nng_mtx_lock(mtx_);
		callback_[nng_ctx_id(work->ctx)] = [callback](nng_msg *msg) {
			callback(util::parse<ofBuffer>(msg));
		};
		nng_mtx_unlock(mtx_);
		work->state = aio::SEND;
		nng_ctx_send(work->ctx, work->aio);
		return true;
	}
private:
	nng_socket socket_;
	aio::WorkPool work_;
	std::map<int, std::function<void(nng_msg*)>> callback_;
	nng_mtx *mtx_;
	
	static void async(void *arg) {
		auto work = (aio::Work*)arg;
		auto me = (Req*)work->userdata;
		switch(work->state) {
			case aio::SEND: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGReq") << "failed to send message; " << nng_strerror(result);
					break;
				}
				work->state = aio::RECV;
				nng_ctx_recv(work->ctx, work->aio);
			}	break;
			case aio::RECV: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGReq") << "failed to receive message; " << nng_strerror(result);
					return;
				}
				auto msg = nng_aio_get_msg(work->aio);
				nng_mtx_lock(me->mtx_);
				me->callback_[nng_ctx_id(work->ctx)](msg);
				me->callback_.erase(nng_ctx_id(work->ctx));
				nng_mtx_unlock(me->mtx_);
				nng_msg_free(msg);
				nng_ctx_close(work->ctx);
				nng_mtx_lock(me->mtx_);
				work->release();
				nng_mtx_unlock(me->mtx_);
			}	break;
		}
	}
};
}}
