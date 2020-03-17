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

And this theory is also alive when you receive.  
Yes you can do this;

```
struct UserType {
	std::string name;
	glm::vec3 pos;
	// if you know this class is memcpy-able, please notify ofxNNG.
	OFX_NNG_NOTIFY_TO_USE_MEMCPY_MEMBER
};
// or if the class is third-party that you cannot add the macro in the scope, still you can notify
OFX_NNG_NOTIFY_TO_USE_MEMCPY(UserType);

socket.setCallback<ofxNNG::Message>([](const ofxNNG::Message &msg) {
	// now you can convert the message into UserType
	UserType value = msg.get<UserType>();
});
```

But ofxNNG generously does the conversion(`ofxNNG::Message -> UserType`) for you internally.  
So all you need to do is;

```
socket.setCallback<UserType>([](const UserType &msg) {});
// receiving by multiple arguments is also valid
socket.setCallback<std::string, glm::vec3>([](const std::string &name, const glm::vec3 &pos) {});
```

Also you can receive by reference;

```
// all valid
UserType type;
socket.setCallback(type);
socket.setCallback(type.name, type.pos);
int intval;
std::string strval;
socket.setCallback(type, intval, strval);
```

If you want to use what is not memcpy-able or you don't want to do so, you have another 2 options to take to implement your own.  

__caution:__  
When you tall to host(s) that uses foreign endian, default conversion(memcpy) may cause an issue.  
So it'd be better to implement them to avoid this.  
Macro style will be the easiest solution.

(Option 1). as static functions in `ofxNNG::adl_converter<UserType>`;

```
namespace ofxNNG {
	template<>
	struct adl_converter<UserType> {
		static inline std::size_t from_msg(Type &type, const ofxNNG::Message &msg, std::size_t offset) {
			return msg.to(offset, type.name, type.pos);
		}

		static inline void append_to_msg(ofxNNG::Message &msg, Type type) { 
			ofxNNG::Message::appendTo(msg, type.name, type.pos);
		}
	};
}
```

It can be also done with macro

```
namespace ofxNNG {
	OFX_NNG_ADL_CONVERTER(UserType, name, pos);
}
```

(Option 2). as member functions;

```
struct UserType {
	...
	std::size_t from_nng_msg(const ofxNNG::Message &msg, std::size_t offset) {
		return msg.to(offset, name, pos);
	}
	void append_to_nng_msg(ofxNNG::Message &msg) const {
		ofxNNG::Message::appendTo(msg, name, pos);
	}
};
```

Of course it can be replaced with macro

```
struct UserType {
	...
	OFX_NNG_MEMBER_CONVERTER(name, pos);
};
```

If both of them are defined, option 1 is the primary.

## ofxNNG::Serialize / ofxNNG::Deserialize

ofxNNG provides APIs for doing Serialize/Deserialize.  
They are like "Adapter" for ofxNNG::Message.  
This means that anything that can be converted from/to ofxNNG::Message can be serialized.  

```
UserType type;
...
// getting binary expression
ofBuffer binary = ofxNNG::Serialize(type);
// unzip
ofxNNG::Deserialize(binary).to(type);

// write to file
ofxNNG::Serialize(type).write("myfile.bin");
// read from file
ofxNNG::Deserialize("myfile.bin").to(type);
```

# Special Thanks

- [2bbb](https://github.com/2bbb/)(for reference of implementing Message conversion)