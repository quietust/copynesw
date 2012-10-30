#ifndef COPYNESW_H
#define COPYNESW_H

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <commctrl.h>
#include <shlobj.h>
#include <time.h>
#include "resource.h"

#include <string>
#include <vector>
using namespace std;

extern	HINSTANCE hInst;
extern	HWND	topHWnd;

struct Plugin
{
	int num;
	string name;
	string desc;
	BYTE header[128];
	BYTE data[1024];
	string romstring;
};

enum plugin_type { PLUG_STD = 0, PLUG_WRAM = 1, PLUG_UTIL = 2, PLUG_UPLOAD = -1 };

struct Category
{
	vector<Plugin *> list;
	plugin_type type;
	string desc;
};
extern	vector<Category *> Plugins;

/* miscdialogs */
extern	HWND	DlgStatus;

void	CloseStatus (void);
void	OpenStatus (HWND hWnd);
void	__cdecl	StatusText (char *text, ...);
void	StatusPercent (int percent);
void	StatusButton (void);
void	StatusButtonAsync (BOOL);
BOOL	StatusButtonPressed (void);
void	StatusOK (void);

extern	char	*PromptTitle;
extern	char	PromptResult[1024];
BOOL	Prompt (HWND hWnd);
BOOL	PromptLong (HWND hWnd);
BOOL	PromptFile (HWND hWnd, const char *Filter, char *FilePath, char *FileName, const char *InitDir, const char *Title, const char *DefExt, BOOL Save);
Plugin *PromptPlugin (plugin_type Type);

/* crc32 */
void	InitCRC	(void);
UINT32	GetCRC	(FILE *File);

/* config */
extern	int	HWVer;
extern	int	ParPort, ParAddr, ParECP;
extern	BOOL	SaveCRC, SaveFiles, MakeUNIF;
extern	char	Path_MAIN[MAX_PATH], Path_PRG[MAX_PATH], Path_CHR[MAX_PATH], Path_WRAM[MAX_PATH],
	Path_NES[MAX_PATH], Path_CRC[MAX_PATH], Path_NSF[MAX_PATH], Path_PLUG[MAX_PATH];
char *	addSlash (char *path);
void	WriteConfig (void);
Plugin *makePlugin (const char *name, const char *filename, int num, const char *desc);

/* io */
#define	RESET_COPYMODE	0
#define	RESET_PLAYMODE	1
#define	RESET_ALTPORT	2
#define	RESET_NORESET	4
extern	char	ROMstring[256];
BOOL	OpenPort	(int port, int addr, int ecp);
void	ClosePort	(void);
void	InitPort	(void);
void	ResetNES	(int);
BOOL	ReadByte	(BYTE &);
BOOL	WriteByte	(BYTE);
BOOL	WriteBlock	(const BYTE *,int);
BOOL	ReadByteSilent	(BYTE &);
BOOL	WriteByteSilent	(BYTE);
BOOL	ReadByteEx	(BYTE &,int,BOOL);
BOOL	WriteByteEx	(BYTE,int,BOOL);
BOOL	ReadByteReady	(void);
BOOL	WriteByteAsync	(BYTE);
BOOL	WriteCommand	(BYTE,BYTE,BYTE,BYTE,BYTE);
BOOL	LoadPlugin	(const Plugin *);
BOOL	LoadPlugin	(const char *filename);
BOOL	RunCode		(void);

/* misc */
#define	SLEEP_SHORT	100
#define	SLEEP_LONG	1000
void	WriteUNIF	(const char *basename,const char *board,int batt,int mirr,int fourscrn,int mcon);
void	WriteNES	(const char *basename,int mapper,int battery,int mirror,int fourscrn);

#define	MSGBOX_TITLE	"CopyNESW - " CMD_NAME

#endif /* COPYNESW_H */