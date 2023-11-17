//this file is part of notepad++
//Copyright (C)2022 Don HO <don.h@free.fr>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include <stdlib.h>

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE /*hModule*/)
{
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    setCommand(0, TEXT("json format"), json_format, NULL, false);
    //setCommand(1, TEXT("Hello (with dialog)"), helloDlg, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

extern "C" void do_format_json(char *data,int data_len,char *out,int out_len);
extern "C" void remove_empty_lines(char *in,const int in_len,char *out,const int out_len);

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void json_format()
{
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;
    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

	char *data=0;
	LONG_PTR data_len=0;
	char *tmp=0;
	LONG_PTR tmp_len=0;

	data_len=::SendMessage(curScintilla,SCI_GETTEXT,0,0);
	data=(char*)calloc(data_len + 4,1);
	tmp_len=(data_len * 10);
	tmp=(char*)calloc(tmp_len,1);
	if(!(data && tmp)){
		::MessageBox(NULL,TEXT("ERROR failed to allocated buffer"),TEXT("Plugin error"),MB_OK);
		goto ERROR_EXIT;
	}
	int res=::SendMessage(curScintilla,SCI_GETTEXT,(WPARAM)data_len+1,(LPARAM)data);
	if(res <= 0){
		goto ERROR_EXIT;
	}
	do_format_json(data,data_len,tmp,tmp_len);
	if(tmp){
		char *tmp2;
		int tmp2_len=tmp_len;
		tmp2=(char*)calloc(tmp2_len,1);
		if(tmp2){
			remove_empty_lines(tmp,tmp_len,tmp2,tmp2_len);
			free(tmp);
			tmp=tmp2;
			tmp2=0;
		}
	}
	SendMessage(curScintilla,SCI_SETTEXT,0,(LPARAM)tmp);

ERROR_EXIT:
	free(tmp);
	free(data);

}

void helloDlg()
{
    ::MessageBox(NULL, TEXT("Hello, Notepad++!"), TEXT("Notepad++ Plugin Template"), MB_OK);
}
