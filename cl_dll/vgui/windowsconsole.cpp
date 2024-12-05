//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#define PROTECTED_THINGS_H
#include "cbase.h"
#include <windows.h>
#undef PROTECTED_THINGS_H
#include "protected_things.h"
#include <vgui/vgui.h>
#include <vgui/keycode.h>
#include <vgui_controls/frame.h>
#include <vgui_controls/richtext.h>
#include <vgui_controls/textentry.h>
#include "gameuipanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar ins_consolecmd("ins_consolecmd", "cmd", FCVAR_ARCHIVE|FCVAR_CLIENTDLL, "Which command interpreter to use for the windows console.\nSet the working directory with ins_consolepath.\n\nType /reset in the console after changing this value to restart the console.");
ConVar ins_consolepath("ins_consolepath", "", FCVAR_ARCHIVE|FCVAR_CLIENTDLL, "Working directory for the command interpreter.\nUses SourceMods\\Insurgency when empty.");

//////////////////////////////////////////////////////////////////////////
// WindowsConsole declaration
//////////////////////////////////////////////////////////////////////////
class WindowsConsole : public Frame
{
	DECLARE_CLASS_SIMPLE(WindowsConsole, Frame);
public:
	WindowsConsole();
	virtual ~WindowsConsole();

	virtual void		OnThink();
	virtual void		OnKeyCodeTyped(KeyCode code);
	virtual void		ApplySchemeSettings(IScheme *pScheme);
private:
	char		m_szCommandHistory[50][200];
	int			m_iCurrentHistory;
	int			m_iMaxHistory;
	RichText	*m_pConsoleBuffer;
	TextEntry	*m_pCommandEntry;

	PROCESS_INFORMATION m_piCmd;

	HANDLE		m_hInReadPipe, m_hInWritePipe;
	HANDLE		m_hOutReadPipe, m_hOutWritePipe;
	bool		m_bGotPipes, m_bWarned;

	void PrintWindowsError();

	void ProcessCheck();
	void ResetProcess();

	void InitCmd();
	void CloseCmd();
};

WindowsConsole::WindowsConsole() : Frame( NULL, "WindowsConsole" )
{
	Q_memset(&m_szCommandHistory,0,sizeof(m_szCommandHistory));
	m_iMaxHistory=m_iCurrentHistory=-1;

	SetParent(g_pGameUIPanel->GetVParent());
	SetScheme("SourceScheme");
	SetSizeable(true);
	m_pConsoleBuffer=new RichText(this,"ConsoleBuffer");
	m_pCommandEntry=new TextEntry(this,"CmdEntry");
	LoadControlSettings("resource/WindowsConsole.res");

	m_bWarned=m_bGotPipes=false;
	InitCmd();
}

WindowsConsole::~WindowsConsole()
{
	CloseCmd();
	delete m_pConsoleBuffer;
	delete m_pCommandEntry;
}

void WindowsConsole::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_pConsoleBuffer->SetFont(pScheme->GetFont("ConsoleText",true));
	if (!m_bWarned)
	{
		m_bWarned=true;
		m_pConsoleBuffer->InsertColorChange(Color(255,0,0,255));
		m_pConsoleBuffer->InsertString("If you don't know what you're doing, you can really wreck your system here. Type /hide <ENTER> to quit now.\n");
		m_pConsoleBuffer->InsertColorChange(m_pConsoleBuffer->GetFgColor());
		m_pConsoleBuffer->InsertString("Type /help for a list of available commands\n\n");
	}
}

void WindowsConsole::PrintWindowsError()
{
	HLOCAL hLocal = NULL;

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0,(LPTSTR)&hLocal, 0, NULL);

	m_pConsoleBuffer->InsertColorChange(Color(255,0,0,255));
	m_pConsoleBuffer->InsertString("\nWindows error returned:");
	m_pConsoleBuffer->InsertString((LPTSTR)hLocal);
	m_pConsoleBuffer->InsertString("\n");
	m_pConsoleBuffer->InsertColorChange(m_pConsoleBuffer->GetFgColor());

	LocalFree(hLocal);
}

