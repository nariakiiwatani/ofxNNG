#pragma once

#include "nng.h"
#include <type_traits>
#include <string>
#include <vector>
#include <deque>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include "ofFileUtils.h"
#include "ofJson.h"
#include "detail/user_optimization.h"

namespace ofxNNG {
	namespace {
		using size_type = typename Message::size_type;
	}

namespace basic_converter {
	
#pragma mark - base template
	template<typename T, typename SFINAE=void>
	struct converter {
		static inline size_type from_msg(T &t, const Message &msg, size_type offset);
		static inline void append_to_msg(Message &msg, const T &t);
	};

#pragma mark - ofxNNGMessage
	template<>
	struct converter<Message> {
		using T = Message;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			t.clear();
			auto pos = offset;
			size_type size;
			pos += msg.to(pos, size);
			t.appendData((const char*)msg.data()+pos, size);
			pos += size;
			return pos-offset;
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			msg.append(t.size());
			msg.appendData(t.data(), t.size());
		}
	};

#pragma mark - arithmetic
	template<typename T>
	struct converter<T, typename std::enable_if<std::is_arithmetic<T>::value>::type> {
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			size_type size = sizeof(T);
			Message copy;
			copy.appendData((const char*)msg.data()+offset, size);
			switch(size) {
				case 1: t = *reinterpret_cast<uint8_t*>(nng_msg_body(copy)); nng_msg_trim(copy, size); break;
				case 2: nng_msg_trim_u16(copy, (uint16_t*)&t); break;
				case 4: nng_msg_trim_u32(copy, (uint32_t*)&t); break;
				case 8: nng_msg_trim_u64(copy, (uint64_t*)&t); break;
			}
			return size;
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			size_type size = sizeof(T);
			switch(size) {
				case 1: nng_msg_append(msg, &t, size); break;
				case 2: nng_msg_append_u16(msg, *reinterpret_cast<const uint16_t*>(&t)); break;
				case 4: nng_msg_append_u32(msg, *reinterpret_cast<const uint32_t*>(&t)); break;
				case 8: nng_msg_append_u64(msg, *reinterpret_cast<const uint64_t*>(&t)); break;
			}
		}
	};

	namespace detail {
		namespace cast {
			template<typename T, typename U>
			struct c_style_cast_converter {
				static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
					return converter<U>::from_msg((U&)t,msg,offset);
				}
				static inline void append_to_msg(Message &msg, const T &t) {
					converter<U>::append_to_msg(msg, (const U&)t);
				}
			};
		}
	}
#pragma mark - nng_pipe
	template<>
	struct converter<nng_pipe> {
		using T = nng_pipe;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			return converter<int>::from_msg((int&)t.id,msg,offset);
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			converter<int>::append_to_msg(msg,nng_pipe_id(t));
		}
	};
	
#pragma mark - enum
	template<typename T>
	struct converter<T, typename std::enable_if<std::is_enum<T>::value>::type> : detail::cast::c_style_cast_converter<T, typename std::underlying_type<T>::type>{};
	
#pragma mark - container
	namespace detail {
		namespace container {
			template<typename T>
			struct common_converter {
				static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
					auto pos = offset;
					size_type size;
					pos += msg.to(pos, size);
					for(auto i = 0; i < size; ++i) {
						typename T::value_type v;
						pos += msg.to(pos, v);
						t.insert(std::end(t), std::move(v));
					}
					return pos-offset;
				}
				static inline void append_to_msg(Message &msg, const T &t) {
					msg.append(t.size());
					for(auto &&val : t) {
						msg.append(val);
					}
				}
			};
		}
	}
