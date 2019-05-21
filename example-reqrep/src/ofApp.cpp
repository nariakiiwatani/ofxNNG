#include "ofApp.h"

using namespace ofx::nng;
//--------------------------------------------------------------
void ofApp::setup(){
	Rep::Settings reps;
	rep_.setup(reps, std::function<bool(const ofBuffer&, ofBuffer&)>([](const ofBuffer &buffer, ofBuffer &dst) {
		dst = buffer;
		return true;
	}));
	rep_.listen("inproc://test");

	Req::Settings reqs;
	req_.setup(reqs);
	req_.dial("inproc://test");
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
	req_.send(buffer, std::function<void(const ofBuffer&)>([](const ofBuffer &buffer) {
		ofLogNotice() << buffer.getText();
	}));
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
