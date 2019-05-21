# ofxNNG

openFrameworks addon for [nng(nanomsg next generation)](https://nanomsg.github.io/nng/)  

## CAUTION
This addon is under development.  
It's ready to use but destructive changes may occur.  

## how to use
You can use most of the classes only by `setup`, `dial/listen` and `send`.  
In some cases you have to call them with `callback`.  


### protocols
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
