#include "ofApp.h"

using namespace ofx::nng;
//--------------------------------------------------------------
void ofApp::setup(){
	Rep::Settings reps;
	reps.url = "inproc://test";
	rep_.setup(reps);

	Req::Settings reqs;
	reqs.url = "inproc://test";
	req_.setup(reqs);
}

//--------------------------------------------------------------
void ofApp::update(){
	while(rep_.hasWaitingRequest()) {
		ofBuffer buffer;
		auto ctx = rep_.getNextRequest(buffer);
		buffer.append(":reply");
		rep_.reply(ctx, buffer);
	}
	while(req_.hasWaitingResponse()) {
		ofBuffer buffer;
		if(req_.getNextResponse(buffer)) {
			ofLogNotice() << buffer.getText();
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	auto buffer = ofBuffer();
	buffer.set("pressed:" + ofToString((char)key));
	req_.send(buffer);
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
