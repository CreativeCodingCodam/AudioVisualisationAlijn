#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(165);
	ofEnableAlphaBlending();

	wavePhase = 0;
	pulsePhase = 0;

	currentColour = ofColor(255, 0, 0, 255);
	
	// start the sound stream with a sample rate of 44100 Hz, and a buffer
	// size of 512 samples per audioOut() call
	ofSoundStreamSettings settings;
	settings.numOutputChannels = 2;
	settings.sampleRate = 44100;
	settings.bufferSize = 512;
	settings.numBuffers = 4;
	settings.setOutListener(this);
	soundStream.setup(settings);
}

//--------------------------------------------------------------
void ofApp::update(){
	
	// "lastBuffer" is shared between update() and audioOut(), which are called
	// on two different threads. This lock makes sure we don't use lastBuffer
	// from both threads simultaneously (see the corresponding lock in audioOut())
	std::unique_lock<std::mutex> lock(audioMutex);

	// this loop is building up a polyline representing the audio contained in
	// the left channel of the buffer
	
	// the x coordinates are evenly spaced on a grid from 0 to the window's width
	// the y coordinates map each audio sample's range (-1 to 1) to the height
	// of the window
	
	waveform.clear();
	for(size_t i = 0; i < lastBuffer.getNumFrames(); i++) {
		float sample = lastBuffer.getSample(i, 0);
		float x = ofMap(i, 0, lastBuffer.getNumFrames(), 0, ofGetWidth());
		float y = ofMap(sample, -1, 1, 0, ofGetHeight());
		waveform.addVertex(x, y);
	}
	
	rms = lastBuffer.getRMSAmplitude();
}

//--------------------------------------------------------------

ofColor	nextColour(const ofColor& prev) {
	constexpr uint8_t	COLOUR_SHIFT = 3;
	ofColor				result = prev;
	unsigned char*		channels[3] = {&result.r, &result.g, &result.b};
	
	for (int i = 0; i < 3; i++) {
		uint8_t& curr = *(channels[i]);
		uint8_t& next = *(channels[(i + 1) % 3]);

		if ((curr && next) || curr == 255) {
			curr -= COLOUR_SHIFT;
			next += COLOUR_SHIFT;
			break;
		}
	}
	return result;
}

void ofApp::draw() {
	ofBackground(ofColor::black);
	currentColour = nextColour(currentColour);
	circles.push_front(Circle{ 50 + 800 * rms, currentColour });
	if (circles.size() > MAX_CIRCLES) {
		circles.pop_back();
	}
	for (int i = circles.size() - 1; i >= 0; i--) {
		Circle&			circle = circles[i];
		const float		range = 1.0f - (float)i / MAX_CIRCLES;
		float			circleWidth = 1 + 100 * range;
		ofPath			circlePath;

		circle.colour.a = 255.0f * range;
		circlePath.setStrokeColor(circle.colour);
		circlePath.setCircleResolution(100);
		circlePath.setStrokeWidth(3);
		circlePath.setFilled(false);
		circlePath.arc(window.x / 2, window.y / 2, circle.size, circle.size, 0, 360);
    	circlePath.close();
		circlePath.draw();
	}
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer &outBuffer) {
	
	// base frequency of the lowest sine wave in cycles per second (hertz)
	float frequency = 172.5;
	
	// mapping frequencies from Hz into full oscillations of sin() (two pi)
	float wavePhaseStep = (frequency / outBuffer.getSampleRate()) * glm::two_pi<float>();
	float pulsePhaseStep = (0.5 / outBuffer.getSampleRate()) * glm::two_pi<float>();
	
	// this loop builds a buffer of audio containing 3 sine waves at different
	// frequencies, and pulses the volume of each sine wave individually. In
	// other words, 3 oscillators and 3 LFOs.
	
	for(size_t i = 0; i < outBuffer.getNumFrames(); i++) {
		
		// build up a chord out of sine waves at 3 different frequencies
		float sampleLow = std::sin(wavePhase);
		float sampleMid = std::sin(wavePhase * 1.5);
		float sampleHi = std::sin(wavePhase * 2.0);
		
		// pulse each sample's volume
		sampleLow *= std::sin(pulsePhase);
		sampleMid *= std::sin(pulsePhase * 1.04);
		sampleHi *= std::sin(pulsePhase * 1.09);
		
		float fullSample = (sampleLow + sampleMid + sampleHi);
		
		// reduce the full sample's volume so it doesn't exceed 1
		fullSample *= 0.3;
		
		// write the computed sample to the left and right channels
		outBuffer.getSample(i, 0) = fullSample;
		outBuffer.getSample(i, 1) = fullSample;
		
		// get the two phase variables ready for the next sample
		wavePhase += wavePhaseStep;
		pulsePhase += pulsePhaseStep;
	}
	
	std::unique_lock<std::mutex> lock(audioMutex);
	lastBuffer = outBuffer;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
