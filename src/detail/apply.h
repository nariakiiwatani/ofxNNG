#pragma once

namespace ofxNNG {
	namespace detail {
		template<typename F, typename T, std::size_t ...I>
		static auto tuple_apply_impl(F &&func, T &&t, nlohmann::detail::index_sequence<I...>)
		-> decltype(func(std::get<I>(t)...)) {
			return func(std::get<I>(t)...);
		}
		template<typename F, typename T, std::size_t N=std::tuple_size<T>::value>
		static auto tuple_apply(F &&func, T &&t)
		-> decltype(tuple_apply_impl(std::forward<F>(func), std::forward<T>(t), nlohmann::detail::make_index_sequence<N>{})) {
			return tuple_apply_impl(std::forward<F>(func), std::forward<T>(t), nlohmann::detail::make_index_sequence<N>{});
		}
		template<typename F, typename ...Args>
		struct apply_fun {
			auto operator()(F &&func, const ofxNNG::Message &msg)
			-> decltype(func(std::declval<Args>()...)) {
				return detail::tuple_apply(func, msg.get<std::tuple<Args...>>());
			}
		};
		template<typename F>
		struct apply_fun<F, ofxNNG::Message> {
			auto operator()(F &&func, const ofxNNG::Message &msg)
			-> decltype(func(std::declval<ofxNNG::Message>())) {
				return func(ofxNNG::Message(msg));
			}
		};
	}

	template<typename ...Args, typename F>
	static auto apply(F &&func, const ofxNNG::Message &msg)
	-> decltype(std::declval<detail::apply_fun<F, Args...>>()(func, msg)) {
		return detail::apply_fun<F, Args...>()(func, msg);
	}
}
