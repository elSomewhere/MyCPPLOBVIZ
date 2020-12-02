#include "ofApp.h"





void ofApp::setup(){

    ofBackground(22);
    #ifdef TIME_SAMPLE_OFX_FONTSTASH2
    TIME_SAMPLE_ENABLE();
    TIME_SAMPLE_SET_AVERAGE_RATE(0.1);
    TIME_SAMPLE_SET_DRAW_LOCATION(TIME_SAMPLE_DRAW_LOC_BOTTOM_RIGHT);
    TIME_SAMPLE_SET_PRECISION(4);
    #endif

    fonts.setup(false);

    //add fonts to the stash
    fonts.addFont("robo", "/Users/estebanlanter/Downloads/Helvetica 400.ttf");
    fonts.addFont("roboBold", "/Users/estebanlanter/Downloads/Helvetica 400.ttf");
    fonts.addFont("roboItalic", "/Users/estebanlanter/Downloads/Helvetica 400.ttf");
    fonts.addFont("roboBlack", "/Users/estebanlanter/Downloads/Helvetica 400.ttf");

    //define font styles
    fonts.addStyle("header", ofxFontStash2::Style("roboBlack", 44, ofColor::white));
    fonts.addStyle("body", ofxFontStash2::Style("robo", 8, ofColor(244)));
    fonts.addStyle("bodyBold", ofxFontStash2::Style("roboBold", 10, ofColor::white));

    ofDisableAntiAliasing(); //to get precise lines
    fonts.pixelDensity = 2.0;
    
    
    
    
    ////////////////////////////
    // setup the server to listen on 11999
    ofxTCPSettings settings(1200);

    // set other options
    //settings.blocking = false;
    //settings.reuse = true;
    //settings.messageDelimiter = "\n";
    TCP.setup(settings);
    // optionally set the delimiter to something else.  The delimiter in the client and the server have to be the same, default being [/TCP]
    TCP.setMessageDelimiter("\n");
    lastSent = 0;
    answerclient_frequency_ms = 100;
    
}




int * getRandom() {
   static int  r[40];
   // set the seed
   srand( (unsigned)time( NULL ) );
   for (int i = 0; i < 10; ++i) {
      r[i] = rand();
      cout << r[i] << endl;
   }
   return r;
}




std::string random_string( size_t length )
{
    auto randchar = []() -> char
    {
        const char charset[] = "0123456789";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}




//--------------------------------------------------------------
void ofApp::update(){

    // for each client lets send them a message letting them know what port they are connected on
    // we throttle the message sending frequency to once every 100ms
    uint64_t now = ofGetElapsedTimeMillis();
    if(now - lastSent >= answerclient_frequency_ms){
        for(int i = 0; i < TCP.getLastID(); i++){
            if( !TCP.isClientConnected(i) ) continue;
            TCP.send(i, "hello client - you are connected on port - "+ofToString(TCP.getClientPort(i)) );
        }
        lastSent = now;
    }

}


void ofApp::draw(){
    
    // for each connected client lets get the data being sent and lets push it to a vector
    for(unsigned int i = 0; i < (unsigned int)TCP.getLastID(); i++){
        // check if connection alive
        if(!TCP.isClientConnected(i)){
            continue;
        }
        
        
        // get the ip and port of the client
        string port = ofToString( TCP.getClientPort(i) );
        string ip   = TCP.getClientIP(i);
        string info = "client "+ofToString(i)+" -connected from "+ip+" on port: "+port;
        store_client_info.push_back(info);

        
        // receive all the available messages, separated by \n
        // and keep only the last one
        string tmp;
        do{
            tmp = TCP.receive(i);
            size_t pos = 0;
            std::string token;
            std::string delimiter = ";";
            while ((pos = tmp.find(delimiter)) != std::string::npos) {
                token = tmp.substr(0, pos);
                tmp.erase(0, pos + delimiter.length());
                //std::cout<<tmp<<std::endl;
                if(received_lines.size()<100){
                    received_lines.push_back(token+" ");
                }else{
                    received_lines.erase(received_lines.begin());
                    received_lines.push_back(token+" ");
                }
            }
        }while(tmp!="");
    }
    
    int numlines = 500;
    float margin = 50;
    float x = margin;
    float y = 100;
    float colW = 300;
    
    string test;
//    for (const auto &piece : received_lines) test += piece;
//    std::cout<<test<<std::endl;
    // simulate random text
    while(orders.size()< numlines){
        orders.push_back("!!!!!!!!!!!!"+random_string(16)+"=============<br/>");
    }
    if(orders.size() == numlines){
        orders.erase(orders.begin());
        orders.push_back("!!!!!!!!!!!!"+random_string(16)+"=============<br/>");
    }
    for (const auto &piece : orders) test += piece;
    
    
    string styledText =
    "<body>"
    "<br/><br/>"
    "<bodyBold>connect on port: " +
    ofToString(TCP.getPort())+"</bodyBold><br/>" +
    test +
    "</body>";
    
    
    ofRectangle bbox;
    TSGL_START("draw formatted GL");
    TS_START("draw formatted");
    bbox = fonts.drawFormattedColumn(styledText, 50, 0, colW, OF_ALIGN_HORZ_LEFT, debug);
    //fonts.drawColumnNVG(test, fonts.getStyle("body"), 0, 100, x, OF_ALIGN_HORZ_LEFT);
    TS_STOP("draw formatted");
    TSGL_STOP("draw formatted GL");
}


void ofApp::keyPressed(int key){

    if(key == 'd'){
        debug ^= true;
    }
}



void ofApp::drawInsertionPoint(float x, float y, float w){
    ofSetColor((ofGetFrameNum() * 20)%255,200);
    ofDrawCircle(x,y, 1.5);
    ofSetColor(255);
}





