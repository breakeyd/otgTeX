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

#include <bps/virtualkeyboard.h>

#include "activeGui.h"
#include GUI_HEADER_FILE

#include "languageTools.h"
#include "qnxExtensions.h"
#include "taskUtils.h"

//shared gui params
float width, height;
screen_context_t screen_cxt;

//local params
int exit_application = 0;
static virtualkeyboard_layout_t keyboardLayout = virtualkeyboard_layout_t(0);
int last_touch[2] = { 0, 0 };

void changeKeyboardType(int newKBType){
	(newKBType>=0 && newKBType<8)?:newKBType=0;
	keyboardLayout = virtualkeyboard_layout_t(newKBType);
    virtualkeyboard_change_options(keyboardLayout, VIRTUALKEYBOARD_ENTER_DEFAULT);
}

int init() {
    loadSettings();
    if (EXIT_SUCCESS != loadLocalStrings()) {//initialize local translation
        fprintf(stderr, "Unable to initialize translation\n");
        return EXIT_FAILURE;
    }

    changeKeyboardType(keyboardLayout);
    if (!checkForFirstRun()){
    	navigator_extend_timeout(1800000,NULL);//we now have 30 minutes to complete installation
    	if(!guiAlertQuery(gStr("STR_FIRST_RUN_QUERY"))){
    		fprintf(stderr,"Cancelled first run.\n");
    		return EXIT_FAILURE;
    	}
    	firstRunTasks();
    	loadSettings();//reload default settings
    	firstRunComplete();
        fprintf(stderr,"Left first use tasks.\n");
    	navigator_extend_timeout(30000,NULL);//we now have 30 seconds to display a window
    }
    setTexBin("");
    return EXIT_SUCCESS;
}

void term(){
    saveSettings();
    guiTerminate();//kill the gui
    destroyLocalStrings();//destroy local strings
}

void render() {
    guiRender();
}

void handleScreenEvent(bps_event_t *event)
{
    screen_event_t screen_event = screen_event_get_event(event);
    mtouch_event_t mtouch_event;

    int screen_val, rc;
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TYPE, &screen_val);

    switch (screen_val) {
    case SCREEN_EVENT_KEYBOARD:
        screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_KEY_FLAGS, &screen_val);
        if (screen_val & KEY_DOWN) {
                screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_KEY_SYM,&screen_val);
                guiHandleKeyboard(screen_val);
        }
        break;
    case SCREEN_EVENT_MTOUCH_TOUCH:
    	rc = screen_get_mtouch_event(screen_event, &mtouch_event, 0);
//    	fprintf(stderr,"Received touch\n");
    	if(!rc){
    		guiHandleTouch(mtouch_event);
    	}
    	break;
    case SCREEN_EVENT_MTOUCH_MOVE:
    	rc = screen_get_mtouch_event(screen_event, &mtouch_event, 0);
		if (rc)
		{
			fprintf(stderr, "Error: failed to get mtouch event\n");
		}else{
            if(last_touch[0] && last_touch[1]) {
    			guiHandlePan(mtouch_event,last_touch);
//    			fprintf(stderr,"Pan %d %d -> cursor: %i\n\n",mtouch_event.x,mtouch_event.y,curSelCursor);
            }
            last_touch[0] = mtouch_event.x;
            last_touch[1] = mtouch_event.y;
		}
    	break;
    case SCREEN_EVENT_MTOUCH_RELEASE:
        last_touch[0] = 0;
        last_touch[1] = 0;
        break;
    }
}

void handleNavigatorEvent(bps_event_t *event){
	switch( bps_event_get_code(event) ) {
	case NAVIGATOR_EXIT:
		exit_application = 1;
		break;
	case NAVIGATOR_SWIPE_DOWN:
		guiHandleSwipeDown();
		break;
	case NAVIGATOR_WINDOW_INACTIVE:
	{
		fprintf(stderr,"GUI deactivated\n");
		bps_event_t *activation_event = NULL;
		//Wait for NAVIGATOR_WINDOW_ACTIVE event
		for (;;) {
			int rc = bps_get_event(&activation_event, -1);
			if(rc){
				fprintf(stderr,"Could not receive activation event.\n");
				exit_application = 1;
				break;
			}else{
				if (bps_event_get_code(activation_event)
						== NAVIGATOR_WINDOW_ACTIVE) {
					fprintf(stderr,"GUI activated\n");
					break;
				}
			}
		}
	}
	break;
	}
}

int main(int argc, char **argv) {
    int rc;
    screen_create_context(&screen_cxt, 0);
    bps_initialize();//Initialize BPS library

    if (BPS_SUCCESS != navigator_request_events(0)) {//...and navigator events...
        fprintf(stderr, "navigator_request_events failed\n");
        screen_destroy_context(screen_cxt);
        return 0;
    }

    if (BPS_SUCCESS != navigator_rotation_lock(true)) {//lock orientation
        fprintf(stderr, "navigator_rotation_lock failed\n");
        screen_destroy_context(screen_cxt);
        return 0;
    }

    if (BPS_SUCCESS != dialog_request_events(0)) {//dialog events...
        fprintf(stderr, "dialog_request_events failed\n");
        screen_destroy_context(screen_cxt);
        return 0;
    }

    if (BPS_SUCCESS != screen_request_events(screen_cxt)) {//tell bps that events will be needed
        fprintf(stderr, "screen_request_events failed\n");
        screen_destroy_context(screen_cxt);
        return 0;
    }

    if (EXIT_SUCCESS != init()) {//Initialize app data
        fprintf(stderr, "Unable to initialize app logic\n");
        screen_destroy_context(screen_cxt);
        return 0;
    }

    if (EXIT_SUCCESS != guiInit()) {//Initialize gui
        fprintf(stderr, "Unable to initialize gui\n");
        screen_destroy_context(screen_cxt);
        return 0;
    }

    render();
    while (!exit_application) {
        bps_event_t *event = NULL;
        for(;;) {
        	rc = bps_get_event(&event, 0);
        	if (event) {
        		int domain = bps_event_get_domain(event);

        		if (domain == screen_get_domain()) {
        			handleScreenEvent(event);
        		} else if ( domain == navigator_get_domain() ){
        			handleNavigatorEvent(event);
        		}
                render();//only render when events have occured
        	}else{
        		break;
        	}
        }
    }
    screen_stop_events(screen_cxt);
//    dialog_stop_events(screen_cxt);
//    navigator_stop_events(screen_cxt);
    bps_shutdown();
    term();
    screen_destroy_context(screen_cxt);
    return 0;
}
