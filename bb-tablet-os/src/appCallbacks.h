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

#ifndef APPCALLBACKS_H_
#define APPCALLBACKS_H_

#include <string>

#include "qnxExtensions.h"
#include "taskUtils.h"

#include "activeGui.h"
#include GUI_HEADER_FILE

//import app text
extern std::string execCommand;
extern std::string execDir;
extern std::string execFile;
extern std::string execString;

//import process settings
extern bool autoDisplayPDF;
extern bool autoBuildCommand;
extern bool autoDisplayResult;
extern std::string TEX_BIN;

//public functions
void saveSettings();
void loadSettings();
void restoreDefaultSettings();

//button callbacks
void bcbChooseFile();
void bcbShowLog();
void bcbExecute();
void bcbToggleApdf();
void bcbToggleAbc();
void bcbToggleAdr();

#endif /* APPCALLBACKS_H_ */