void WindowsConsole::InitCmd()
{
	SECURITY_ATTRIBUTES saSecAttr={0};
	saSecAttr.nLength=sizeof(saSecAttr);
	saSecAttr.bInheritHandle=TRUE;
	STARTUPINFO suiCmd={0};
	if (CreatePipe(&m_hInReadPipe,&m_hInWritePipe,&saSecAttr,0)&&CreatePipe(&m_hOutReadPipe,&m_hOutWritePipe,&saSecAttr,0))
	{
		suiCmd.cb=sizeof(STARTUPINFO);
		suiCmd.hStdInput=m_hInReadPipe;
		suiCmd.hStdOutput=m_hOutWritePipe;
		suiCmd.hStdError=m_hOutWritePipe;
		suiCmd.dwFlags=STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
		suiCmd.wShowWindow=SW_HIDE;
		Q_memset(&m_piCmd,0,sizeof(PROCESS_INFORMATION));
		const char* pszDir=ins_consolepath.GetString();
		if (!strlen(pszDir))
			pszDir=engine->GetGameDirectory();
		if (CreateProcess(NULL, (char*)ins_consolecmd.GetString(), &saSecAttr, &saSecAttr, TRUE, NORMAL_PRIORITY_CLASS, NULL, pszDir, &suiCmd, &m_piCmd))
		{
			m_bGotPipes=true;
			DWORD dwMode=PIPE_READMODE_BYTE|PIPE_NOWAIT;
			SetNamedPipeHandleState(m_hOutReadPipe,&dwMode,NULL,NULL);
		}
		else
		{
			PrintWindowsError();
			CloseHandle(m_piCmd.hProcess);
			CloseHandle(m_piCmd.hThread);
		}
	}
	else
	{
		PrintWindowsError();
	}
}

void WindowsConsole::CloseCmd()
{
	if (m_bGotPipes)
	{
		m_bGotPipes=false;
		PostThreadMessage(m_piCmd.dwThreadId,WM_SYSCOMMAND,SC_CLOSE,-1);
		PostThreadMessage(m_piCmd.dwThreadId,WM_CLOSE,0,0);
		PostThreadMessage(m_piCmd.dwThreadId,WM_QUIT,0,0);
		if (g_pVCR->Hook_WaitForSingleObject(m_piCmd.hProcess,200)==WAIT_TIMEOUT)
		{
			DWORD dwCode;
			GetExitCodeProcess(m_piCmd.hProcess,&dwCode);
			if (dwCode==STILL_ACTIVE)
				TerminateProcess(m_piCmd.hProcess,0);
		}
		CloseHandle(m_piCmd.hProcess);
		CloseHandle(m_piCmd.hThread);
		CloseHandle(m_hInReadPipe);
		CloseHandle(m_hInWritePipe);
		CloseHandle(m_hOutReadPipe);
		CloseHandle(m_hOutWritePipe);
	}
}

void WindowsConsole::ProcessCheck()
{
	if (m_bGotPipes)
	{
		DWORD dwCode=STILL_ACTIVE;
		GetExitCodeProcess(m_piCmd.hProcess,&dwCode);
		if (dwCode!=STILL_ACTIVE)
		{
			ResetProcess();
		}
	}
}

void WindowsConsole::ResetProcess()
{
	CloseCmd();
	m_pConsoleBuffer->InsertColorChange(Color(255,0,0,255));
	m_pConsoleBuffer->InsertString("\nProcess halted, restarting...\n");
	m_pConsoleBuffer->InsertColorChange(m_pConsoleBuffer->GetFgColor());
	InitCmd();
}


void WindowsConsole::OnThink()
{
	BaseClass::OnThink();

	if (!m_bGotPipes)
		return;

	ProcessCheck();

	static char cBuf[8193];

	float flNow=engine->Time();
	DWORD dwRead=0;
	do
	{
		ReadFile(m_hOutReadPipe,cBuf,8192,&dwRead,NULL);
		cBuf[dwRead]=0;
		m_pConsoleBuffer->InsertString(cBuf);
	} while (dwRead&&engine->Time()-flNow<0.05); //don't think longer than 50ms
}

