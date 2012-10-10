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

#ifndef GUIOPENGL_H_
#define GUIOPENGL_H_

#include <string>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <bps/screen.h>
#include <bps/event.h>
#include "input/screen_helpers.h"
#include <sys/keycodes.h>

#include "guiOpenGLbbutil.h"
#include "qnxExtensions.h"
#include "appCallbacks.h"
#include "languageTools.h"
#include "dialogTools.h"

//import GUI base properties
extern float width, height;
extern screen_context_t screen_cxt;

//Tie-ins to dialogTools (used so that a new gui can be written without requiring dialogTools)
void guiInfoBox(std::string istr);
void guiAlert(std::string istr);
std::string guiFileSelPopup(std::string istr);
void guiCloseDialog();
int  guiAlertQuery(std::string istr);

//Public functions (all possible GUIs must provide these)
int  guiInit();
void guiRender();
void guiTerminate();
void guiHandleTouch(mtouch_event_t mtouch_event);
void guiHandlePan(mtouch_event_t mtouch_event,int last_touch[]);
void guiHandleKeyboard(int screen_val);
void guiHandleSwipeDown();
void guiHandleAddDist();

#endif /* GUIOPENGL_H_ */
