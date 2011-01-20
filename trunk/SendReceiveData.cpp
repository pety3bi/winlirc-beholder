/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.8.6.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 2011 Artem Golubev
 */

#include <windows.h>
#include "SendReceiveData.h"
#include "Globals.h"
#include <stdio.h>
#include <tchar.h>

DWORD WINAPI BeholdRC(void *recieveClass) {

   ((SendReceiveData*)recieveClass)->threadProc();
   return 0;
}

SendReceiveData::SendReceiveData() {

   threadHandle	= NULL;
   beginEvent     = NULL;
   exitEvent		= NULL;
}


BOOL SendReceiveData::init() {
   irCode			= 0;
   irLastCode		= 0;
   repeats			= 0;

   if( BTV_SelectCard( 0 ) ) {
      threadHandle = CreateThread(NULL,0,BeholdRC,(void *)this,0,NULL);
      if(threadHandle) {
         return true;
      }
   }

   return false;
}

void SendReceiveData::deinit() {

   killThread();
}


void SendReceiveData::threadProc() {

	HANDLE     events[2];
	DWORD      result = 0;

   beginEvent = CreateEvent(NULL,FALSE,TRUE,NULL);
	exitEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	events[0] = beginEvent;
	events[1] = exitEvent;

   //SetEvent(beginEvent);

	while(1) {
		result = WaitForMultipleObjects(2,events,FALSE,INFINITE);

		if(result==(WAIT_OBJECT_0)) 
		{
         ULONG code = 0;
         code = BTV_GetRCCodeEx();
         
		   irCode = code;
         
         if( irCode == irLastCode )
            if( repeats >= 9)
               repeats++;
            else
               repeats = 0;
         else
            repeats = 0;

         if( code ) {
            //printf( "%x" , code )
            SetEvent(dataReadyEvent);
         }

         Sleep( 250 );

         SetEvent(beginEvent);
		}

		if(result==(WAIT_OBJECT_0+1))
		{
			//printf("leaving thread \n");
			break;
		}
	}

	if(exitEvent) {
		CloseHandle(exitEvent);
		exitEvent = NULL;
	}

	if(beginEvent) {
		CloseHandle(beginEvent);
		beginEvent = NULL;
	}

}

void SendReceiveData::killThread() {

	//
	// need to kill thread here
	//
	if(exitEvent) {
		SetEvent(exitEvent);
	}

	if(threadHandle!=NULL) {

		DWORD result = 0;

		if(GetExitCodeThread(threadHandle,&result)==0) 
		{
			CloseHandle(threadHandle);
			threadHandle = NULL;
			return;
		}

		if(result==STILL_ACTIVE)
		{
         // walkaround for deadlock
			WaitForSingleObject(threadHandle,5000);
         TerminateThread(threadHandle, 0); 

         //WaitForSingleObject(threadHandle,INFINITE);
			CloseHandle(threadHandle);
			threadHandle = NULL;
		}
	}
}

void SendReceiveData::waitTillDataIsReady(int maxUSecs) {

	HANDLE events[2]={dataReadyEvent,threadExitEvent};
	int evt;
	if(threadExitEvent==NULL) evt=1;
	else evt=2;

	if(irCode==0)
	{
		ResetEvent(dataReadyEvent);
		int res;
		if(maxUSecs)
			res=WaitForMultipleObjects(evt,events,FALSE,(maxUSecs+500)/1000);
		else
			res=WaitForMultipleObjects(evt,events,FALSE,INFINITE);
		if(res==(WAIT_OBJECT_0+1))
		{
			ExitThread(0);
			return;
		}
	}

}

int SendReceiveData::decodeCommand(char *out) {

	//==================
	char buttonName[32];
   char remoteName[32];
	//==================

   memset(&buttonName,0,sizeof(char)*32);
   memset(&remoteName,0,sizeof(char)*32);

	switch(irCode) {
      case 0:
         irCode = 0;
         return 0;

      // BEGIN AverMedia 407
      case 0x2FD00FF:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"POWER");
         break;

      case 0x2FD01FE:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"TV/FM");
         break;
      case 0x2FD02FD:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"CD");
         break;
      case 0x2FD03FC:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"TELETEXT");
         break;

      case 50136570:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"1");
         break;
      case 50136825:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"2");
         break;
      case 50137080:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"3");
         break;
      case 50137590:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"4");
         break;
      case 50137845:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"5");
         break;
      case 50138100:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"6");
         break;
      case 50138610:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"7");
         break;
      case 50138865:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"8");
         break;
      case 50139120:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"9");
         break;
      case 50139630:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"0");
         break;

      case 50136315:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"VIDEO");
         break;
      case 50137335:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"AUDIO");
         break;
      case 50138355:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"SCREEN");
         break;

      case 50139885:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"DISPLAY");
         break;
      case 50140140:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"LOOP");
         break;
      case 50139375:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"PREVIEW");
         break;
      case 50140650:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"AUTOSCAN");
         break;
      case 50140905:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"FREEZE");
         break;
      case 50141160:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"CAPTURE");
         break;
      case 50140395:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"MUTE");
         break;
      case 50141670:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"RECORD");
         break;
      case 50141925:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"PAUSE");
         break;
      case 50142180:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"STOP");
         break;
      case 50141415:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"PLAY");
         break;

      case 0x2FD1EE1:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"VOL_DOWN");
         break;

      case 0x2FD1FE0:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"VOL_UP");
         break;

      case 0x3FC02FD:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"CHANNEL_DOWN");
         break;
      case 0x3FC03FC:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"CHANNEL_UP");
         break;

      case 50142690:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"RED");
         break;
      case 50142435:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"YELLOW");
         break;
      case 66847230:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"GREEN");
         break;
      case 66846975:
         strcpy_s(remoteName,_countof(remoteName),"AverMedia");
         strcpy_s(buttonName,_countof(buttonName),"BLUE");
         break;

      // END AverMedia 407
	}

   if( buttonName[0] == 0 )
      _snprintf_s(buttonName,_countof(buttonName),"0x%X", irCode);

   if( remoteName[0] == 0 )
      strcpy_s(remoteName,_countof(remoteName),"Unknown");
   
   //_snprintf_s(out, PACKET_SIZE+1, PACKET_SIZE+1, "%016llx %02x %s %s\n", irCode, repeats, "", buttonName);
   _snprintf_s(out, PACKET_SIZE+1, PACKET_SIZE+1, "%016llx %02x %s %s\n", __int64(0), repeats, buttonName, remoteName);
	
   irLastCode	= irCode;
   irCode = 0;

	return 1;
}
