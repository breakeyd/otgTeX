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

#include "guiOpenGL.h"

//local functions
static int selectField (int touchx, int touchy);
static void limitCursor();
static int pressButton(int touchx, int touchy);
static void showMenu();
static std::string fileSelPopup(std::string directory);
static std::string cdPopup(std::string directory);


//local variables
static std::string dispString  = "";//"otgTeX!";

static GLuint background;
static GLfloat vertices[8];
static GLfloat tex_coord[8];
static float pos_x, pos_y;
static font_t* font;

//this probably should have been done with a field structure
static std::string *theFieldStrings[] = {&execDir,&execFile,&execCommand,&execString};
static float fieldStringPos[] = {0.65,0.55,0.45,0.8};
static int fieldStringBounds[8];
static int selFieldCursors[] = {0,0,0,0,0};
static int activeFieldString = 0;


//this probably should have been done with a button structure
static bool alwaysTrue = true;
static std::string theBtnStrings[] = {"STR_BTN_FILESEL","STR_BTN_SHOWLOG","STR_BTN_EXEC","STR_BTN_AUTOPDF","STR_BTN_AUTOBUILDCOMMAND","STR_BTN_AUTODISPRESULT"};
static void (*theBtnCallbacks[])(void) = {&bcbChooseFile,&bcbShowLog,&bcbExecute,&bcbToggleApdf,&bcbToggleAbc,&bcbToggleAdr};
static bool * btnContents[] = {&alwaysTrue,&alwaysTrue,&alwaysTrue,&autoDisplayPDF,&autoBuildCommand,&autoDisplayResult};
static float btnStringPos[12] = {0.25,0.1,0.25,0.2,0.25,0.3,0.75,0.1,0.75,0.2,0.75,0.3};
static int btnStringBounds[24];

static int curSelCursor = 0;
static int curStrLength = 0;
static int curSens      = 4;

static int selectField (int touchx, int touchy){
	int selectedField = activeFieldString;
    selFieldCursors[selectedField] = curSelCursor;
    for(unsigned int i=0;i<(sizeof(theFieldStrings)/sizeof(theFieldStrings[0]));i++){
//		fprintf(stderr,"%i: %i %i %i\n",i,touchy,fieldStringBounds[2*i],fieldStringBounds[2*i+1]);
    	if(fieldStringBounds[2*i] <= touchy && touchy <= fieldStringBounds[2*i+1]){
    		selectedField = i;
//            fprintf(stderr,"Selected field %i\n",selectedField);
    	}
    }
    activeFieldString = selectedField;
    curSelCursor = selFieldCursors[selectedField];
    curStrLength = (*theFieldStrings[activeFieldString]).length();
	return selectedField;
}

static void limitCursor(){
	if (curSelCursor<0){
		curSelCursor = 0;
	}else if(curSelCursor>curStrLength){//-1
		curSelCursor=curStrLength;//-1
	}
}

static int pressButton(int touchx, int touchy){
	int selectedBtn = -1;
    for(unsigned int i=0;i<(sizeof(theBtnStrings)/sizeof(theBtnStrings[0]));i++){
//		fprintf(stderr,"%i:x: %i %i %i\n",i,touchx,btnStringBounds[4*i],btnStringBounds[4*i+1]);
//		fprintf(stderr,"%i:y: %i %i %i\n",i,touchy,btnStringBounds[4*i+2],btnStringBounds[4*i+3]);
		if( 	btnStringBounds[4*i]   <= touchx && touchx <= btnStringBounds[4*i+1]
			&&	btnStringBounds[4*i+2] <= touchy && touchy <= btnStringBounds[4*i+3]){
    		selectedBtn = i;
//        	fprintf(stderr,"Selected button %i\n",selectedBtn);
    		break;
    	}
    }
    if(selectedBtn>=0){
    	(*theBtnCallbacks[selectedBtn])();
    }
	return selectedBtn;
}

