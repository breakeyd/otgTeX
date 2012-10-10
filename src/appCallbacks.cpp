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

#include "appCallbacks.h"

//local functions
static void buildExecString();
static void executeCommand();

//app fields (these hard-coded defaults will be overwritten when loadSettings() is called)
std::string execDir     = "$HOME/../shared/downloads";
std::string execFile    = "test.tex";
std::string execCommand = "$TEXBIN/pdftex -fmt=pdflatex -halt-on-error ";
std::string execString  = "cmd:";//gStr("STR_DEFAULT_CMD");

//process settings (these hard-coded defaults will be overwritten when loadSettings() is called)
bool autoDisplayPDF    = true;
bool autoBuildCommand  = true;
bool autoDisplayResult = true;
std::string TEX_BIN = "$HOME/texlive/otgtexlive-2012basic/bin/arm-unknown-nto-qnx6.5.0eabi";

//addresses and types for saving settings
void * settingsAdds []  = {	&execDir,&execFile,&execCommand,&execString,&curLocale,
							&autoDisplayResult,&autoBuildCommand,&autoDisplayPDF};
int settingsTypes []	= {	STRING_SETTING,STRING_SETTING,STRING_SETTING,STRING_SETTING,STRING_SETTING,
							BOOL_SETTING,BOOL_SETTING,BOOL_SETTING};

static void buildExecString(){
    execString = "cd " + execDir + "; " + execCommand + " " + execFile;
}

static void executeCommand(){
	if(autoBuildCommand){
		buildExecString();
	}
	std::string outputFile = execDir + "/" + execFile.substr(0,execFile.find(".tex",0)) + ".pdf";
	outputFile = systemCallResponse("ls -Ld "+ outputFile);//expand path
	outputFile = outputFile.substr(0,outputFile.length()-1);//remove newline character
	guiRender();//this provides confirmation to the user that the right command is being run
	guiInfoBox(gStr("STR_PROCESSING_CMD"));
	std::string execResult = systemCallResponse(execString);
	saveLog(execResult);
	guiCloseDialog();
    if (autoDisplayResult){
    	guiAlert(execResult);
	}
    if(autoDisplayPDF){
//		fprintf(stderr,"Opening file: %s\n",outputFile.c_str());
		navigator_open_file(outputFile.c_str(),NULL);
	}
}

void saveSettings(){
	saveSettingsToFile(settingsAdds,settingsTypes,sizeof(settingsAdds)/sizeof(settingsAdds[0]));
}
void loadSettings(){
	loadSettingsFromFile(settingsAdds,settingsTypes,sizeof(settingsAdds)/sizeof(settingsAdds[0]));
}
void restoreDefaultSettings(){
	systemCallResponse("cp app/native/appSettings-default.txt data/appSettings.txt");
	loadSettings();
}

void bcbChooseFile(){
	std::string selectedFile = guiFileSelPopup(execDir);
	if(selectedFile.compare("")){
		int lastSlash = selectedFile.find_last_of("/",std::string::npos);
		std::string selectedPath = selectedFile.substr(0,lastSlash);
		selectedFile = selectedFile.substr(lastSlash+1,std::string::npos);
//		fprintf(stderr,">%s\n%s<mainfull\n",selectedPath.c_str(),selectedFile.c_str());
		execDir = selectedPath;
		execFile = selectedFile;
	}
}

void bcbShowLog(){
	showLog();
}

void bcbExecute(){
	executeCommand();
}

void bcbToggleApdf(){
	autoDisplayPDF = !autoDisplayPDF;
}
void bcbToggleAbc(){
	autoBuildCommand = !autoBuildCommand;
}
void bcbToggleAdr(){
	autoDisplayResult = !autoDisplayResult;
}
