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

#include "Globals.h"
#include "SendReceiveData.h"

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


   if( !BTV_GetIStatus() ) {
      MessageBox( 0, _T( "Library not found" ), _T( "Beholder" ), MB_OK | MB_ICONERROR );
      return false;
   }

   /*
   int status = BTV_GetIStatus();
   switch( status ) {
      case 0: // Library not found.
         MessageBox( 0, _T( "Library not found" ), _T( "Beholder" ), MB_OK | MB_ICONERROR );
         break;
      case 1:	// WDM device not selected.
         MessageBox( 0, _T( "WDM device not selected" ), _T( "Beholder" ), MB_OK | MB_ICONERROR );
         break;
      case 2:	// OK.
         MessageBox( 0, _T( "OK" ), _T( "Beholder" ), MB_OK | MB_ICONERROR );
         break;

      default:
         MessageBox( 0, _T( "default" ), _T( "Beholder" ), MB_OK | MB_ICONERROR );
         break;
   }
   */

   if( BTV_SelectCard() ) {
      threadHandle = CreateThread(NULL,0,BeholdRC,(void *)this,0,NULL);
      if(threadHandle) {
         return true;
      }
   }

   MessageBox( 0, _T( "Error selecting card" ), _T( "Beholder" ), MB_OK | MB_ICONERROR );
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
		   irCode = BTV_GetRCCodeEx();
         if( irCode == irLastCode )
            if( repeats >= 9)
               repeats++;
            else
               repeats = 0;
         else
            repeats = 0;

         if( irCode ) {
            SetEvent(dataReadyEvent);
         }

         Sleep( 250 );

         SetEvent(beginEvent);
		}

		if(result==(WAIT_OBJECT_0+1))
		{
			// leaving thread
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

int SendReceiveData::decodeCommand(TCHAR *out) {

	//==================
	TCHAR buttonName[32];
   TCHAR remoteName[32];
	//==================

   memset( &buttonName, 0, sizeof(TCHAR)*32 );
   memset( &remoteName, 0, sizeof(TCHAR)*32 );

	switch( irCode ) {
      case 0:
         irCode = 0;
         return 0;

      // BEGIN AverMedia 407
      case 0x2FD00FF:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "POWER" ) );
         break;

      case 0x2FD01FE:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "TV/FM" ) );
         break;
      case 0x2FD02FD:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "CD" ) );
         break;
      case 0x2FD03FC:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "TELETEXT" ) );
         break;

      case 50136570:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "1 ") );
         break;
      case 50136825:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "2" ) );
         break;
      case 50137080:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "3" ) );
         break;
      case 50137590:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "4" ) );
         break;
      case 50137845:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "5" ) );
         break;
      case 50138100:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "6" ) );
         break;
      case 50138610:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "7" ) );
         break;
      case 50138865:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "8" ) );
         break;
      case 50139120:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "9" ) );
         break;
      case 50139630:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "0" ) );
         break;

      case 50136315:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "VIDEO" ) );
         break;
      case 50137335:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "AUDIO" ) );
         break;
      case 50138355:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "SCREEN" ) );
         break;

      case 50139885:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "DISPLAY" ) );
         break;
      case 50140140:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "LOOP" ) );
         break;
      case 50139375:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "PREVIEW" ) );
         break;
      case 50140650:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "AUTOSCAN" ) );
         break;
      case 50140905:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "FREEZE" ) );
         break;
      case 50141160:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "CAPTURE" ) );
         break;
      case 50140395:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "MUTE" ) );
         break;
      case 50141670:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "RECORD" ) );
         break;
      case 50141925:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "PAUSE" ) );
         break;
      case 50142180:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "STOP" ) );
         break;
      case 50141415:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "PLAY" ) );
         break;

      case 0x2FD1EE1:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "VOL_DOWN" ) );
         break;

      case 0x2FD1FE0:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "VOL_UP" ) );
         break;

      case 0x3FC02FD:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "CHANNEL_DOWN" ) );
         break;
      case 0x3FC03FC:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "CHANNEL_UP" ) );
         break;

      case 50142690:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "RED" ) );
         break;
      case 50142435:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "YELLOW" ) );
         break;
      case 66847230:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "GREEN" ) );
         break;
      case 66846975:
         _tcscpy_s( remoteName, _countof(remoteName), _T( "AverMedia" ) );
         _tcscpy_s( buttonName, _countof(buttonName), _T( "BLUE" ) );
         break;

      // END AverMedia 407
	}

   if( buttonName[0] == 0 )
      _sntprintf_s( buttonName, _countof(buttonName), _T( "0x%X" ), irCode );

   if( remoteName[0] == 0 )
      _tcscpy_s( remoteName, _countof(remoteName), _T( "Unknown" ) );
   
   _sntprintf_s( out, PACKET_SIZE+1, PACKET_SIZE+1, _T( "%016llx %02x %s %s\n" ), __int64(0), repeats, buttonName, remoteName );
	
   irLastCode	= irCode;
   irCode = 0;

	return 1;
}
