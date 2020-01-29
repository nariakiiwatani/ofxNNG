#include "ofApp.h"

using namespace ofx::nng;

//--------------------------------------------------------------
void ofApp::setup(){
	Surveyor::Settings surveys;
	surveys.allow_callback_from_other_thread = false;
	survey_.setup(surveys);
	survey_.createListener("inproc://test")->start();

	Respondent::Settings responds;
	respond_.resize(8);
	for(auto &r : respond_) {
		r = std::make_shared<ofx::nng::Respondent>();
		r->setup<ofBuffer, ofBuffer>(responds, [](const ofBuffer &buffer, ofBuffer& dst) {
			dst.set(buffer);
			ofLogNotice("got survey: ") << buffer.getText();
			return true;
		});
		r->createDialer("inproc://test")->start();
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
	survey_.send<ofBuffer, ofBuffer>(buffer, [](const ofBuffer &buffer) {
		ofLogNotice("got respond: ") << buffer.getText();
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
