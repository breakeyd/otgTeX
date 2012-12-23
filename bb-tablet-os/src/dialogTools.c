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

#include "dialogTools.h"

int * popupIndices;
int popupIndicesLength;

int 		selectedIndex;
const char* selectedLabel;
const char* selectedContext;

dialog_instance_t theDialog = 0;

enum {
	DT_DLGTYPE_VOID,
	DT_DLGTYPE_ALERT,
	DT_DLGTYPE_CONTEXT,
	DT_DLGTYPE_PROMPT,
	DT_DLGTYPE_POPUP,
	DT_DLGTYPE_FILEBROWSE,
	DT_DLGTYPE_LOGIN,
	DT_DLGTYPE_FILESAVE,
};
int dialogType = DT_DLGTYPE_VOID;

int showDialog(){
	if (dialog_show(theDialog) != BPS_SUCCESS) {
		fprintf(stderr, "Failed to show dialog.");
		dialog_destroy(theDialog);
		theDialog = 0;
		return BPS_FAILURE;
	}
//	fprintf(stderr,"showed dialog - %i\n",theDialog);
	return BPS_SUCCESS;
}

int closeDialog(){
	dialog_destroy(theDialog);
	theDialog = 0;
	dialogType = DT_DLGTYPE_VOID;
//	fprintf(stderr,"Dialog closed\n");
	return BPS_SUCCESS;
}

int getDialogResponse(bps_event_t *diagevent){
	if (diagevent == NULL) {
		return BPS_FAILURE;
	}

	switch(dialogType){
	case DT_DLGTYPE_POPUP:
	    dialog_event_get_popuplist_selected_indices(diagevent, &popupIndices, &popupIndicesLength);
//	    int ij;
//	    for(ij=0;ij<popupIndicesLength;ij++){
//	    	fprintf(stderr,"popupIndex: %i\n",popupIndices[ij]);
//	    }
		break;
	}
	selectedIndex = dialog_event_get_selected_index(diagevent);
	selectedLabel = dialog_event_get_selected_label(diagevent);
	selectedContext = dialog_event_get_selected_context(diagevent);

//	char output[1024];
//	snprintf(output, 1024, "Selected Index: %d, Label: %s, Context: %s\n",
//			selectedIndex, selectedLabel?selectedLabel:"n/a", selectedContext?(char*)selectedContext:"n/a");
//	fprintf(stderr, output);

	if(selectedContext){//we pass a context only if it is the cancel button
		return BPS_FAILURE;
	}

	return BPS_SUCCESS;
}

int waitForDialogResponse(){
	int respMessage = BPS_SUCCESS;
	if(!theDialog){
		return BPS_FAILURE;
	}
	bps_event_t *diagevent = NULL;
	while(theDialog)  {
//	for(;;){
//		fprintf(stderr,"Waiting for dialog event...\n");
		bps_get_event(&diagevent, -1);
		if (diagevent) {
//			fprintf(stderr,"0x%x\n",bps_event_get_code(diagevent));
			if (bps_event_get_domain(diagevent) == dialog_get_domain()) {
				respMessage = getDialogResponse(diagevent);
				closeDialog();
			}
			else if(bps_event_get_domain(diagevent) == navigator_get_domain()){
				switch( bps_event_get_code(diagevent) ) {
//				case NAVIGATOR_WINDOW_STATE://this will kill orphan, hanging dialogs with an upward swipe or power button press
				//	unfortunately this kills dialogs when the power button is pressed, which is a
				//	likely action if the screen falls asleep during an action (e.g. distro install)
				case NAVIGATOR_SWIPE_DOWN://or a downward swipe (natural user actions in case of a UI freeze)
					closeDialog();
					fprintf(stderr,"The navigator cancelled the dialog.\n");
					return BPS_FAILURE;
					break;
				}
			}
		}
	}
	return respMessage;
}

int buildModalAlert(const char* message,int dlgSize, int nButtons, const char** buttonNames)
{
	bool htmlmessage = false;
	if(message[0]=='<' && message[1]=='h' && message[2]=='t' && message[3]=='m' && message[4]=='l' && message[5]=='>')
		htmlmessage=true;

//	char message[strlen(message0)+1];
//	strcpy(message,message0);
////	//remove non-printable characters
//	int js;
//	for(js=0;js<strlen(message);js++){
//		if(message[js]>255) //|| message[js]<32)
//			message[js]=63;
//	}


//	fprintf(stderr,"%i, %i - %s\n",dlgSize,nButtons,message);
    if (theDialog) {
        fprintf(stderr, "theDialog was not 0\n");
        return BPS_FAILURE;
    }

    if (dialog_create_alert(&theDialog) != BPS_SUCCESS) {
    	fprintf(stderr, "Failed to create alert dialog.");
        return BPS_FAILURE;
    }

    if (dialog_set_size(theDialog,dlgSize) != BPS_SUCCESS) {
    	fprintf(stderr, "Failed to create alert dialog.");
        theDialog = 0;
        return BPS_FAILURE;
    }

    if (htmlmessage?dialog_set_alert_html_message_text(theDialog,message):dialog_set_alert_message_text(theDialog,message) != BPS_SUCCESS) {
        fprintf(stderr, "Failed to set alert dialog message text.");
        dialog_destroy(theDialog);
        theDialog = 0;
        return BPS_FAILURE;
    }

    int i;
    for (i=0;i<nButtons;i++){
    	if (dialog_add_button(theDialog, buttonNames[i], true, 0, true) != BPS_SUCCESS) {
    		fprintf(stderr, "Failed to add button to alert dialog.");
    		dialog_destroy(theDialog);
    		theDialog = 0;
    		return BPS_FAILURE;
    	}
    }

    dialogType = DT_DLGTYPE_ALERT;

    return BPS_SUCCESS;
}