static void showMenu(){
	char b1[128],b2[128],b3[128],b4[128];//maximum button label length
	strcpy(b1,gStr("STR_MENU_CHOOSE_TEX").c_str());
	strcpy(b2,gStr("STR_MENU_CHOOSE_LANGUAGE").c_str());
	strcpy(b3,gStr("STR_MENU_CHOOSE_HELP").c_str());
	strcpy(b4,gStr("STR_MENU_CHOOSE_RESET").c_str());
	const char* buttontexts[] = {DIALOG_CANCEL_LABEL,b1,b2,b3,b4};//gStr("STR_MENU_CHOOSE_LANGUAGE").c_str()};
	int selectedButton = modalAlertOpt(
			("<html>otgTeX " + getVersion()).c_str()
			,DIALOG_SIZE_LARGE,sizeof(buttontexts)/sizeof(buttontexts[0]),buttontexts);
	switch(selectedButton){
	case 1:
		chooseDistribution();
		break;
	case 2:
		chooseLanguage();
		break;
	case 3:
		guiAlert(gStr("STR_INST_CONT"));
		break;
	case 4:{
		const char* confButtons []= {DIALOG_CANCEL_LABEL,DIALOG_OK_LABEL};
		if(modalAlertOpt(gStr("STR_MENU_CHOOSE_RESET_CONF").c_str(),DIALOG_SIZE_SMALL,2,confButtons))
			restoreDefaultSettings();
	}
		break;
	}
}

std::string fileSelPopup(std::string directory){
	int numEnt = 0;
	std::string cmd = "ls -a1 " + directory + " | sort -f";
	if(*(directory.c_str())=='\b')//hide hidden files if directory is proceeded by backspace
	{
		directory = directory.substr(1,std::string::npos);
		cmd = "ls -1 " + directory + " | sort -f";
	}
	std::string dirContents = systemCallResponse(cmd);

//	fprintf(stderr,"--dCStart--\n%s\n--dCEND--\n",dirContents.c_str());

//	int separaterLocs[dirContents.size()];
	int * separaterLocs = new int[dirContents.size()+1];
	separaterLocs[0]=-1;
	int maxStrLen = 0;

	if(dirContents[0] == '\n'){
		separaterLocs[1] = 0;
		numEnt++;
	}

	for(unsigned int i=1;i<dirContents.size();i++){
		if (dirContents[i] == '\n'){
			separaterLocs[numEnt+1] = i;
			int theStrLen = i-separaterLocs[numEnt-1];
			if(theStrLen>maxStrLen)
				maxStrLen=theStrLen;
			numEnt++;
		}
	}

	#define maxFilenameLen 256
	char ents[numEnt][maxFilenameLen];//maxStrLen
	const char* entsadd[numEnt];

	for(int i=0;i<numEnt;i++){
		strcpy(ents[i],((dirContents.substr(separaterLocs[i]+1,separaterLocs[i+1]-separaterLocs[i]-1)).substr(0,maxFilenameLen)).c_str());
		entsadd[i]= &ents[i][0];
//		fprintf(stderr,"%s <---\n",ents[i]);
	}
//	fprintf(stderr,"--numEnts--\n%i <---\n",numEnt);

	delete [] separaterLocs;              // Deallocate memory for the array
	separaterLocs = NULL;

	int numSel = numEnt;
	int * selAddress = modalPopupList(entsadd,&numSel,(gStr("STR_CONTENTS_OF")+" "+directory).c_str(),false);
//	fprintf(stderr,"%i selected\n",numSel);
	if (numSel==1){
//		fprintf(stderr,"%s/%s<--Was selected\n",directory.c_str(),ents[*selAddress]);
		std::string returnValue = ents[*selAddress];
		bps_free(selAddress);
		return returnValue;
	}
	bps_free(selAddress);

//	fprintf(stderr,"--File select popup cancelled with-->%s<---selected\n",directory.c_str());
	return "";
}

