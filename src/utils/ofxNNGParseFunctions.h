#pragma once

#include "ofFileUtils.h"
#include "nng.h"

namespace ofx {
namespace nng {
namespace util {
	template<typename T>
	inline bool parse(const nng_msg &src, T &dst) {
		return false;
	}
	template<>
	inline bool parse(const nng_msg &src, ofBuffer &dst) {
		auto body = nng_msg_body(const_cast<nng_msg*>(&src));
		auto len = nng_msg_len(&src);
		dst.set((char*)body, len);
		return true;
	}
}
}
}
