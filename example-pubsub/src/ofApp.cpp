#include "ofApp.h"

using namespace ofx::nng;

//--------------------------------------------------------------
void ofApp::setup(){
	Pub::Settings pubs;
	pubs.url = "inproc://test";
	pub_.setup(pubs);

	Sub::Settings subs;
	subs.url = "inproc://test";
	subs.onReceive = [this](nng_msg *msg) {
		onReply(*msg);
		return true;
	};
	sub_.resize(8);
	for(auto &&s : sub_) {
		s.setup(subs);
	}
}

void ofApp::onReply(nng_msg &msg)
{
	auto body = nng_msg_body(&msg);
	auto len = nng_msg_len(&msg);
	ofLogNotice() << std::string((char*)body, len);
}

//--------------------------------------------------------------
void ofApp::update(){
	for(auto &&s : sub_) {
		s.receive(NNG_FLAG_NONBLOCK);
	}
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	std::string msg = "pressed:" + ofToString((char)key);
	pub_.send(const_cast<char*>(msg.data()), msg.length(), NNG_FLAG_NONBLOCK);
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
