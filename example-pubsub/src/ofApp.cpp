#include "ofApp.h"

using namespace ofx::nng;

//--------------------------------------------------------------
void ofApp::setup(){
	Pub::Settings pubs;
	pubs.url = "inproc://test";
	pub_.setup(pubs);

	Sub::Settings subs;
	subs.url = "inproc://test";
	sub_.resize(8);
	for(auto &&s : sub_) {
		s.setup(subs, std::function<void(const ofBuffer&)>([](const ofBuffer &buffer) {
			ofLogNotice("sub") << buffer.getText();
		}));
		s.subscribe(nullptr, 0);
	}
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	ofBuffer buffer;
	buffer.set(ofToString((char)key));
	pub_.send(buffer);
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