#define CONTAINER_COMMON_CONVERTER(Type) \
template<typename ...T> struct converter<Type<T...>> : detail::container::common_converter<Type<T...>>{}
//	CONTAINER_COMMON_CONVERTER(std::vector);
	CONTAINER_COMMON_CONVERTER(std::list);
	CONTAINER_COMMON_CONVERTER(std::forward_list);
	CONTAINER_COMMON_CONVERTER(std::deque);
	CONTAINER_COMMON_CONVERTER(std::priority_queue);
	CONTAINER_COMMON_CONVERTER(std::set);
	CONTAINER_COMMON_CONVERTER(std::multiset);
	CONTAINER_COMMON_CONVERTER(std::unordered_set);
	CONTAINER_COMMON_CONVERTER(std::unordered_multiset);
	CONTAINER_COMMON_CONVERTER(std::map);
	CONTAINER_COMMON_CONVERTER(std::multimap);
	CONTAINER_COMMON_CONVERTER(std::unordered_map);
	CONTAINER_COMMON_CONVERTER(std::unordered_multimap);
//	CONTAINER_COMMON_CONVERTER(std::array);
//	CONTAINER_COMMON_CONVERTER(std::bitset);
	CONTAINER_COMMON_CONVERTER(std::stack);
	CONTAINER_COMMON_CONVERTER(std::queue);
#undef COMMON_CONTAINER_CONVERTER

#pragma mark - pair
	template<typename U, typename V>
	struct converter<std::pair<U,V>> {
		using T = std::pair<U,V>;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			auto pos = offset;
			pos += msg.to(pos,
						  const_cast<typename std::remove_const<U>::type&>(t.first),
						  const_cast<typename std::remove_const<V>::type&>(t.second)
						  );
			return pos-offset;
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			msg.append(
					   const_cast<typename std::remove_const<U>::type&>(t.first),
					   const_cast<typename std::remove_const<V>::type&>(t.second)
					   );
		}
	};
#pragma mark - tuple
	namespace detail {
		namespace tuple {
			template<typename T, std::size_t ...I>
			auto from_msg_impl(size_type offset, const Message &msg, T &t, nlohmann::detail::index_sequence<I...>)
			-> decltype(msg.to(offset,const_cast<typename std::remove_cv<decltype(std::get<I>(t))>::type>(std::get<I>(t))...)) {
				return msg.to(offset, const_cast<typename std::remove_cv<decltype(std::get<I>(t))>::type>(std::get<I>(t))...);
			}
			template<typename T, std::size_t N=std::tuple_size<T>::value>
			auto from_msg(size_type offset, const Message &msg, T &t)
			-> decltype(from_msg_impl(offset, msg, t, nlohmann::detail::make_index_sequence<N>{})) {
				return from_msg_impl(offset, msg, t, nlohmann::detail::make_index_sequence<N>{});
			}
			template<typename T, std::size_t ...I>
			auto append_to_impl(Message &msg, const T &t, nlohmann::detail::index_sequence<I...>)
			-> decltype(msg.append(std::get<I>(t)...)) {
				return msg.append(std::get<I>(t)...);
			}
			template<typename T, std::size_t N=std::tuple_size<T>::value>
			auto append_to(Message &msg, const T &t)
			-> decltype(append_to_impl(msg, t, nlohmann::detail::make_index_sequence<N>{})) {
				return append_to_impl(msg, t, nlohmann::detail::make_index_sequence<N>{});
			}
		}
	}
	template<typename ...U>
	struct converter<std::tuple<U...>> {
		using T = std::tuple<U...>;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			auto pos = offset;
			pos += detail::tuple::from_msg(pos, msg, t);
			return pos-offset;
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			detail::tuple::append_to(msg, t);
		}
	};

