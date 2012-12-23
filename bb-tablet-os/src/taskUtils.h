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

#ifndef TASKUTILS_H_
#define TASKUTILS_H_

#include "languageTools.h"
#include "qnxExtensions.h"
#include <time.h>

#include "activeGUI.h"
#include GUI_HEADER_FILE

enum settingsTypes {STRING_SETTING, BOOL_SETTING};

int checkForFirstRun();
int firstRunTasks();
int firstRunComplete();
int saveLog(std::string logdata);
int showLog();
int saveSettingsToFile(void * settingsAdds [], int settingsTypes [], int numSettings);
int loadSettingsFromFile(void * settingsAdds [], int settingsTypes [], int numSettings);
void setTexBin(std::string texDist);
void installDistribution(std::string distToInstall);
void chooseDistribution();
void chooseLanguage();
std::string getVersion();

#endif /* TASKUTILS_H_ */
