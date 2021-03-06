#pragma once

#include "detail/user_optimization.h"

#pragma mark - adl converter
namespace ofxNNG {
	
	namespace {
		template<typename T>
		static inline auto from_nng_msg(T &t, const Message &msg, std::size_t offset)
		-> decltype(::ofxNNG::basic_converter::converter<T>::from_msg(t,msg,offset)) {
			return ::ofxNNG::basic_converter::converter<T>::from_msg(t,msg,offset);
		}
		template<typename T>
		static inline auto append_to_nng_msg(Message &msg, const T &t)
		-> decltype(::ofxNNG::basic_converter::converter<T>::append_to_msg(msg,t)) {
			return ::ofxNNG::basic_converter::converter<T>::append_to_msg(msg,t);
		}
	}

	template<typename T, typename SFINAE=void>
	struct adl_converter {
		static inline auto from_msg(T &t, const Message &msg, std::size_t offset)
		-> decltype(from_nng_msg(t,msg,offset)) {
			return from_nng_msg(t,msg,offset);
		}
		static inline auto append_to_msg(Message &msg, const T &t)
		-> decltype(append_to_nng_msg(msg, t)) {
			return append_to_nng_msg(msg, t);
		}
	};

	namespace detail {
		template<typename T>
		struct has_member_converters {
		private:
			template<typename U,
				std::size_t(U::*)(const Message&,std::size_t)=&U::from_nng_msg,
				void(U::*)(Message&)const=&U::append_to_nng_msg
			>
			static std::true_type check(U*);
			static std::false_type check(...);
			public:
			static constexpr bool value = decltype(check((T*)(nullptr)))::value;
		};
	}
	template<typename T>
	struct adl_converter<T, typename std::enable_if<detail::has_member_converters<T>::value>::type> {
		static inline auto from_msg(T &t, const Message &msg, std::size_t offset)
		-> decltype(t.from_nng_msg(msg, offset)) {
			return t.from_nng_msg(msg, offset);
		}
		static inline auto append_to_msg(Message &msg, T &&t)
		-> decltype(t.append_to_nng_msg(msg)) {
			return t.append_to_nng_msg(msg);
		}
		static inline auto append_to_msg(Message &msg, const T &t)
		-> decltype(t.append_to_nng_msg(msg)) {
			return t.append_to_nng_msg(msg);
		}
	};
	
	template<typename T>
	struct adl_converter<T, typename std::enable_if<is_safe_to_use_memcpy<T>::value>::type> {
		static inline std::size_t from_msg(T &t, const Message &msg, std::size_t offset) {
			memcpy(&t, (const char*)msg.data()+offset, sizeof(T));
			return sizeof(T);
		}
		static inline void append_to_msg(Message &msg, T &&t) {
			msg.appendData(&t, sizeof(T));
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			msg.appendData(&t, sizeof(T));
		}
	};
}

#pragma mark - MEMBER_CONVERTER
#define OFX_NNG_MEMBER_CONVERTER_FROM(...) \
std::size_t from_nng_msg(const ofxNNG::Message &msg, std::size_t offset) { return msg.to(offset,__VA_ARGS__); }

#define OFX_NNG_MEMBER_CONVERTER_TO(...) \
void append_to_nng_msg(ofxNNG::Message &msg) const { ofxNNG::Message::appendTo(msg, __VA_ARGS__); }

#define OFX_NNG_MEMBER_CONVERTER(...) \
OFX_NNG_MEMBER_CONVERTER_FROM(__VA_ARGS__); \
OFX_NNG_MEMBER_CONVERTER_TO(__VA_ARGS__);


#pragma mark - PP_NARG
/*
 * The PP_NARG macro evaluates to the number of arguments that have been
 * passed to it.
 *
 * Laurent Deniau, "__VA_NARG__," 17 January 2006, <comp.std.c> (29 November 2007).
 */
#define EXPAND(x) x
#define PP_NARG(...)    PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_(...)   EXPAND(PP_ARG_N(__VA_ARGS__))

#define PP_ARG_N( \
_1, _2, _3, _4, _5, _6, _7, _8, _9,_10,  \
_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
_61,_62,_63,N,...) N

#define PP_RSEQ_N() \
63,62,61,60,                   \
59,58,57,56,55,54,53,52,51,50, \
49,48,47,46,45,44,43,42,41,40, \
39,38,37,36,35,34,33,32,31,30, \
29,28,27,26,25,24,23,22,21,20, \
19,18,17,16,15,14,13,12,11,10, \
9,8,7,6,5,4,3,2,1,0

#pragma mark - CAT
#define CAT(a,b) CAT_I(a,b)
#define CAT_I(a,b) a ## b

