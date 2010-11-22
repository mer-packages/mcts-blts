/* main.cpp -- Implementation of command line interface.

   Copyright (C) 2000-2010, Nokia Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

//QTM_USE_NAMESPACE

#include "stable.h"
#include "FeedbackTest.h"
//#include <iostream>

QTM_USE_NAMESPACE

int main( int argc, char** argv ) {

	qDebug() << "In main function";


        FeedbackTest test;

        test.SetCaseName("MT1");

	test.Initialize();

        test.StartEffect();
	test.Uninitialize();

	return 0;
}
