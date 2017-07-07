/*********************************************************
 dataCrystalsApp.h
 class for Data Crystals
 
 Created by Scott Kildall
 July 2, 2017
 
 **********************************************************/

#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "datum.h"

#define DEFAULT_SCREEN_WIDTH (1280)
#define DEFAULT_SCREEN_HEIGHT (800)


class dataCrystalsApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

        // camera
        ofEasyCam cam;
    
        // data points
        datum *data;
        unsigned long numData = 0;
        unsigned short nextClusterID;
        int maxUnattachedSize;
        bool bDrawClusterIDs;
        bool bUseColor;
        bool bAllLoaded;
        bool bUseSizeColumn;
    
        ofVec3f gravCenter;
    private:
        //-- VARIABLES
        unsigned long numClusterCycles;
    
        void loadCSVFiles();
        void loadAllData();
        unsigned long loadCSVData(string filename, datum *dataPtr, int fileIndex);
        void saveMesh();
    
        void makeClusters();
        void bindClusters( datum *d1, datum *d2);
        void attachToCluster(datum *subCluster, datum *mainCluster);
    
        void findGravCenter();
        bool inClusterDistance( datum *d1, datum *d2 );
        void countParentsAndChildren();
        bool inSameCluster( datum *d1, datum *d2 );
    
        // CSV files
        vector <ofFile> csvFiles;
        int numCSVFiles;
        int currentFileIndex;
    
        Boolean bClustering;
        unsigned long numChildren;
        unsigned long numParents;
        unsigned long numUnattached;
    
        // DRAWING
        float map(float m, float in_min, float in_max, float out_min, float out_max);
    
        ofTrueTypeFont  drawFont;
        string loadedFilename;
        string numDataPointsStr;
    
        // GUI
        void initGui();
        
        bool bHideGui;
        bool bShowClusterStatus;
        ofxPanel gui;
    
        ofxFloatSlider gravSlider;
        float gravRatio;
        void gravSliderChanged(float & val);
    
        ofxFloatSlider jiggleSlider;
        float jigglePct;
        void jiggleSliderChanged(float & val);
    
        ofxFloatSlider xScaleSlider;
        float xScale;
        void xScaleChanged(float & val);
    
    
        ofxFloatSlider yScaleSlider;
        float yScale;
        void yScaleChanged(float & val);
    
    
        ofxFloatSlider zScaleSlider;
        float zScale;
        void zScaleChanged(float & val);
    
        ofxFloatSlider clusterPctSlider;
        float clusterPct;
        void clusterPctChanged(float & val);
    
    
        ofxButton applyButton;
        void applyButtonHit();
    
        // UTILITY
        void getColorFromFileIndex(int currentFileIndex, unsigned short &r, unsigned short &b, unsigned short &g);
        void applyColor();
        void applyColorToAll();
        std::string makePointsStr(unsigned long value);
    
        void drawClusterStatus();
        void makeClusterDisplayStrings();
        void formGUIStrings();
    
        char numClusterCyclesStr[64];
        char numUnattachedStr[64];
        char numParentsString[64];
        char numChildrenString[64];
        char numDataString[64];
        char clusterString[64];
        char maxUnattachedSizeString[64];
        char sizeOnString[64];
        char fileDisplayStr[64];
};