void WindowsConsole::OnKeyCodeTyped(KeyCode code)
{
	if (m_pCommandEntry->HasFocus())
	{
		if (code==KEY_ENTER)
		{
			int iLen=m_pCommandEntry->GetTextLength();
			char* pszTmp=new char[iLen+1];
			m_pCommandEntry->GetText(pszTmp,iLen+1);
			Q_memmove(m_szCommandHistory[1],m_szCommandHistory[0],sizeof(m_szCommandHistory[0])*49);
			if (m_iMaxHistory<49)
				m_iMaxHistory++;
			Q_strncpy(m_szCommandHistory[0],pszTmp,200);
			m_iCurrentHistory=-1;

			if (Q_strnicmp(pszTmp,"/reset",6)==0)
			{
				ResetProcess();
			}
			else if (m_bGotPipes)
			{
				ProcessCheck();
				
				DWORD dwWritten;
				if (*pszTmp=='/')
				{
					if (Q_strnicmp(pszTmp,"/break",6)==0)
					{
						WriteFile(m_hInWritePipe,"\x03",1,&dwWritten,NULL);
					}
					else if (Q_strnicmp(pszTmp,"/hup",4)==0)
					{
						WriteFile(m_hInWritePipe,"\x04",1,&dwWritten,NULL);
					}
					else if (Q_strnicmp(pszTmp,"/hide",5)==0)
					{
						SetVisible(false);
					}
					else if (Q_strnicmp(pszTmp,"/clear",6)==0)
					{
						m_iMaxHistory=-1;
						m_pConsoleBuffer->SetText("");
						m_pConsoleBuffer->InsertColorChange(Color(0,128,192,255));
						m_pConsoleBuffer->InsertString("History cleared\n");
						m_pConsoleBuffer->InsertColorChange(m_pConsoleBuffer->GetFgColor());
						WriteFile(m_hInWritePipe,"\n",1,&dwWritten,NULL);
					}
					else if (Q_strnicmp(pszTmp,"/cd",3)==0)
					{
						const char* pszDir=engine->GetGameDirectory();
						WriteFile(m_hInWritePipe,pszDir,2,&dwWritten,NULL);
						WriteFile(m_hInWritePipe,"\ncd \"",5,&dwWritten,NULL);
						WriteFile(m_hInWritePipe,pszDir,strlen(pszDir),&dwWritten,NULL);
						WriteFile(m_hInWritePipe,"\"\n",2,&dwWritten,NULL);
					}
					else if (Q_strnicmp(pszTmp,"/help",5)==0)
					{
						m_pConsoleBuffer->InsertColorChange(Color(0,128,192,255));
						m_pConsoleBuffer->InsertString("\nWindows console help\n\n");
						m_pConsoleBuffer->InsertColorChange(m_pConsoleBuffer->GetFgColor());
						m_pConsoleBuffer->InsertString("Available commands:\n");
						m_pConsoleBuffer->InsertString("  /break - Send 0x03\n");
						m_pConsoleBuffer->InsertString("  /hup   - Send 0x04\n");
						m_pConsoleBuffer->InsertString("  /hide  - Close this window\n");
						m_pConsoleBuffer->InsertString("  /clear - Clear both the history and the scrollback\n");
						m_pConsoleBuffer->InsertString("  /reset - Restart the command interpreter. This is set by the console variable ins_consolecmd.\n");
						m_pConsoleBuffer->InsertString("  /cd    - Change directory to SourceMods\\Insurgency\n");
						m_pConsoleBuffer->InsertString("  /help  - Display this help\n\n");
						WriteFile(m_hInWritePipe,"\n",1,&dwWritten,NULL);
					}
					else
					{
						m_pConsoleBuffer->InsertColorChange(Color(255,0,0,255));
						m_pConsoleBuffer->InsertString("\nNo such command: ");
						m_pConsoleBuffer->InsertString(pszTmp);
						m_pConsoleBuffer->InsertString("\n");
						m_pConsoleBuffer->InsertColorChange(m_pConsoleBuffer->GetFgColor());
						m_pConsoleBuffer->InsertString("Type /help for a list of available commands\n");
						WriteFile(m_hInWritePipe,"\n",1,&dwWritten,NULL);
					}
				}
				else
				{
					WriteFile(m_hInWritePipe,pszTmp,iLen,&dwWritten,NULL);
					WriteFile(m_hInWritePipe,"\n",1,&dwWritten,NULL);
				}
			}
			delete [] pszTmp;
			m_pCommandEntry->SetText("");
		}
		else if (code==KEY_UP)
		{
			if (m_iCurrentHistory<m_iMaxHistory)
			{
				if (m_iCurrentHistory>=0)
				{
					int iLen=m_pCommandEntry->GetTextLength();
					char* pszTmp=new char[iLen+1];
					m_pCommandEntry->GetText(pszTmp,iLen+1);
					Q_strncpy(m_szCommandHistory[m_iCurrentHistory],pszTmp,200);

					delete [] pszTmp;
				}
				m_iCurrentHistory++;
				m_pCommandEntry->SetText(m_szCommandHistory[m_iCurrentHistory]);
			}
		}
		else if (code==KEY_DOWN)
		{
			if (m_iCurrentHistory>=0)
			{
				int iLen=m_pCommandEntry->GetTextLength();
				char* pszTmp=new char[iLen+1];
				m_pCommandEntry->GetText(pszTmp,iLen+1);
				Q_strncpy(m_szCommandHistory[m_iCurrentHistory],pszTmp,200);

				delete [] pszTmp;

				m_iCurrentHistory--;
				if (m_iCurrentHistory>=0)
					m_pCommandEntry->SetText(m_szCommandHistory[m_iCurrentHistory]);
				else
					m_pCommandEntry->SetText("");
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Console Command
//////////////////////////////////////////////////////////////////////////
CON_COMMAND_F(vgui_windowsconsole,"Show windows console",FCVAR_CLIENTDLL)
{
	static WindowsConsole* pWindowsConsole=NULL;
	if (!pWindowsConsole)
		pWindowsConsole=new WindowsConsole();
	pWindowsConsole->Activate();
}