//
//  CanvasLayerManager.cpp
//  livedraw
//
//  Created by Christopher P. Baker on 5/17/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "CanvasLayerManager.h"

//--------------------------------------------------------------
CanvasLayerManager::CanvasLayerManager() : OscNodeListener("/layer") {
}

//--------------------------------------------------------------
CanvasLayerManager::~CanvasLayerManager() {
    for (int i = 0; i < layers.size(); i++)
        delete layers[ i ];
    layers.clear();
}

//--------------------------------------------------------------
void CanvasLayerManager::setup() {
    addCommand("/new");
    addCommand("/delete");
}

//--------------------------------------------------------------
void CanvasLayerManager::setAssetManager(AssetManager* _assetManager) {
    assetManager = _assetManager;
}

//--------------------------------------------------------------
void CanvasLayerManager::setEffectsManager(EffectsManager* _effectsManager) {
    effectsManager = _effectsManager;
}

//--------------------------------------------------------------
void CanvasLayerManager::processOscMessage(string pattern, ofxOscMessage& m) {
    
    if(isMatch(pattern,"/new")) {
        // /livedraw/canvas/layer/new      LAYER_NAME [X_POSITION Y_POSITION [Z_POSITION]]
        if(validateOscSignature("s[fi][fi][fi]?", m)) {
            int x = 0;
            int y = 0;
            int z = 0;
            string layerName = m.getArgAsString(0);
            
            if(m.getNumArgs() > 1) {
                x = m.getArgAsInt32(1);
                y = m.getArgAsInt32(2);
                if(m.getNumArgs() > 3) {
                    cout << " got a z" << endl;
                    z = m.getArgAsInt32(3);
                }
            }

            
            // make a new layer
            newLayer(layerName, ofPoint(x,y,z));
            
        }
    } else if(isMatch(pattern, "/delete")) {
        // /livedraw/canvas/layer/new      LAYER_NAME
        if(validateOscSignature("s", m)) {
            string layerName = m.getArgAsString(0);
            
            // delete a layer
            deleteLayer(layerName);
        }
    }
    
}

//--------------------------------------------------------------
CanvasLayer* CanvasLayerManager::newLayer(string layerName, ofPoint point) {
    

    // rename if needed
    if(hasLayer(layerName)) {
        layerName = layerName + "_" + ofToString(ofGetElapsedTimeMillis());
    }

    CanvasLayer* cl = new CanvasLayer(layerName); // MAKE SURE THESE ARE DELETED
    cl->setup();
    
    cl->setPosition(point);
    cl->setAssetManager(assetManager);
    cl->setEffectsManager(effectsManager);

    
    CanvasLayerTransform* xform = cl->getTransform();
    ofPoint p = xform->getPosition();
    
    layers.push_back(cl);
    addChild(cl);
    
    return cl;
}


//--------------------------------------------------------------
bool CanvasLayerManager::hasLayer(string name) {
    return getLayerByName(name) != NULL;
}

//--------------------------------------------------------------
bool CanvasLayerManager::deleteLayer(string layerName) {
    bool success = false;
    cout << "deleeting layer called " << layerName << endl;
    
    vector<CanvasLayer*>::iterator it;
    
    for ( it=layers.begin() ; it < layers.end(); it++ ) {
        
        if(isMatch(layerName,(*it)->getName())) {
            break;
        }
    }
    ofArduino of;
    // delete first, then erase from the vector
    if(it != layers.end()) {
        removeChild(*it); // remove it from the OSC tree!
        delete *it;       // free the memory
        layers.erase(it); // erase it from the vector
        success = true;
    } else {
        success = false; // something went wrong
    }
    
    return success;
    
}

//--------------------------------------------------------------
CanvasLayer* CanvasLayerManager::getLayerByName(string layerName) {
    CanvasLayer* r = NULL;
    for(int i = 0; i < layers.size(); i++) {
        if(isMatch(layerName,layers[i]->getName())) {
            r = layers[i];
            break;
        }
    }
    return r;
}


void CanvasLayerManager::update() {
    for(int i = 0; i < layers.size(); i++) layers[i]->update();
}

void CanvasLayerManager::draw() {
    // we are going to draw onto the caller's drawing context (namely, canvas renderer)
    
    // TODO: perhaps best to manage groups within layers manager????
    // /livedraw/canvas/layer/group/(GROUP_NAME)/add/layer
    // everything automatically shows up in a group manager
    
    // reset it to white
//    ofSetColor(255,255,255,80);
    
    for(int i = 0; i < layers.size(); i++) {
		
		CanvasLayer* layer = layers[i];
		
		
		CanvasLayerTransform* xform = layer->getTransform();
		ofPoint a = xform->getAnchorPoint();
		ofPoint p = xform->getPosition();
		ofPoint r = xform->getRotation();
		ofPoint s = xform->getScale();
        float opacity = xform->getOpacity();

		
        int w = xform->getWidth();
        int h = xform->getHeight();
        
        //cout << layer->getName() << ": " << p.x << "/" << p.y << "/" << p.z << endl;
        
		glPushMatrix();
		
		glTranslated(p.x, p.y, p.z);

		glRotated(r.x, 1, 0, 0);
		glRotated(r.y, 0, 1, 0);
		glRotated(r.z, 0, 0, 1);
		glScaled(s.x, s.y, s.z);
        
		// do effects
		
		// get image reference and draw it
		//layer->getSource()->getImage()->draw(0,0);
		
        //        layer->draw();
        
        //layer->getFbo()->begin();

        ofSetColor(255,255,255,opacity);

 //       ofFill();
        layer->getSource()->draw(-a.x, -a.y);
        if(layer->getMask()->bAllocated()) 
        layer->getMask()->draw(-a.x, -a.y);
//        ofRect(-a.x, -a.y,0,w,h);
//        ofNoFill();
        //layer->getFbo()->end();
        
        //layer->getFbo()->draw(0,0);

		glPopMatrix();
         
        
        
	}
    
    ofSetColor(255);
    
}


/*
//--------------------------------------------------------------
bool CanvasLayerManager::deleteLayer(string layerName) {
    
}


//--------------------------------------------------------------
void CanvasLayerManager::setSolo(string layerName, bool solo) {
    
}


//--------------------------------------------------------------
void CanvasLayerManager::setLock(string layerName, bool lock) {
    
}

//--------------------------------------------------------------
bool CanvasRenderer::deleteLayer(string layerName) {
    
}
*/