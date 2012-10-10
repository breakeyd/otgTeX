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

#include "languageTools.h"

//local functions
static void matchLocale(std::string language, std::string country);
static int readLangFile(std::string *langFileText);

std::string curLocale = "Auto";
std::string setLocale;

static std::string * appStringNames;
static std::string * appStrings;
static unsigned int strCount;

static void matchLocale(std::string language, std::string country){
	if(!language.compare("en")){
		setLocale = "en_CA";
		return;
	}
	if(!language.compare("de")){
		setLocale = "de_DE";
		return;
	}
	setLocale = language + "_" + country;
}

static int readLangFile(std::string *langFileText){
//	setLocale = "en_CA";//temporary to force english

	FILE * langFile;
	langFile = fopen (("$HOME/../app/native/lang/"+setLocale).c_str(),"r");
	if (langFile!=NULL)
	{
		char buffer[80];
		while( fgets( buffer, 80, langFile ) != NULL ) {
			*langFileText += buffer;
		}
		fclose(langFile);
//		fprintf(stderr,"%s",(*langFileText).c_str());
	}else{
		fprintf(stderr,"Could not load %s, reverting to en_CA\n",setLocale.c_str());
		setLocale = "en_CA";
		return 1;
	}

	return 0;
}

int loadLocalStrings(){
	setLocale = curLocale;
	if(!curLocale.compare("Auto")){
		char* country = NULL;
		char* language = NULL;
		locale_get(&language, &country);
		//    fprintf(stderr,"country:%s, language:%s\n",country,language);
		matchLocale((std::string)language,(std::string)country);
		bps_free((char*)language);
		bps_free((char*)country);
	}else{
		if(curLocale.length()>=5){
			matchLocale(curLocale.substr(0,2),curLocale.substr(3,2));
		}
	}

	std::string langFileText = "";
	if(readLangFile(&langFileText))
		readLangFile(&langFileText);

	strCount = 0;
	unsigned int strPos = 0;
	while(strPos<langFileText.length() && strPos!=std::string::npos){
		strPos = langFileText.find("STR_",strPos);
		if(strPos!=std::string::npos){
			strCount++;
			strPos++;
		}
	}
//	fprintf(stderr,"%i strings found\n",strCount);
	appStrings = new std::string[strCount];
	appStringNames = new std::string[strCount];

	strPos=0;
	unsigned int strPosE,strPosET = 0;
	for(unsigned int strInd=0;strInd<strCount;strInd++){
		strPos  = langFileText.find("STR_",strPos);
		strPosE = langFileText.find(" ",strPos);
		strPosET= langFileText.find("\t",strPos);
		strPosE = (strPosE<strPosET)?strPosE:strPosET;
//		fprintf(stderr,"%i -> %i\n",strPos,strPosE);
//		fprintf(stderr,"%s\n",(langFileText.substr(strPos,strPosE-strPos)).c_str());
		appStringNames[strInd] = langFileText.substr(strPos,strPosE-strPos);
		strPos = langFileText.find(":",strPos)+1;
		strPosE = langFileText.find("\n",strPos);
//		fprintf(stderr,"%s\n",(langFileText.substr(strPos,strPosE-strPos)).c_str());
		appStrings[strInd] = langFileText.substr(strPos,strPosE-strPos);
//		fprintf(stderr,"%s = %s\n",appStringNames[strInd].c_str(),appStrings[strInd].c_str());
	}

    return EXIT_SUCCESS;
}
void destroyLocalStrings(){
	delete [] appStringNames;
	delete [] appStrings;
}

std::string gStr(std::string strName){
	for(unsigned int strInd = 0;strInd<strCount;strInd++){
		if( !(strName.compare(appStringNames[strInd])) ){
			return appStrings[strInd];
		}
	}
	return strName;
}