#pragma mark - GET_MEMBERS
#define GET_MEMBERS(type,...) CAT(GET_MEMBERS, PP_NARG(__VA_ARGS__))(type,__VA_ARGS__)
#define GET_MEMBERS0(type,...) 
#define GET_MEMBERS1(type,arg1,...)  type.arg1
#define GET_MEMBERS2(type,arg1,...)  GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS1(type,__VA_ARGS__))
#define GET_MEMBERS3(type,arg1,...)  GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS2(type,__VA_ARGS__))
#define GET_MEMBERS4(type,arg1,...)  GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS3(type,__VA_ARGS__))
#define GET_MEMBERS5(type,arg1,...)  GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS4(type,__VA_ARGS__))
#define GET_MEMBERS6(type,arg1,...)  GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS5(type,__VA_ARGS__))
#define GET_MEMBERS7(type,arg1,...)  GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS6(type,__VA_ARGS__))
#define GET_MEMBERS8(type,arg1,...)  GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS7(type,__VA_ARGS__))
#define GET_MEMBERS9(type,arg1,...)  GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS8(type,__VA_ARGS__))
#define GET_MEMBERS10(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS9(type,__VA_ARGS__))
#define GET_MEMBERS11(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS10(type,__VA_ARGS__))
#define GET_MEMBERS12(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS11(type,__VA_ARGS__))
#define GET_MEMBERS13(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS12(type,__VA_ARGS__))
#define GET_MEMBERS14(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS13(type,__VA_ARGS__))
#define GET_MEMBERS15(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS14(type,__VA_ARGS__))
#define GET_MEMBERS16(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS15(type,__VA_ARGS__))
#define GET_MEMBERS17(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS16(type,__VA_ARGS__))
#define GET_MEMBERS18(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS17(type,__VA_ARGS__))
#define GET_MEMBERS19(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS18(type,__VA_ARGS__))
#define GET_MEMBERS20(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS19(type,__VA_ARGS__))
#define GET_MEMBERS21(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS20(type,__VA_ARGS__))
#define GET_MEMBERS22(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS21(type,__VA_ARGS__))
#define GET_MEMBERS23(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS22(type,__VA_ARGS__))
#define GET_MEMBERS24(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS23(type,__VA_ARGS__))
#define GET_MEMBERS25(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS24(type,__VA_ARGS__))
#define GET_MEMBERS26(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS25(type,__VA_ARGS__))
#define GET_MEMBERS27(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS26(type,__VA_ARGS__))
#define GET_MEMBERS28(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS27(type,__VA_ARGS__))
#define GET_MEMBERS29(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS28(type,__VA_ARGS__))
#define GET_MEMBERS30(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS29(type,__VA_ARGS__))
#define GET_MEMBERS31(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS30(type,__VA_ARGS__))
#define GET_MEMBERS32(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS31(type,__VA_ARGS__))
#define GET_MEMBERS33(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS32(type,__VA_ARGS__))
#define GET_MEMBERS34(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS33(type,__VA_ARGS__))
#define GET_MEMBERS35(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS34(type,__VA_ARGS__))
#define GET_MEMBERS36(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS35(type,__VA_ARGS__))
#define GET_MEMBERS37(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS36(type,__VA_ARGS__))
#define GET_MEMBERS38(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS37(type,__VA_ARGS__))
#define GET_MEMBERS39(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS38(type,__VA_ARGS__))
#define GET_MEMBERS40(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS39(type,__VA_ARGS__))
#define GET_MEMBERS41(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS40(type,__VA_ARGS__))
#define GET_MEMBERS42(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS41(type,__VA_ARGS__))
#define GET_MEMBERS43(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS42(type,__VA_ARGS__))
#define GET_MEMBERS44(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS43(type,__VA_ARGS__))
#define GET_MEMBERS45(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS44(type,__VA_ARGS__))
#define GET_MEMBERS46(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS45(type,__VA_ARGS__))
#define GET_MEMBERS47(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS46(type,__VA_ARGS__))
#define GET_MEMBERS48(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS47(type,__VA_ARGS__))
#define GET_MEMBERS49(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS48(type,__VA_ARGS__))
#define GET_MEMBERS50(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS49(type,__VA_ARGS__))
#define GET_MEMBERS51(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS50(type,__VA_ARGS__))
#define GET_MEMBERS52(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS51(type,__VA_ARGS__))
#define GET_MEMBERS53(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS52(type,__VA_ARGS__))
#define GET_MEMBERS54(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS53(type,__VA_ARGS__))
#define GET_MEMBERS55(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS54(type,__VA_ARGS__))
#define GET_MEMBERS56(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS55(type,__VA_ARGS__))
#define GET_MEMBERS57(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS56(type,__VA_ARGS__))
#define GET_MEMBERS58(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS57(type,__VA_ARGS__))
#define GET_MEMBERS59(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS58(type,__VA_ARGS__))
#define GET_MEMBERS60(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS59(type,__VA_ARGS__))
#define GET_MEMBERS61(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS60(type,__VA_ARGS__))
#define GET_MEMBERS62(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS61(type,__VA_ARGS__))
#define GET_MEMBERS63(type,arg1,...) GET_MEMBERS1(type,arg1),EXPAND(GET_MEMBERS62(type,__VA_ARGS__))

#pragma mark - ADL_CONVERTER
#define OFX_NNG_ADL_CONVERTER_FROM(Type,...) \
static inline std::size_t from_msg(Type &type, const ofxNNG::Message &msg, std::size_t offset) { return msg.to(offset, GET_MEMBERS(type, __VA_ARGS__)); }

#define OFX_NNG_ADL_CONVERTER_TO(Type,...) \
static inline void append_to_msg(ofxNNG::Message &msg, Type type) { ofxNNG::Message::appendTo(msg, GET_MEMBERS(type, __VA_ARGS__)); }

#define OFX_NNG_ADL_CONVERTER(Type,...) \
template<> struct adl_converter<Type> { \
OFX_NNG_ADL_CONVERTER_FROM(Type, __VA_ARGS__); \
OFX_NNG_ADL_CONVERTER_TO(Type, __VA_ARGS__); \
}