#pragma mark - array
	namespace detail {

		namespace array {
			template<typename T>
			struct size_of { static constexpr std::size_t value = sizeof(T); };
			
			template<typename T>
			struct should_use_memcpy {
				static constexpr bool value = false
				|| size_of<T>::value==1
				|| ::ofxNNG::is_safe_to_use_memcpy<T>::value
#if defined(TARGET_OSX)
				|| std::is_trivially_copyable<T>::value
#endif
				;
			};

			template<typename T>
			static inline auto from_msg(T &t, const Message &msg, size_type offset, std::size_t length)
			-> typename std::enable_if<should_use_memcpy<decltype(t[0])>::value, size_type>::type {
				std::size_t size = sizeof(decltype(t[0]));
				memcpy(&t, (const char*)msg.data()+offset, size*length);
				return size*length;
			}
			template<typename T>
			static inline auto from_msg(T *t, const Message &msg, size_type offset, std::size_t length)
			-> typename std::enable_if<should_use_memcpy<T>::value, size_type>::type {
				std::size_t size = sizeof(T);
				memcpy(t, (const char*)msg.data()+offset, size*length);
				return size*length;
			}
			template<typename T>
			static inline auto from_msg(T &t, const Message &msg, size_type offset, std::size_t length)
			-> typename std::enable_if<!should_use_memcpy<decltype(t[0])>::value, size_type>::type {
				auto pos = offset;
				for(auto i = 0; i < length; ++i) {
					pos += msg.to(pos, t[i]);
				}
				return pos - offset;
			}
			template<typename T>
			static inline auto from_msg(T *t, const Message &msg, size_type offset, std::size_t length)
			-> typename std::enable_if<!should_use_memcpy<T>::value, size_type>::type {
				auto pos = offset;
				for(auto i = 0; i < length; ++i) {
					pos += msg.to(pos, t[i]);
				}
				return pos - offset;
			}
			template<typename T>
			static inline auto append_to_msg(Message &msg, T &&t, std::size_t length)
			-> typename std::enable_if<should_use_memcpy<decltype(t[0])>::value>::type {
				std::size_t size = sizeof(decltype(t[0]));
				msg.appendData(&t, size*length);
			}
			template<typename T>
			static inline auto append_to_msg(Message &msg, const T *t, std::size_t length)
			-> typename std::enable_if<should_use_memcpy<T>::value>::type {
				std::size_t size = sizeof(T);
				msg.appendData(t, size*length);
			}
			template<typename T>
			static inline auto append_to_msg(Message &msg, T &&t, std::size_t length)
			-> typename std::enable_if<!should_use_memcpy<decltype(t[0])>::value>::type {
				for(auto i = 0; i < length; ++i) {
					msg.append(t[i]);
				}
			}
			template<typename T>
			static inline auto append_to_msg(Message &msg, const T *t, std::size_t length)
			-> typename std::enable_if<!should_use_memcpy<T>::value>::type {
				for(auto i = 0; i < length; ++i) {
					msg.append(t[i]);
				}
			}

			template<typename T, size_type L>
			struct without_length_converter {
				static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
					return array::from_msg(t,msg,offset,L);
				}
				static inline void append_to_msg(Message &msg, const T &t) {
					array::append_to_msg(msg,t,L);
				}
			};
			template<typename T, size_type L>
			struct with_length_converter {
				static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
					auto pos = offset;
					size_type length;
					pos += msg.to(pos, length);
					if(length != L) {
						ofLogWarning("ofxNNG::detail::array::with_length_converter") << "length mismatch";
					}
					pos += array::from_msg(t,msg,pos,length);
					return pos-offset;
				}
				static inline void append_to_msg(Message &msg, const T &t) {
					msg.append(L);
					array::append_to_msg(msg,t,L);
				}
			};
		}
	}
#pragma mark - std::array
	template<typename T, size_type L>
	struct converter<std::array<T,L>> : detail::array::without_length_converter<std::array<T,L>,L>{};
	
#pragma mark - native array
	namespace detail {
		template<typename T>
		struct length_of;
		template<typename T, std::size_t L>
		struct length_of<T[L]> {
			static const std::size_t value = L;
		};
	}
	template<typename T>
	struct converter<T, typename std::enable_if<std::is_array<T>::value>::type> : detail::array::with_length_converter<T, detail::length_of<T>::value>{};

#pragma mark - std::vector
	template<typename ...U>
	struct converter<std::vector<U...>> {
		using T = std::vector<U...>;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			auto pos = offset;
			size_type size;
			pos += msg.to(pos, size);
			t.resize(size);
			pos += detail::array::from_msg(&t[0],msg,pos,size);
			return pos-offset;
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			msg.append(t.size());
			detail::array::append_to_msg(msg,t.data(),t.size());
		}
	};
