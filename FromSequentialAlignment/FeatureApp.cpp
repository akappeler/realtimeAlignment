/***********************************************************************************
 Feature Calculation Console Application
 
 Created:
 TCL Research America
 Armin Kappeler, 09/18/2013

 Modification:
 
 ************************************************************************************/

#include "stdafx.h"
#include "alignLib.h"
#include <fstream>
#include "ConfigFile.h"

/***********************************************************************************/
//Reads configuration file (config.ini)
void loadAlignConfiguration(ConfigFile cfg, SequentialAlignment &sa) 
{

	int value = cfg.getValueOfKey<int>("showFrames",0);
	sa.setShowFrames(value);

	VideoSettings vidCfg1;
	vidCfg1.startFr = cfg.getValueOfKey<int>("Vid1_StartFrame",vidCfg1.startFr);
	vidCfg1.maxNrFr = cfg.getValueOfKey<int>("Vid1_NrFrames",vidCfg1.maxNrFr);
	vidCfg1.uBorder = cfg.getValueOfKey<int>("Vid1_BorderTop",vidCfg1.uBorder);
	vidCfg1.dBorder = cfg.getValueOfKey<int>("Vid1_BorderBottom",vidCfg1.dBorder);
	vidCfg1.lBorder = cfg.getValueOfKey<int>("Vid1_BorderLeft",vidCfg1.lBorder);
	vidCfg1.rBorder = cfg.getValueOfKey<int>("Vid1_BorderRight",vidCfg1.rBorder);
	sa.setVideoConfig(1,vidCfg1);

	VideoSettings vidCfg2;
	vidCfg2.startFr = cfg.getValueOfKey<int>("Vid2_StartFrame",vidCfg2.startFr);
	vidCfg2.maxNrFr = cfg.getValueOfKey<int>("Vid2_NrFrames",vidCfg2.maxNrFr);
	vidCfg2.uBorder = cfg.getValueOfKey<int>("Vid2_BorderTop",vidCfg2.uBorder);
	vidCfg2.dBorder = cfg.getValueOfKey<int>("Vid2_BorderBottom",vidCfg2.dBorder);
	vidCfg2.lBorder = cfg.getValueOfKey<int>("Vid2_BorderLeft",vidCfg2.lBorder);
	vidCfg2.rBorder = cfg.getValueOfKey<int>("Vid2_BorderRight",vidCfg2.rBorder);
	sa.setVideoConfig(2,vidCfg2);

	FeatureSettings featCfg;
	featCfg.featureType = cfg.getValueOfKey<int>("Feat_Type",featCfg.featureType);
	if (featCfg.featureType==2) //HS histogram
	{
		featCfg.Bins1 = cfg.getValueOfKey<int>("Feat_HBins",featCfg.Bins1);
		featCfg.Bins2 = cfg.getValueOfKey<int>("Feat_SBins",featCfg.Bins2);
	} else //Ordinal Feature
	{
		featCfg.Bins1 = cfg.getValueOfKey<int>("Feat_HBins",featCfg.Bins1);
		featCfg.Bins2 = cfg.getValueOfKey<int>("Feat_VBins",featCfg.Bins2);
	}
	sa.setFeatureConfig(featCfg);
}

/***********************************************************************************/
int main(int argc, char* argv[])
{
	char* fileVid;
	char* resultfile;
	char* configFileName = "config.ini";
	string cfgfile;
	if (argc >=4) {
		cfgfile = argv[3];
	}else {
		cfgfile = "config.ini";
	}

	//check inputs
    if (argc < 3) { 
        cout << "VIDEO FEATURE CALCULATION\n"; 
        cout << "--------------------------------------------------\n"; 
        cout << "Syntax: calcFeatures.exe <video> <result> [config]\n"; 
        cout << "\n"; 
        cout << "The <result> file ending should be either \".csv\" or \"yaml\"\n"; 
		cout << "yaml-files can be used as input for \"align.exe\" instead of the video files\n";
        cout << "\n"; 
        cout << "Example 1: calcFeatures.exe video1.avi result.csv\n"; 
        cout << "Example 2: calcFeatures.exe video1.avi result.yaml\n"; 
        cout << "Example 3: calcFeatures.exe video1.avi result.csv myconfig.ini\n"; 
        cout << "Example 4: calcFeatures.exe video1.avi result.yaml myconfig.ini\n"; 
        cout << "\n"; 
        cout << "--------------------------------------------------\n"; 
        cout << "TCL Research America\n"; 
        cout << "Armin Kappeler\n"; 
        cout << "09//04//2013\n"; 
        cin.get();
        exit(0);
    } else { // if we got enough parameters...
		fileVid = argv[1];
		resultfile = argv[2];
		
		ofstream ofile(resultfile);
		if (!ofile) {
			cout << "Invalid input argument 2: invalid output filename\n";
			exit(0);
		}
		ofile.close();		
		ifstream ifile1(fileVid);
		if (!ifile1) {
			cout << "Invalid input argument 1: First video doesn't exist\n";
			exit(0);
		}
	}

	//load configuration
	cout << "READ CONFIGURATION FILE\n";
	ConfigFile cfg(cfgfile);
	SequentialAlignment sa;
	loadAlignConfiguration(cfg,sa);

	//call Video Alignment
	cout << "FEATURE CALCULATION\n";
	sa.calculateVideoFeatures(fileVid, resultfile);

	return 0;
}
