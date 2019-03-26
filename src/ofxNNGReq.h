#pragma once

#include <stddef.h>
#include "nng.h"
#include "reqrep0/req.h"
#include "supplemental/util/platform.h"
#include "ofLog.h"

namespace ofx {
namespace nng {
class Req
{
public:
	struct Settings {
		std::string url;
		nng_dialer *dialer=nullptr;
		int flags=NNG_FLAG_NONBLOCK;
	};
	bool setup(const Settings &s) {
		int result;
		result = nng_req0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGReq") << "failed to open socket";
			return false;
		}
		result = nng_dial(socket_, s.url.data(), s.dialer, s.flags);
		if(result != 0) {
			ofLogError("ofxNNGReq") << "failed to create dialer";
			fatal("nng_dial", result);
			return false;
		}
		nng_mtx_alloc(&lock_wait_);
		nng_mtx_alloc(&lock_run_);
		is_thread_running_ = true;
		nng_thread_create(&thread_, Req::threadedFunction, this);
		return true;
	}
	virtual ~Req() {
		if(thread_) {
			nng_mtx_lock(lock_run_);
			is_thread_running_ = false;
			nng_mtx_unlock(lock_run_);
			nng_thread_destroy(thread_);
		}
		if(lock_wait_) {
			nng_mtx_free(lock_wait_);
		}
		if(lock_run_) {
			nng_mtx_free(lock_run_);
		}
	};
	void send(void *msg, size_t len, int flags) {
		int result = nng_send(socket_, msg, len, flags);
		if(result != 0) {
			ofLogError("ofxNNGReq") << "failed to send message";
			return false;
		}
		nng_mtx_lock(lock_wait_);
		++waiting_msg_count_;
		nng_mtx_unlock(lock_wait_);
	}
private:
	static void fatal(const char *func, int rv)
	{
		fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
		exit(1);
	}
	nng_socket socket_;
	nng_thread *thread_=nullptr;
	int waiting_msg_count_=0;
	nng_mtx *lock_wait_, *lock_run_;
	bool is_thread_running_=false;
	static void threadedFunction(void *arg) {
		auto me = (Req*)arg;
		void *msg = nullptr;
		int flags = NNG_FLAG_NONBLOCK | NNG_FLAG_ALLOC;
		for(;;) {
			nng_mtx_lock(me->lock_run_);
			if(!me->is_thread_running_) {
				nng_mtx_unlock(me->lock_run_);
				return;
			}
			nng_mtx_unlock(me->lock_run_);
			nng_mtx_lock(me->lock_wait_);
			int waiting = me->waiting_msg_count_;
			nng_mtx_unlock(me->lock_wait_);
			if(waiting == 0) {
				continue;
			}
			size_t len=256;
			int result = nng_recv(me->socket_, &msg, &len, flags);
			if(result != 0) {
				if(result == NNG_EAGAIN) {
					continue;
				}
				ofLogError("ofxNNGReq") << "failed to receive message";
				fatal("nng_recv", result);
				return false;
			}
			ofLogNotice("ofxNNGReq") << "receive response: " << std::string((char*)msg, len);
			nng_mtx_lock(me->lock_wait_);
			--me->waiting_msg_count_;
			nng_mtx_unlock(me->lock_wait_);
		}
	}
};
}}
