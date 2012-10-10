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

#ifndef LANGUAGETOOLS_H_
#define LANGUAGETOOLS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <bps/bps.h>
#include <bps/locale.h>

extern std::string curLocale;

int loadLocalStrings();
void destroyLocalStrings();
//static void matchLocale(std::string language, std::string country);
//static int readLangFile(std::string *langFileText);

std::string gStr(std::string strName);

#endif /* LANGUAGETOOLS_H_ */