std::string cdPopup(std::string directory){
	std::string dValid = systemCallResponse("[ -d "+ directory +" ] && echo 'Y' || echo 'N'");
	fprintf(stderr,"%s\n%s\n",directory.c_str(),dValid.c_str());
	if(!dValid.compare("N\n")){//if it wasn't a valid directory, we'll assume shared
		directory = "shared";
	}

	std::string selectedFile = fileSelPopup(directory);
//	fprintf(stderr,"%s<incdpopup\n",selectedFile.c_str());
	if (!selectedFile.compare("")){
//		fprintf(stderr,"noFile\n");
		return "";
	}else if(!selectedFile.compare(".")){
//		fprintf(stderr,"dirSelected\n");
		return directory;
	}else if(!(systemCallResponse("ls -a "+ directory + "/" +selectedFile + "/").substr(0,13)).compare("ls: Not a dir")){
//		fprintf(stderr,"fileSelected\n");
		return directory + "/" +selectedFile;
	}
//	fprintf(stderr,"Recurse into...-->%s<--\n",selectedFile.c_str());
	return cdPopup(directory+ ((directory.length()>0)?"/":"") +selectedFile);
}

//Tie-ins to dialogTools (used so that a new gui can be written without requiring dialogTools)
void guiInfoBox(std::string istr){
	infoBoxAlert(istr.c_str());
}
void guiAlert(std::string istr){
#define DLG_MAX_ALERT_CHAR 10000//30000 is the real max, 10000 makes the dialog more responsive
	unsigned int istrl = istr.length();
//	fprintf(stderr,"strlen=%i\n",istrl);
	if(istrl<DLG_MAX_ALERT_CHAR){
		modalAlert(istr.c_str());
	}else{
		unsigned int st1 = 0, res = 0;
		char b1[128],b2[128];//maximum button label length
		strcpy(b1,gStr("STR_ALERT_PAGEREV").c_str());
		strcpy(b2,gStr("STR_ALERT_PAGEADV").c_str());
		const char* buttontexts[] = {b1,DIALOG_OK_LABEL,b2};
		while(res!=1){
//			fprintf(stderr,"--->/n--->\n--->\n--->\n--->\n%i-%i-%s\n",istr.substr(st1,DLG_MAX_ALERT_CHAR).length(),DLG_MAX_ALERT_CHAR,istr.substr(st1,DLG_MAX_ALERT_CHAR).c_str());
			std::string disps = istr.substr(st1,DLG_MAX_ALERT_CHAR);
//			for(unsigned int ijs=0;ijs<disps.length();ijs++)//unfortunately, the dialog can't display '<' because the alert shows html-formatted text
//				if(disps[ijs]=='<')
//					disps[ijs]='?';
			res=modalAlertOpt(disps.c_str(),DIALOG_SIZE_FULL,3,buttontexts);
			switch(res){
			case 0:
				if(st1>=DLG_MAX_ALERT_CHAR)
					st1 -= DLG_MAX_ALERT_CHAR;
				break;
			case 2:
				if(st1<istrl-DLG_MAX_ALERT_CHAR)
					st1 += DLG_MAX_ALERT_CHAR;
				break;
			}
		}
	}
}

std::string guiFileSelPopup(std::string istr){
	if(istr[0]=='\b'){
		return fileSelPopup(istr.c_str());
	}else{
		return cdPopup(istr.c_str());
	}
}

void guiCloseDialog(){
	closeDialog();
}
int guiAlertQuery(std::string istr){
	const char* buttontexts[] = {DIALOG_CANCEL_LABEL,DIALOG_OK_LABEL};
	return modalAlertOpt(istr.c_str(),DIALOG_SIZE_FULL,2,buttontexts);
}

