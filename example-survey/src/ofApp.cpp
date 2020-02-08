#include "ofApp.h"

using namespace ofxNNG;
using namespace std;

//--------------------------------------------------------------
void ofApp::setup(){
	Surveyor::Settings surveys;
	surveys.allow_callback_from_other_thread = false;
	survey_.setup(surveys);
	survey_.createListener("inproc://test")->start();

	const vector<string> names{
		"apple",
		"banana",
		"cylinder",
		"device",
		"bang",
		"circle"
	};
	Respondent::Settings responds;
	for(auto &&n : names) {
		auto r = std::make_shared<ofxNNG::Respondent>();
		r->setup<char, string>(responds, [n](const char &request, string& response) {
			response = n + " is here!";
			return n[0] == request;
		});
		r->createDialer("inproc://test")->start();
		respond_.emplace_back(r);
	}
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofDrawBitmapString("press a,b,c,d and see console to know what happens", 10, 14);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	char ch = key;
	ofLogNotice("survey") << "is there anyone who's name starts with:" << ch;
	survey_.send<string>(ch, [](const string &response) {
		ofLogNotice("renponse") << response;
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
