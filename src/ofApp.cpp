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
    fonts.addStyle("bodyBold", ofxFontStash2::Style("roboBold", 18, ofColor::white));
    fonts.addStyle("bodyItalic", ofxFontStash2::Style("roboItalic", 18, ofColor::white));
    fonts.addStyle("bodyRed", ofxFontStash2::Style("robo", 18, ofColor::red));
    fonts.addStyle("bodyGreen", ofxFontStash2::Style("robo", 18, ofColor::green));
    fonts.addStyle("bodyBlue", ofxFontStash2::Style("robo", 18, ofColor::blue));
    fonts.addStyle("bodyDarkgreen", ofxFontStash2::Style("robo", 18, ofColor::darkGreen));

    ofDisableAntiAliasing(); //to get precise lines
    fonts.pixelDensity = 2.0;
}


std::string random_string( size_t length )
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}

void ofApp::draw(){
    
    int numlines = 1000;
    
    float margin = 50;
    float x = margin;
    float y = 100;
    float colW = MIN(MAX(120, ofGetMouseX() - x), ofGetWidth()/2 - x);

    drawInsertionPoint(x, y, 100);

    string dynamicSizeTag = "<style font='robo' size='" + ofToString(18 + 1 * sinf(30 * ofGetElapsedTimef()), 2) + "' color='#888888'>";
    string blinkTag = "<style font='robo' size='18' color='#FFFFFF" + string(ofGetFrameNum()%20 < 10 ? "11":"EE") + "'>";
    
    string test = "<body>";
    //std::vector<std::string> test2;
    for(int i = 0; i<numlines; i++){
        test += "!!!!!!!!!!!!"+random_string(32)+"=============<br>";
    }
    test = test + "</body>";
    
    
    string test2 = "<body>";
    //std::vector<std::string> test2;
    for(int i = 0; i<numlines; i++){
        test2 += "::::::::::::::::::"+random_string(32)+"|||||||||||||||||<br>";
    }
    test2 = test2 + "</body>";
    
    
    
    string styledText =
    "<header>ofxFontStash2</header>"
    "<body>"
    "<br/><br/>"
    "<bodyBold>ofxFontStash2</bodyBold> allows you to draw text. It can draw individual "
    
    "lines of text, or longer texts constrained to a column. "
    "It offers text alignment (left, center, right).<br/><br/>"
    "It also allows you to mix and match different text <bodyItalic>styles</bodyItalic> in the same "
    "paragraph. You can do so by <bodyRed>creating styles that can be applied at an "
    "individual <bodyDarkgreen>cha<bodyGreen>rac</bodyGreen>ter level</bodyDarkgreen> with style nesting</bodyRed>. You can also inline styles."
    "<br/><br/>"
    
    "Here, we demo the inline styles feature by dynamically "
    "setting a font " + dynamicSizeTag + "size.</style><br/>"
    "Here we show how to easily make text " + blinkTag + "blink.</style>"
    "<br/><br/>"
    "Here we test what happens when to styles are <bodyBlue>side</bodyBlue> <bodyRed> by side. </bodyRed>"
    "Here we test inline style <bodyItalic color='#66'>overrides.</bodyItalic>"
    "<br/><br/>"
    "Here we test a 1/2 lineHeight line break...<br/><br heightMult='0.5'/>"
    "And we are done testing. Let's go home."
    "</body>";
    
    

    ofRectangle bbox;
    TSGL_START("draw formatted GL");
    TS_START("draw formatted");
    bbox = fonts.drawFormattedColumn(styledText, x, y, colW, OF_ALIGN_HORZ_LEFT, debug);
    TS_STOP("draw formatted");
    TSGL_STOP("draw formatted GL");

    //draw left & right column limits
    ofSetColor(255,32);
    ofDrawLine(x , 0, x, ofGetHeight());
    ofDrawLine(x + colW, 0, x + colW, ofGetHeight());

    //draw bbox on top
    ofSetColor(255,8);
    ofDrawRectangle(bbox);

    //now draw with straight nanovg renderer
    x = ofGetWidth() * 0.5 + margin/2;
    drawInsertionPoint(x, y, 100);
    TSGL_START("draw NVG GL");
    TS_START("draw NVG");
    fonts.drawColumnNVG(test, fonts.getStyle("body"), x, y, colW, OF_ALIGN_HORZ_LEFT);
    TS_STOP("draw NVG");
    TSGL_STOP("draw NVG GL");
    ofSetColor(255,32);
    ofDrawLine(x , 0, x, ofGetHeight());
    ofDrawLine(x + colW, 0, x + colW, ofGetHeight());
}


void ofApp::keyPressed(int key){

    if(key == 'd'){
        debug ^= true;
    }
}



void ofApp::drawInsertionPoint(float x, float y, float w){
    ofSetColor((ofGetFrameNum() * 20)%255,200);
    ofDrawCircle(x,y, 1.5);
    ofSetColor(255,64);
    ofDrawLine(x - 10, y, x + w, y);
    ofSetColor(255);
}





