/***********************************************************************************
 Sequential Alignment Console Application
 
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
void loadConfiguration(ConfigFile cfg, SequentialAlignment &sa) 
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

	AlignSettings alignCfg;
	alignCfg.blockSize			= cfg.getValueOfKey<int>("A_BlockSize",alignCfg.blockSize);
	alignCfg.costpMove			= cfg.getValueOfKey<float>("A_CostPerMove",alignCfg.costpMove);
	alignCfg.costpNondiagMove   = cfg.getValueOfKey<float>("A_CostPerNondiagMove",alignCfg.costpNondiagMove);
	alignCfg.diagonalAvgLength  = cfg.getValueOfKey<int>("A_DiagonalAvgLength",alignCfg.diagonalAvgLength);
	
	sa.setAlignConfig(alignCfg);

	value = cfg.getValueOfKey<int>("KF_UseKeyframes",1);
	sa.setUseKeyframes(value);
	KeyframeSettings kfCfg;
	kfCfg.distanceNorm		= cfg.getValueOfKey<int>("KF_DistanceNorm",kfCfg.distanceNorm);
	kfCfg.skipFactor		= cfg.getValueOfKey<int>("KF_SkipFactor",kfCfg.skipFactor);
	kfCfg.tolerance			= cfg.getValueOfKey<float>("KF_Tolerance",kfCfg.tolerance);
	kfCfg.saveMask		= cfg.getValueOfKey<int>("KF_SaveMask",kfCfg.saveMask);
	sa.setKeyframeConfig(kfCfg);

}

/***********************************************************************************/
int main(int argc, char* argv[])
{
	string cfgfile;
	char* file1;
	char* file2;
	char* resultfile;

	//check inputs
    if (argc < 4) { 
        cout << "VIDEO ALIGNMENT\n"; 
        cout << "--------------------------------------------------\n"; 
        cout << "Syntax: align <video1> <video2> <result> [config]\n"; 
        cout << "\n"; 
        cout << "Example 1: align.exe video1.avi video2.avi result.txt\n"; 
        cout << "\n"; 
        cout << "Example 2: align.exe video1.avi video2.avi result.txt myconfig.ini\n"; 
        cout << "\n";         
		cout << "--------------------------------------------------\n"; 
        cout << "TCL Research America\n"; 
        cout << "Armin Kappeler\n"; 
        cout << "08//29//2013\n"; 
        cin.get();
        exit(0);
    } else { // if we got enough parameters...
		file1 = argv[1];
		file2 = argv[2];
		resultfile = argv[3];

		if (argc >=5) {
			cfgfile = argv[4];
		}else {
			cfgfile = "config.ini";
		}

		ofstream ofile(resultfile);
		if (!ofile) {
			cout << "Invalid input argument 3: invalid output filename\n";
			exit(0);
		}
		ofile.close();		
		ifstream ifile1(file1);
		if (!ifile1) {
			cout << "Invalid input argument 1: First video doesn't exist\n";
			exit(0);
		}
		ifile1.close();
		ifstream ifile2(file2);
		if (!ifile2) {
			cout << "Invalid input argument 2: Second video doesn't exist\n";
			exit(0);
		}
		ifile2.close();
		ifstream ifile3(cfgfile);
		if (!ifile3) {
			cout << "No configuration file found (Default filename: \"config.ini\")\n";
			exit(0);
		}
		ifile3.close();
	}
	//load configuration
	cout << "READ CONFIGURATION FILE\n";
	ConfigFile cfg(cfgfile);
	SequentialAlignment sa;
	loadConfiguration(cfg,sa);

	//call Video Alignment
	cout << "VIDEO SEQUENCE ALIGNMENT\n";
	sa.alignVideos(file1, file2, resultfile);

	return 0;
}