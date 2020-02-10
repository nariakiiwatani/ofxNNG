# ofxNNG

openFrameworks addon for [nng(nanomsg next generation)](https://nanomsg.github.io/nng/)  

## CAUTION
This addon is under development.  
It's ready to use but destructive changes may occur.  

## how to use
See example.  
You can use most of the classes only by `setup`, `createDialer/createListener`, `start`, and `send`.  
In some cases you have to call them with `callback`.  

### protocols
- bus
	- ofxNNGBus
- pair
	- ofxNNGPair
- pipeline(push/pull)
	- ofxNNGPush
	- ofxNNGPull
- reqrep
	- ofxNNGReq
	- ofxNNGRep
- pubsub
	- ofxNNGPub
	- ofxNNGSub
- survey
	- ofxNNGSurveyor
	- ofxNNGRespondent

## ofxNNG::Message

This class is a kind of "glue" class.  
You can use this class explicitly like;

```
ofxNNG::Message msg;
msg.append(42, 3.14, glm::vec3(0,0,1));
socket.send(msg);
```

But you can get the same result by;

```
socket.send({42, 3.14, glm::vec3(0,0,1)});
```

And this theory is alive when you receive.  
Yes you can do this;

```
struct UserType {
	int answer;
	float pi;
	glm::vec3 normal;
};
// setup callback function
socket.setup<ofxNNG::Message>([](const ofxNNG::Message &msg) {
	UserType value = msg.get<UserType>();
});
```

But ofxNNG generously do the conversion(`ofxNNG::Message -> UserType`) for you internally;

```
socket.setup<UserType>([](const UserType &msg) {
});
```

If you want to use what is not memcpy-able or you don't want to do so, you can define your own convert functions in `ofxNNG` namespace;

```
namespace ofxNNG {
	template<>
	struct adl_converter<UserType> {
		static inline void from_msg(UserType &t, const Message &msg, std::size_t offset) {
			return msg.to(offset, t.name, t.pos);
		}
		static inline Message to_msg(UserType t) {
			return Message{t.name, t.pos};
		}
	};
}
```

Or you can do the same with macro (`name` and `pos` here are members of `UserType`)

```
namespace ofxNNG {
	OFX_NNG_ADL_CONVERTER(UserType, name, pos);
}
```

# Special Thanks

- [2bbb](https://github.com/2bbb/)(for reference of implementing Message conversion)