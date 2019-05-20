#pragma once

#include "ofFileUtils.h"
#include "nng.h"

namespace ofx {
namespace nng {
namespace util {
	template<typename T>
	inline T parse(const nng_msg *src) {
		return T();
	}
	inline const char* parse(const nng_msg *src) {
		return (char*)nng_msg_body(const_cast<nng_msg*>(src));
	}
	template<>
	inline std::string parse(const nng_msg *src) {
		auto body = nng_msg_body(const_cast<nng_msg*>(src));
		auto len = nng_msg_len(src);
		return std::string((char*)body, len);
	}
	template<>
	inline ofBuffer parse(const nng_msg *src) {
		auto body = nng_msg_body(const_cast<nng_msg*>(src));
		auto len = nng_msg_len(src);
		return ofBuffer((char*)body, len);
	}
}
}
}
