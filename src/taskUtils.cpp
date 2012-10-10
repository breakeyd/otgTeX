/*
    Copyright (C) 2012 David Breakey

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "taskUtils.h"

//local functions
static std::string getTexBin();

int checkForFirstRun(){
	//This is debug code to enforce first run behaviour
//	std::string output = systemCallResponse("rm -R data/*");
//	systemCallResponse("cp app/native/appSettings-default.txt data/appSettings.txt");
//	fprintf(stderr,"%s\n",output.c_str());
	//End debug code

	fprintf(stderr,"Checking if this is first run...");
	std::string frStatus = systemCallResponse("[ -f data/firstruncomplete" + getVersion() + " ] && echo 'no.' || echo 'yes.'");
	fprintf(stderr,"%s",frStatus.c_str());
	if(!frStatus.compare("yes.\n")){
		return 0;
	}
	return 1;
}

int firstRunTasks(){
	systemCallResponse("cp app/native/appSettings-default.txt data/appSettings.txt");

	guiInfoBox(gStr("STR_FIRST_RUN_RUNNING") + " -> " + gStr("STR_FIRST_RUN_MOVE"));
	std::string output =systemCallResponse("cp -rf app/native/texlive data/");
	fprintf(stderr,"%s\n",output.c_str());
	guiCloseDialog();

	guiInfoBox(gStr("STR_FIRST_RUN_RUNNING") + " -> " + gStr("STR_FIRST_RUN_EXTRACT"));
	output = systemCallResponse("cd data/texlive; ls *.zip | while read x;do echo ${x%.zip};rm -Rf ${x%.zip}; unzip $x; done");
	fprintf(stderr,"%s\n",output.c_str());
	guiCloseDialog();

	guiInfoBox(gStr("STR_FIRST_RUN_RUNNING") + " -> " + gStr("STR_FIRST_RUN_REMOVE"));
	output = systemCallResponse("ls data/texlive/*.zip | while read x; do rm $x; done");
	fprintf(stderr,"%s\n",output.c_str());
	guiCloseDialog();

	setTexBin("otgtexlive-2012basic");

	return 0;
}

int firstRunComplete(){

	systemCallResponse("touch data/firstruncomplete" + getVersion());
	fprintf(stderr,"Completed first run tasks...\n");

	guiAlert(gStr("STR_FIRST_RUN_INST_REQ"));

	return 0;
}

int saveLog(std::string logdata){
	logdata = systemCallResponse("date") + "\n-----Log-----\n" + logdata;
	FILE * logFile;
	logFile = fopen ("$HOME/../tmp/log.txt","w");
	if (logFile!=NULL)
	{
		fprintf(logFile,"%s",logdata.c_str());
		fclose (logFile);
		return 0;
	}else
	{
		guiAlert(gStr("STR_LOG_WRITE_FAIL"));
	}
	return 1;
}

int showLog(){
	int endState = 1;
	std::string logdata= "";
	FILE * logFile;
	logFile = fopen ("$HOME/../tmp/log.txt","r");
	if (logFile!=NULL)
	{
		endState = 0;
		char buffer[80];
		while( fgets( buffer, 80, logFile ) != NULL ) {
			logdata += buffer;
		}
		fclose(logFile);
	}else
	{
	 logdata = gStr("STR_LOG_READ_FAIL");
	}
	guiAlert(logdata);
	return endState;
}

int saveSettingsToFile(void * settingsAdds [], int settingsTypes [], int numSettings){
	std::string settingsString = "";
	for(int i=0;i<numSettings;i++){
		switch(settingsTypes[i]){
		case STRING_SETTING:
			settingsString += *(std::string *)settingsAdds[i] + "\n";
			break;
		case BOOL_SETTING:
			if(*(bool *)settingsAdds[i]){
				settingsString += "1\n";
			}else{
				settingsString += "0\n";
			}
			break;
		}
	}
//	fprintf(stderr,"%s\n",settingsString.c_str());
	FILE * settingsFile;
	settingsFile = fopen ("$HOME/../data/appSettings.txt","w");
	if (settingsFile!=NULL)
	{
		fprintf(settingsFile,"%s",settingsString.c_str());
		fclose(settingsFile);
	}else
	{
		fprintf(stderr,"Could not save settings.\n");
	}
	return 0;
}

int loadSettingsFromFile(void * settingsAdds [], int settingsTypes [], int numSettings){
	std::string settingsString = "";
	FILE * settingsFile;
	settingsFile = fopen ("$HOME/../data/appSettings.txt","r");
	if (settingsFile!=NULL)
	{
		char buffer[80];
		while( fgets( buffer, 80, settingsFile ) != NULL ) {
			settingsString += buffer;
		}
		fclose(settingsFile);
		if(settingsString.compare("")){
			int strPos = 0, strPosP = 0;
			for(int i=0;i<numSettings;i++){
				strPos = settingsString.find("\n",strPos) + 1;
				std::string subStr = settingsString.substr(strPosP,strPos-strPosP-1);
//				fprintf(stderr,"%i:%i %i - %s\n",i,strPosP,strPos,subStr.c_str());
				if(strPos>strPosP){
					switch(settingsTypes[i]){
					case STRING_SETTING:
						*(std::string *)settingsAdds[i] = subStr;
						break;
					case BOOL_SETTING:
						(!subStr.compare("0"))?*(bool *)settingsAdds[i] = false:*(bool *)settingsAdds[i] = true;
						break;
					}
				}
				strPosP = strPos;
			}
		}
	}else
	{
		fprintf(stderr,"Could not load settings.\n");
	}
//	fprintf(stderr,"%s\n",settingsString.c_str());
	return 0;
}

static std::string getTexBin(){
	std::string TEX_BIN= "";
	FILE * tbfile;
	tbfile = fopen ("$HOME/../data/texlive/.TEXBIN","r");
	if (tbfile!=NULL)
	{
		char buffer[80];
		while( fgets( buffer, 80, tbfile ) != NULL ) {
			TEX_BIN += buffer;
		}
		fclose(tbfile);
	}else
	{
		fprintf(stderr,"Could not load tex distribution settings. Reverting to otgtexlive-2012basic.\n");
		std::string TEX_BIN="$HOME/texlive/otgtexlive-2012basic/bin/arm-unknown-nto-qnx6.5.0eabi";
	}
	return TEX_BIN;
}

void setTexBin(std::string texDist){
	if (texDist.length()>0){
		texDist = "$HOME/texlive/" + texDist + "/bin/arm-unknown-nto-qnx6.5.0eabi";
	}else{
		texDist = getTexBin();
	}
	if (texDist.length()>0){
		std::string expandedTB = systemCallResponse("echo "+texDist);//expand tex path
		expandedTB = expandedTB.substr(0,expandedTB.length()-1);//get rid of newline character
		setenv("TEXBIN",expandedTB.c_str(),1);
		FILE * tbfile;
		tbfile = fopen ("$HOME/../data/texlive/.TEXBIN","w");
		if (tbfile!=NULL)
		{
			fprintf(tbfile,"%s",texDist.c_str());
			fclose(tbfile);
		}else
		{
			fprintf(stderr,"Could not save tex distribution settings to file.\n");
		}
	}else{
		fprintf(stderr,"Could not set tex distribution.\n");
	}
}

void installDistribution(std::string distToInstall){
	guiInfoBox(gStr("STR_FIRST_RUN_EXTRACT"));
	std::string response = systemCallResponse("cd data/texlive; unzip " + distToInstall);
	guiCloseDialog();
	if(!response.substr(0,7).compare("Archive")){
		guiAlert(gStr("STR_ADDDIST_SUCCESS"));
	}else{
		guiAlert(response);
	};
}

void chooseDistribution(){
	std::string newDist= guiFileSelPopup("\b$HOME/texlive");
	if(newDist.length()>0){
		if(!newDist.compare("Add...")){
			guiHandleAddDist();
		}else{
			setTexBin(newDist);
		}
	}
}

void chooseLanguage(){
	std::string newLang= guiFileSelPopup("\b$HOME/../app/native/lang");
	if(newLang.length()>0){
		destroyLocalStrings();
		curLocale = newLang;
		loadLocalStrings();
	}
}

std::string getVersion(){
	std::string version = "";
	FILE * vfile;
	vfile = fopen ("$HOME/../app/native/bar-descriptor.xml","r");
	if (vfile!=NULL)
	{
		char buffer[80];
		while( fgets( buffer, 80, vfile ) != NULL ) {
			version += buffer;
		}
		fclose(vfile);
		int sp1 = version.find("<versionNumber>",0);
		int sp2 = version.find("</versionNumber>",sp1);
		version = version.substr(sp1+15,sp2-sp1-15);
	}else
	{
		fprintf(stderr,"Could not retrieve version number.\n");
	}
	return version;
}