#pragma mark - std::string
	template<typename ...U>
	struct converter<std::basic_string<U...>> {
		using T = std::basic_string<U...>;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			auto pos = offset;
			size_type size;
			pos += msg.to(pos, size);
			t.resize(size);
			pos += detail::array::from_msg(&t[0],msg,pos,size);
			return pos-offset;
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			msg.append(t.size());
			detail::array::append_to_msg(msg,t.data(),t.size());
		}
	};
	
#pragma mark - glm
	template<glm::length_t L, typename T, glm::qualifier Q>
	struct converter<glm::vec<L,T,Q>> : detail::array::without_length_converter<glm::vec<L,T,Q>,L>{};

	template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
	struct converter<glm::mat<C,R,T,Q>> : detail::array::without_length_converter<glm::mat<C,R,T,Q>,C>{};

	template<typename T, glm::qualifier Q>
#if GLM_HAS_CONSTEXPR
	struct converter<glm::qua<T,Q>> : detail::array::without_length_converter<glm::qua<T,Q>,glm::qua<T,Q>::length()>{};
#else 
	struct converter<glm::qua<T,Q>> : detail::array::without_length_converter<glm::qua<T,Q>,4>{};
#endif
	
#pragma mark - of types
	template<> struct converter<ofVec2f> : detail::cast::c_style_cast_converter<ofVec2f, glm::vec2>{};
	template<> struct converter<ofVec3f> : detail::cast::c_style_cast_converter<ofVec3f, glm::vec3>{};
	template<> struct converter<ofVec4f> : detail::cast::c_style_cast_converter<ofVec4f, glm::vec4>{};
	template<> struct converter<ofMatrix3x3> : detail::cast::c_style_cast_converter<ofMatrix3x3, glm::mat3>{};
	template<> struct converter<ofMatrix4x4> : detail::cast::c_style_cast_converter<ofMatrix4x4, glm::mat4>{};
	template<> struct converter<ofQuaternion> : detail::cast::c_style_cast_converter<ofQuaternion, glm::quat>{};
	template<typename PixelType> struct converter<ofColor_<PixelType>> : detail::array::without_length_converter<ofColor_<PixelType>, 4>{};

	template<>
	struct converter<ofRectangle> {
		using T = ofRectangle;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			return msg.to(offset, t.x, t.y, t.width, t.height);
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			msg.append(t.x, t.y, t.width, t.height);
		}
	};
	template<>
	struct converter<ofBuffer> {
		using T = ofBuffer;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			auto pos = offset;
			size_type size;
			pos += msg.to(pos, size);
			auto data = (const char*)msg.data();
			t.set(data+pos, size);
			return pos+size-offset;
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			msg.append(t.size());
			msg.appendData(t.getData(), t.size());
		}
	};
	template<>
	struct converter<ofJson> {
		using T = ofJson;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			auto pos = offset;
			size_type size;
			pos += msg.to(pos, size);
			auto data = (const char*)msg.data() + pos;
			t = ofJson::parse(data, data+size);
			return pos+size-offset;
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			msg.append(t.dump());
		}
	};
	template<>
	struct converter<ofNode> {
		using T = ofNode;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			auto pos = offset;
			glm::vec3 position, scale;
			glm::quat orientation;
			pos += msg.to(position, position, scale, orientation);
			t.setPosition(position);
			t.setScale(scale);
			t.setOrientation(orientation);
			return pos-offset;
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			msg.append(t.getPosition(), t.getScale(), t.getOrientationQuat());
		}
	};
	template<>
	struct converter<ofCamera> {
		using T = ofCamera;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			auto pos = offset;
			pos += converter<ofNode>::from_msg(t, msg, pos);
			float fov, near_clip, far_clip, aspect_ratio;
			glm::vec2 lens_offset;
			bool is_ortho, is_force_aspect_ratio, is_v_flipped;
			pos += msg.to(pos, fov, near_clip, far_clip, lens_offset, is_force_aspect_ratio, aspect_ratio, is_v_flipped);
			is_ortho ? t.enableOrtho() : t.disableOrtho();
			t.setFov(fov);
			t.setNearClip(near_clip);
			t.setFarClip(far_clip);
			t.setLensOffset(lens_offset);
			t.setAspectRatio(aspect_ratio);
			t.setForceAspectRatio(is_force_aspect_ratio);
			t.setVFlip(is_v_flipped);
			return pos-offset;
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			converter<ofNode>::append_to_msg(msg, t);
			msg.append(t.getOrtho(),
					   t.getFov(),
					   t.getNearClip(),
					   t.getFarClip(),
					   t.getLensOffset(),
					   t.getForceAspectRatio(),
					   t.getAspectRatio(),
					   t.isVFlipped());
		}
	};
	template<typename ...U>
	struct converter<ofMesh_<U...>> {
		using T = ofMesh_<U...>;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			auto pos = offset;
			ofPrimitiveMode mode;
			bool use_colors, use_texcoords, use_normals, use_indices;
			pos += msg.to(pos,
						  mode,
						  t.getVertices(),
						  use_colors,
						  t.getColors(),
						  use_texcoords,
						  t.getTexCoords(),
						  use_normals,
						  t.getNormals(),
						  use_indices,
						  t.getIndices());
			t.setMode(mode);
			use_colors ? t.enableColors() : t.disableColors();
			use_texcoords ? t.enableTextures() : t.disableTextures();
			use_normals ? t.enableNormals() : t.disableNormals();
			use_indices ? t.enableIndices() : t.disableIndices();
			return pos-offset;
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			msg.append(t.getMode(),
					   t.getVertices(),
					   t.usingColors(),
					   t.getColors(),
					   t.usingTextures(),
					   t.getTexCoords(),
					   t.usingNormals(),
					   t.getNormals(),
					   t.usingIndices(),
					   t.getIndices());
		}
	};
	template<class V, class N, class C, class TC>
	struct converter<ofMeshFace_<V,N,C,TC>> {
		using T = ofMeshFace_<V,N,C,TC>;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			auto pos = offset;
			std::vector<V> vertices;
			std::vector<C> colors;
			std::vector<N> normals;
			std::vector<TC> texcoords;
			pos += msg.to(vertices, colors, normals, texcoords);
			t.setHasColors(false);
			t.setHasNormals(false);
			t.setHasTexcoords(false);
			for(ofIndexType i = 0; i < vertices.size(); ++i) { t.setVertex(i, vertices[i]); }
			for(ofIndexType i = 0; i < colors.size(); ++i) { t.setColor(i, colors[i]); }
			for(ofIndexType i = 0; i < normals.size(); ++i) { t.setNormal(i, normals[i]); }
			for(ofIndexType i = 0; i < texcoords.size(); ++i) { t.setTexCoord(i, texcoords[i]); }
			return pos-offset;
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			static const ofIndexType NUM = 3;
			msg.append(3);
			for(ofIndexType i = 0; i < NUM; ++i) { msg.append(t.getVertex(i)); }
			if(t.hasColors()) {
				msg.append(3);
				for(ofIndexType i = 0; i < NUM; ++i) { msg.append(t.getColor(i)); }
			}
			else {
				msg.append(0);
			}
			if(t.hasNormals()) {
				msg.append(3);
				for(ofIndexType i = 0; i < NUM; ++i) { msg.append(t.getNormal(i)); }
			}
			else {
				msg.append(0);
			}
			if(t.hasTexcoords()) {
				msg.append(3);
				for(ofIndexType i = 0; i < NUM; ++i) { msg.append(t.getTexCoord(i)); }
			}
			else {
				msg.append(0);
			}
		}
	};
	template<typename PixelType>
	struct converter<ofPixels_<PixelType>> {
		using T = ofPixels_<PixelType>;
		static inline size_type from_msg(T &t, const Message &msg, size_type offset) {
			auto pos = offset;
			ofPixelFormat format;
			std::size_t width, height;
			pos += msg.to(pos, format, width, height);
			t.allocate(width, height, format);
			auto *data = t.getData();
			pos += detail::array::from_msg(data, msg, pos, t.size());
			return pos-offset;
		}
		static inline void append_to_msg(Message &msg, const T &t) {
			msg.append(t.getPixelFormat(), t.getWidth(), t.getHeight());
			detail::array::append_to_msg(msg, t.getData(), t.size());
		}
	};
}
}

#include "ofxNNGMessageADLConverter.h"
