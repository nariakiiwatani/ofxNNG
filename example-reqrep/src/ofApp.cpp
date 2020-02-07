#include "ofApp.h"

using namespace ofxNNG;
//--------------------------------------------------------------
void ofApp::setup(){
	Rep::Settings reps;
	// setup with function for replying that returns bool.
	// if true the output arg will be sent to the peer else nothing will be sent.
	// the types of input and output args can be anything that can convert from/to nng_msg.
	// see ofxNNGConvertFunctions.h and ofxNNGParseFunctions.h
	// or you can add your convert/parse functionalities in ofx::nng::util namespace.
	rep_.setup<ofBuffer, ofBuffer>(reps, [](const ofBuffer &input, ofBuffer &output) {
		output = input;
		return true;
	});
	// easiest way to start listener
	rep_.createListener("tcp://127.0.0.1:9000")->start();

	Req::Settings reqs;
	reqs.timeout_milliseconds = 1000;
	req_.setup(reqs);
	// you can retain the dialer/listener pointer to do additional settings.
	auto dialer = req_.createDialer("tcp://127.0.0.1:9000");
	dialer->setEventCallback(NNG_PIPE_EV_ADD_PRE, [this]() {
		ofLogNotice() << "this is pre-connection callback. you cannot send any message to the peer here.";
	});
	dialer->setEventCallback(NNG_PIPE_EV_ADD_POST, [this]() {
		ofLogNotice() << "this is post-connection callback. now you can send anything.";
		req_.send<std::string>("this is a connection message", [](const std::string &reply) {
			ofLogNotice() << reply;
		});
	});
	dialer->setEventCallback(NNG_PIPE_EV_REM_POST, [this]() {
		ofLogNotice() << "this is post-removal callback. you can no longer send any message.";
	});
	dialer->start();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	auto buffer = ofBuffer();
	buffer.set("pressed:" + ofToString((char)key));
	req_.send<ofBuffer>(buffer, [](const ofBuffer &buffer) {
		ofLogNotice() << buffer.getText();
	});
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