int modalAlertOpt(const char * message,int dlgSize,int nButtons,const char** buttonNames){
	buildModalAlert(message,dlgSize,nButtons,buttonNames);
	showDialog();
	waitForDialogResponse();
	return selectedIndex;
}

int modalAlert(const char * message){
    const char* okmessage[] = {DIALOG_OK_LABEL};
	return modalAlertOpt(message,DIALOG_SIZE_FULL,1,okmessage);
}

int infoBoxAlert(const char * message){
	buildModalAlert(message,DIALOG_SIZE_SMALL,0,NULL);
	showDialog();
	return BPS_SUCCESS;
}

int buildModalPopupList(const char* vals[],int numvals,const char* message,bool multisel){
    if (theDialog) {
        fprintf(stderr, "theDialog was not 0\n");
        return BPS_FAILURE;
    }

    if (dialog_create_popuplist(&theDialog) != BPS_SUCCESS) {
    	fprintf(stderr, "Failed to create filebrowse dialog.");
        return BPS_FAILURE;
    }

    if (dialog_set_title_text(theDialog, message) != BPS_SUCCESS) {
    	fprintf(stderr, "Failed to add message to alert dialog.");
    	dialog_destroy(theDialog);
		theDialog = 0;
		return BPS_FAILURE;
    }

    char* cancel_button_context = "Cancelled";
    if (dialog_add_button(theDialog, DIALOG_CANCEL_LABEL, true, cancel_button_context, true) != BPS_SUCCESS) {
    	fprintf(stderr, "Failed to add button to alert dialog.");
    	dialog_destroy(theDialog);
		theDialog = 0;
		return BPS_FAILURE;
    }
    if (dialog_add_button(theDialog, DIALOG_OK_LABEL, true, 0, true) != BPS_SUCCESS) {
    	fprintf(stderr, "Failed to add button to alert dialog.");
    	dialog_destroy(theDialog);
		theDialog = 0;
		return BPS_FAILURE;
    }

    if (dialog_set_popuplist_items(theDialog,vals,numvals) != BPS_SUCCESS) {
    	fprintf(stderr, "Failed to set prompt items.");
    	dialog_destroy(theDialog);
		theDialog = 0;
		return BPS_FAILURE;
    }

    if (dialog_set_popuplist_multiselect(theDialog,multisel) != BPS_SUCCESS) {
    	fprintf(stderr, "Failed to set multiselect.");
    	dialog_destroy(theDialog);
		theDialog = 0;
		return BPS_FAILURE;
    }

    dialogType = DT_DLGTYPE_POPUP;
    return BPS_SUCCESS;
}

int * modalPopupList(const char* vals[],int * numvals,const char* message,bool multisel){
	buildModalPopupList(vals,*numvals,message,multisel);
	showDialog();
	if (waitForDialogResponse()==BPS_SUCCESS){
		*numvals = popupIndicesLength;
		//		static int[popupIndicesLength+1] indexReturn;
		//		static
		return popupIndices;
		//		bps_free(popupIndices);
	}else{
		*numvals=0;

	}
    return NULL;
}

int buildModalFilesel(){
    if (theDialog) {
        fprintf(stderr, "theDialog was not 0\n");
        return BPS_FAILURE;
    }

    if (dialog_create_filebrowse(&theDialog) != BPS_SUCCESS) {
    	fprintf(stderr, "Failed to create filebrowse dialog.");
        return BPS_FAILURE;
    }

    const char* exts[] = {"*.*"};
    int numexts = 1;
    if (dialog_set_filebrowse_filter(theDialog,exts,numexts) != BPS_SUCCESS) {
    	fprintf(stderr, "Failed to set filebrowse filter.");
    	theDialog = 0;
        return BPS_FAILURE;
    }

//    int dialog_event_get_filebrowse_filepaths(bps_event_t *event, char **file_paths[], int *num_paths)
//    if (dialog_add_button(theDialog, "OK", true, 0, true) != BPS_SUCCESS) {
//        fprintf(stderr, "Failed to add button to alert dialog.");
//        dialog_destroy(theDialog);
//        theDialog = 0;
//        return BPS_FAILURE;
//    }

    dialogType = DT_DLGTYPE_FILEBROWSE;
    return BPS_SUCCESS;
}

int modalFilesel(){
	buildModalFilesel();
	showDialog();
	waitForDialogResponse();
	return BPS_SUCCESS;
};
