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

#ifndef DIALOGTOOLS_H_
#define DIALOGTOOLS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <bps/navigator.h>
#include <bps/bps.h>
#include <bps/dialog.h>

#ifdef __cplusplus
extern "C" {
#endif

int showDialog();
int closeDialog();
int getDialogResponse(bps_event_t *diagevent);
int waitForDialogResponse();
int buildModalAlert(const char* message,int dlgSize, int nButtons, const char** buttonNames);
int modalAlertOpt(const char * message,int dlgSize,int nButtons,const char** buttonNames);
int modalAlert(const char * message);
int infoBoxAlert(const char * message);
int buildModalPopupList(const char* vals[],int numvals,const char* message,bool multisel);
int * modalPopupList(const char* vals[],int * numvals,const char* message,bool multisel);
int buildModalFilesel();
int modalFilesel();

#ifdef __cplusplus
}
#endif

#endif /* DIALOGTOOLS_H_ */