//public gui functions
int guiInit(){
    //Use utility code to initialize EGL for rendering with GL ES 1.1
    if (EXIT_SUCCESS != bbutil_init_egl(screen_cxt)) {
        fprintf(stderr, "Unable to initialize EGL\n");
        return EXIT_FAILURE;
    }

	EGLint surface_width, surface_height;

	int dpi = bbutil_calculate_dpi(screen_cxt);

	font = bbutil_load_font("/usr/fonts/font_repository/adobe/MyriadPro-Bold.otf", 15, dpi);
	if (!font) {
        bbutil_terminate();
		return EXIT_FAILURE;
	}

	//Load background texture
	float tex_x, tex_y;
	if (EXIT_SUCCESS
			!= bbutil_load_texture("app/native/otgtex_bkgd.png",
					NULL, NULL, &tex_x, &tex_y, &background)) {
		fprintf(stderr, "Unable to load background texture\n");
	}

	//Query width and height of the window surface created by utility code
	eglQuerySurface(egl_disp, egl_surf, EGL_WIDTH, &surface_width);
	eglQuerySurface(egl_disp, egl_surf, EGL_HEIGHT, &surface_height);

	EGLint err = eglGetError();
	if (err != 0x3000) {
		fprintf(stderr, "Unable to query EGL surface dimensions\n");
		bbutil_destroy_font(font);
        bbutil_terminate();
		return EXIT_FAILURE;
	}

	width = (float) surface_width;
	height = (float) surface_height;

	//Initialize GL for 2D rendering
	glViewport(0, 0, (int) width, (int) height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrthof(0.0f, width / height, 0.0f, 1.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//Set world coordinates to coincide with screen pixels
	glScalef(1.0f / height, 1.0f / height, 1.0f);

	float text_width, text_height;
	bbutil_measure_text(font, dispString.c_str(), &text_width, &text_height);
	pos_x = width/10;//(width - text_width) / 2;
	pos_y = height*9/10;


	//Setup background polygon
	vertices[0] = 0.0f;
	vertices[1] = 0.0f;
	vertices[2] = width;
	vertices[3] = 0.0f;
	vertices[4] = 0.0f;
	vertices[5] = height;
	vertices[6] = width;
	vertices[7] = height;

	tex_coord[0] = 0.0f;
	tex_coord[1] = 0.0f;
	tex_coord[2] = tex_x;
	tex_coord[3] = 0.0f;
	tex_coord[4] = 0.0f;
	tex_coord[5] = tex_y;
	tex_coord[6] = tex_x;
	tex_coord[7] = tex_y;

	curStrLength = execString.length();

	//measure field y bounds
	float text_width_exec, text_height_exec;
	for(unsigned int i=0;i<(sizeof(theFieldStrings)/sizeof(theFieldStrings[1]));i++){
		bbutil_measure_text(font, (*theFieldStrings[i]).c_str(), &text_width_exec, &text_height_exec);
		if(text_height_exec<25)
			text_height_exec=25;
		fieldStringBounds[2*i] = fieldStringPos[i]*height;
		fieldStringBounds[2*i+1] = fieldStringPos[i]*height+text_height_exec;
	}
	//measure button xy bounds
	for(unsigned int i=0;i<(sizeof(theBtnStrings)/sizeof(theBtnStrings[1]));i++){
		bbutil_measure_text(font, gStr(theBtnStrings[i]).c_str(), &text_width_exec, &text_height_exec);
		btnStringBounds[4*i]   = btnStringPos[2*i]*width-text_width_exec/2;
		btnStringBounds[4*i+1] = btnStringPos[2*i]*width+text_width_exec/2;
		btnStringBounds[4*i+2] = btnStringPos[2*i+1]*height;
		btnStringBounds[4*i+3] = btnStringPos[2*i+1]*height+text_height_exec;
	}

	return EXIT_SUCCESS;
}

void guiRender(){
	//Typical rendering pass
		glClear(GL_COLOR_BUFFER_BIT);

		//Render background quad first
		glEnable(GL_TEXTURE_2D);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glTexCoordPointer(2, GL_FLOAT, 0, tex_coord);
		glBindTexture(GL_TEXTURE_2D, background);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisable(GL_TEXTURE_2D);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		//Use utility code to render welcome text onto the screen
		bbutil_render_text(font, dispString.c_str(), pos_x, pos_y, 0.8f, 1.0f, 0.8f, 1.0f);

		//render buttons
		float text_width_exec, text_height_exec, pos_x_exec, pos_y_exec;
		for(unsigned int i=0;i<(sizeof(theBtnStrings)/sizeof(theBtnStrings[1]));i++){
			bbutil_measure_text(font, gStr(theBtnStrings[i]).c_str(), &text_width_exec, &text_height_exec);
			pos_x_exec =  width*btnStringPos[2*i] - text_width_exec/2;
			pos_y_exec = height*btnStringPos[2*i+1];
			if(*btnContents[i]){
				bbutil_render_text(font, gStr(theBtnStrings[i]).c_str(), pos_x_exec, pos_y_exec, 1.0f, 1.0f, 1.0f, 1.0f);
			}else{
				bbutil_render_text(font, gStr(theBtnStrings[i]).c_str(), pos_x_exec, pos_y_exec, 0.5f, 0.5f, 0.5f, 1.0f);
			}
		}

		//render fields as well
		selFieldCursors[activeFieldString] = curSelCursor;
		//	float text_width_exec, text_height_exec, pos_x_exec, pos_y_exec;
		for(unsigned int i=0;i<(sizeof(theFieldStrings)/sizeof(theFieldStrings[1]));i++){
			bbutil_measure_text(font, (*theFieldStrings[i]).c_str(), &text_width_exec, &text_height_exec);
			if (text_width_exec<=0.975*width){
				pos_x_exec = (width - text_width_exec) / 2;
			}else{
				pos_x_exec = (0.5*width - text_width_exec*selFieldCursors[i]/((*theFieldStrings[i]).length()));
			}
			pos_y_exec = height * fieldStringPos[i];
			bbutil_render_text(font, (*theFieldStrings[i]).c_str(), pos_x_exec, pos_y_exec, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		//underline field cursor selection
		float csl=0,csw=0,csh=0;//,csl1=0,csh1=0;
		std::string theDispString = *theFieldStrings[activeFieldString];
		bbutil_measure_text(font, theDispString.c_str(), &text_width_exec, &text_height_exec);
		if (text_width_exec<=0.975*width){
			pos_x_exec = (width - text_width_exec) / 2;
		}else{
			pos_x_exec = (0.5*width - text_width_exec*selFieldCursors[activeFieldString]/((*theFieldStrings[activeFieldString]).length()));
		}
		pos_y_exec = height * fieldStringPos[activeFieldString];
		int numByte = mblen( theDispString.c_str()+curSelCursor, MB_LEN_MAX );
		while(	numByte < 0 &&
				(	*(theDispString.c_str()+curSelCursor)<UNICODE_BASIC_LATIN_FIRST
				||	*(theDispString.c_str()+curSelCursor)>UNICODE_BASIC_LATIN_LAST))
		{
			curSelCursor -= 1;
			numByte = mblen( theDispString.c_str()+curSelCursor, MB_LEN_MAX );
		}
		bbutil_measure_text(font, (theDispString.substr(0,curSelCursor)).c_str(), &csl, &csh);
//		bbutil_measure_text(font, (theDispString.substr(0,curSelCursor+numByte)).c_str(), &csl1, &csh1);
		//    fflush(stdout);
		csh = 30;//px high
		csw = 3;//csl1-csl;
		glEnableClientState(GL_VERTEX_ARRAY);
		float vertices[8];
		vertices[0] = 0.0f + pos_x_exec + csl;
		vertices[1] = 0.0f + pos_y_exec - 0.05*csh;
		vertices[2] = 1.0f*csw + pos_x_exec + csl;
		vertices[3] = 0.0f + pos_y_exec - 0.05*csh;
		vertices[4] = 0.0f + pos_x_exec + csl;
		vertices[5] = 1.0f*csh + pos_y_exec - 0.05*csh;
		vertices[6] = 1.0f*csw + pos_x_exec + csl;
		vertices[7] = 1.0f*csh + pos_y_exec - 0.05*csh;
		//    fprintf(stderr,"%5.3f %5.3f\n",vertices[0],vertices[2]);
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glPushMatrix();
		glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glPopMatrix();
		glDisableClientState(GL_VERTEX_ARRAY);


		//Use utility code to update the screen
		bbutil_swap();
}

void guiTerminate(){
	//Destroy the font
	bbutil_destroy_font(font);
    //Use utility code to terminate EGL setup
    bbutil_terminate();
}

void guiHandleTouch(mtouch_event_t mtouch_event){
	if (pressButton(mtouch_event.x, height - mtouch_event.y)<0){
		selectField(mtouch_event.x, height - mtouch_event.y);
	}
}

void guiHandlePan(mtouch_event_t mtouch_event,int last_touch[]){
	curSelCursor -= ((last_touch[0] - mtouch_event.x)/curSens);
	limitCursor();
}

void guiHandleKeyboard(int screen_val){
	if (screen_val>=UNICODE_BASIC_LATIN_FIRST && UNICODE_LATIN_1_SUPPLEMENT_LAST >=screen_val ){//UNICODE_BASIC_LATIN_LAST//UNICODE_LATIN_1_SUPPLEMENT_LAST
		wchar_t addChar = screen_val;
		char addString[MB_LEN_MAX+1];
		int wstr_len = wctomb( addString, addChar);
		addString[wstr_len] = '\0';
//		int numByte = mblen( (*theFieldStrings[activeFieldString]).c_str()+curSelCursor, MB_LEN_MAX );
//		curSelCursor += numByte - 1;

//		while(	numByte < 0 && (*theFieldStrings[activeFieldString]).length() &&
//				(	*((*theFieldStrings[activeFieldString]).c_str()+curSelCursor)<UNICODE_BASIC_LATIN_FIRST
//				||	*((*theFieldStrings[activeFieldString]).c_str()+curSelCursor)>UNICODE_BASIC_LATIN_LAST))
//		{
//			curSelCursor += 1;
//			numByte = mblen( (*theFieldStrings[activeFieldString]).c_str()+curSelCursor, MB_LEN_MAX );
//		}
		if((*theFieldStrings[activeFieldString]).length()==0 || curSelCursor==0)
//			(*theFieldStrings[activeFieldString]) = (std::string)addString;
			(*theFieldStrings[activeFieldString]) = (std::string)addString + (*theFieldStrings[activeFieldString]);
		else if((int)((*theFieldStrings[activeFieldString]).length())==curSelCursor)
			(*theFieldStrings[activeFieldString]) += (std::string)addString;
		else
			(*theFieldStrings[activeFieldString]) = (*theFieldStrings[activeFieldString]).substr(0,curSelCursor) + (std::string)addString + (*theFieldStrings[activeFieldString]).substr(curSelCursor,(*theFieldStrings[activeFieldString]).length()-curSelCursor);
		curSelCursor += wstr_len;
//		for (unsigned int ij=0;ij<(*theFieldStrings[activeFieldString]).length();ij++)
//			fprintf(stderr,"/%04.0i",*((*theFieldStrings[activeFieldString]).substr(ij,1)).c_str());
//		fprintf(stderr,"\n%s\n",(*theFieldStrings[activeFieldString]).c_str());
	}
	else{
		switch (screen_val) {
		case KEYCODE_BACKSPACE://curSelCursor
		{
			if((*theFieldStrings[activeFieldString]).length()>0){
				if(curSelCursor==0)
					curSelCursor++;
				curSelCursor--;
				int numByte = 0;//, shift = 0;
				numByte = mblen( ((*theFieldStrings[activeFieldString]).c_str())+curSelCursor, MB_LEN_MAX );
				while(	( numByte<0 &&
						(	*((*theFieldStrings[activeFieldString]).c_str()+curSelCursor)<UNICODE_BASIC_LATIN_FIRST
						||	*((*theFieldStrings[activeFieldString]).c_str()+curSelCursor)>UNICODE_BASIC_LATIN_LAST)))
				{
					curSelCursor -= 1;
					numByte = mblen( ((*theFieldStrings[activeFieldString]).c_str())+curSelCursor, MB_LEN_MAX );
				}
				(*theFieldStrings[activeFieldString]) =
						(*theFieldStrings[activeFieldString]).erase(curSelCursor,numByte);
			}
		}
			break;
		case KEYCODE_RETURN:
			bcbExecute();
			break;
		default:
			//fprintf(stderr,"Unhandled input! (%c, %i) \n",(char)screen_val,screen_val);
			break;
		}
	}
	curStrLength = (*theFieldStrings[activeFieldString]).length();
	limitCursor();
}

void guiHandleSwipeDown(){
	showMenu();
}

void guiHandleAddDist(){
	char b1[128],b2[128];//maximum button label length
	strcpy(b1,gStr("STR_ADDDIST_FILE").c_str());
	strcpy(b2,gStr("STR_ADDDIST_DOWNLOAD").c_str());
	const char* buttontexts[] = {DIALOG_CANCEL_LABEL,b1,b2};
	int selectedButton = modalAlertOpt(gStr("STR_ADDDIST_MESSAGE").c_str()
					,DIALOG_SIZE_LARGE,sizeof(buttontexts)/sizeof(buttontexts[0]),buttontexts);
	switch(selectedButton){
	case 1:{
		std::string distarchive = cdPopup("$HOME/../shared/downloads");
		if(distarchive.length()>0){
			if(distarchive.find(".zip")==std::string::npos){//if it's not a zip file we advise and abort
				modalAlert(gStr("STR_ADDDIST_FILE_BADSEL").c_str());
				return guiHandleAddDist();
			}else{
				if(!guiAlertQuery(gStr("STR_ADDDIST_FILE_QUERY"))){
					return guiHandleAddDist();
				}
				fprintf(stderr,"Installing from: %s\n",distarchive.c_str());//show what we are doing
				int s1 = distarchive.find_last_of("/")+1;
				int s2 = distarchive.find_last_of(".");
				std::string distArc = distarchive.substr(s1,s2-s1);
				std::string response = systemCallResponse(("[ -d data/texlive/" + distArc + " ] && echo 'Y'").c_str());//does it already exist?
				if(response.compare("Y\n"))
				{//doesn't exist, so go for it
					installDistribution(distarchive);
				}else{//does exist, so confirm overwrite
					const char* confButtons []= {DIALOG_CANCEL_LABEL,DIALOG_OK_LABEL};
					if(modalAlertOpt(gStr("STR_ADDDIST_OVERWRITE_CONF").c_str(),DIALOG_SIZE_SMALL,2,confButtons)){
						guiInfoBox(gStr("STR_ADDDIST_OVERWRITE_DEL"));
						systemCallResponse(("rm -R data/texlive/" + distArc).c_str());//delete previous version before writing
						closeDialog();
						fprintf(stderr,"%s\n",("rm -R data/texlive/" + distArc).c_str());
						installDistribution(distarchive);
					}else{
						return guiHandleAddDist();
					}
				}
			}
		}else{
			return guiHandleAddDist();
		}
	}
	break;
	case 2:
		modalAlert(gStr("STR_ADDDIST_DOWNLOAD_INSTRUCT").c_str());
		navigator_invoke("http://github.com/breakeyd/otgtex/tree/master/texsrc/distros-zipped",NULL);//redirects to github to download new dists
	break;
	}
}
