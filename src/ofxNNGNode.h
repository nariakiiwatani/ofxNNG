#pragma once

#include "nng.h"

namespace ofx {
namespace nng {
class Node
{
public:
	bool dial(const std::string &url, bool blocking=false) {
		int flags = 0;
		if(!blocking) flags |= NNG_FLAG_NONBLOCK;
		nng_dialer dialer;
		auto result = nng_dial(socket_, url.data(), &dialer, flags);
		if(result != 0) {
			return false;
		}
		dialer_.insert(std::make_pair(url, dialer));
		return true;
	}
	bool listen(const std::string &url, bool blocking=false) {
		int flags = 0;
		if(!blocking) flags |= NNG_FLAG_NONBLOCK;
		nng_listener listener;
		auto result = nng_listen(socket_, url.data(), &listener, flags);
		if(result != 0) {
			return false;
		}
		listener_.insert(std::make_pair(url, listener));
		return true;
	}
	void close(const std::string &url) {
		auto dialer = dialer_.find(url);
		if(dialer != std::end(dialer_)) {
			nng_dialer_close(dialer->second);
			dialer_.erase(dialer);
		}
		auto listener = listener_.find(url);
		if(listener != std::end(listener_)) {
			nng_listener_close(listener->second);
			listener_.erase(listener);
		}
	}
	void close() {
		while(!dialer_.empty()) {
			close(dialer_.begin()->first);
		}
		while(!listener_.empty()) {
			close(listener_.begin()->first);
		}
	}
protected:
	nng_socket socket_;
	std::unordered_multimap<std::string, nng_dialer> dialer_;
	std::unordered_multimap<std::string, nng_listener> listener_;
};
}}
