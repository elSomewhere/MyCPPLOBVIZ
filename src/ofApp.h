#pragma once

#include "ofMain.h"
#include "ofxFontStash2.h"
#include "ofxNetwork.h"


//#define TIME_SAMPLE_OFX_FONTSTASH2 //comment this line to remove ofxTimeMeasurements dependency

#define TS_START_NIF
#define TS_STOP_NIF
#define TS_START_ACC
#define TS_STOP_ACC
#define TS_START
#define TS_STOP
#define TSGL_START
#define TSGL_STOP

class ofApp : public ofBaseApp{

public:
    
    std::vector<std::string> orders;
    
    std::string gen_random(const int len);
    
    void setup();
    void update();
    void draw();

    void keyPressed(int key);

    bool debug;
    ofxFontStash2::Fonts fonts;

    void drawInsertionPoint(float x, float y, float w);
    ofxTCPServer TCP;
    uint64_t lastSent;
    uint64_t answerclient_frequency_ms;
    std::vector<std::string> store_client_info;
    std::vector<std::string> received_lines;
    
    

};
