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

#include "qnxExtensions.h"

std::string systemCallResponse(std::string sysCall){
	std::string response = "";

	FILE* fpipe;
	if( ( fpipe = popen( (sysCall+ " 2>&1").c_str(), "r" ) ) == NULL ) {
		perror( "popen failed!" );
		printf("failed popen");
		fflush(stdout);
	}else{
		char buffer[256];
		    while( fgets( buffer, 256, fpipe ) != NULL ) {
		    	response += buffer;
		}
		pclose( fpipe );
//		fprintf(stderr,"%s",response.c_str());
	}


	return response;
//return 0;
}
