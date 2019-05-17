#pragma once

#include "ofFileUtils.h"
#include "nng.h"

namespace ofx {
namespace nng {
namespace util {
	template<typename T>
	inline T parse(const nng_msg &src) {
		return T();
	}
	template<>
	inline ofBuffer parse(const nng_msg &src) {
		auto body = nng_msg_body(const_cast<nng_msg*>(&src));
		auto len = nng_msg_len(&src);
		return ofBuffer((char*)body, len);
	}
}
}
}
