#ifndef COPYNESW_H
#define COPYNESW_H

extern	HINSTANCE hInst;
extern	HWND	topHWnd;

/* Plugins */
typedef	struct	Plugin
{
	char *name;
	char *file;
	int num;
	char *desc;
}	TPlugin, *PPlugin;

typedef	struct	Category
{
	PPlugin *list;
	int type;
	char *desc;
}	TCategory, *PCategory;
extern	PCategory *Plugins;

/* Misc Dialogs */
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
BOOL	PromptFile (HWND hWnd, char *Filter, char *FilePath, char *FileName, char *InitDir, char *Title, char *DefExt, BOOL Save);
PPlugin	PromptPlugin (int Type);
#define	PLUG_STD	0
#define	PLUG_WRAM	1
#define	PLUG_UTIL	2
#define	PLUG_UPLOAD	-1

/* CRC32 */
void	InitCRC	(void);
UINT32	GetCRC	(FILE *File);

/* Config */
extern	int	HWVer;
extern	int	ParPort, ParAddr, ParECP;
extern	BOOL	SaveCRC, SaveFiles, MakeUNIF;
extern	char	Path_MAIN[MAX_PATH], Path_PRG[MAX_PATH], Path_CHR[MAX_PATH], Path_WRAM[MAX_PATH],
	Path_NES[MAX_PATH], Path_CRC[MAX_PATH], Path_NSF[MAX_PATH], Path_PLUG[MAX_PATH];
char *	addSlash (char *path);
void	WriteConfig (void);

/* I/O Driver */
BOOL	OpenPort (int port, int addr, int ecp);
void	ClosePort (void);
unsigned char	prData (void);
void	pwData (unsigned char);
unsigned char	prStatus (void);
void	pwControl (unsigned char);

/* I/O routines */
#define	RESET_COPYMODE	0
#define	RESET_PLAYMODE	1
#define	RESET_ALTPORT	2
#define	RESET_NORESET	4
extern	char	ROMstring[256];
void	InitPort	(void);
void	ResetNES	(int);
BOOL	ReadByte	(BYTE *);
BOOL	WriteByte	(BYTE);
BOOL	WriteBlock	(BYTE *,int);
BOOL	ReadByteSilent	(BYTE *);
BOOL	WriteByteSilent	(BYTE);
BOOL	ReadByteEx	(BYTE *,int,BOOL);
BOOL	WriteByteEx	(BYTE,int,BOOL);
BOOL	ReadByteReady	(void);
BOOL	WriteByteAsync	(BYTE);
BOOL	WriteCommand	(BYTE,BYTE,BYTE,BYTE,BYTE);
BOOL	LoadPlugin	(char *);
BOOL	RunCode		(void);

/* Miscellaneous functions */
#define	SLEEP_SHORT	100
#define	SLEEP_LONG	1000
char	*strjoin3	(char *out, const char *in1, const char *in2, const char *in3);
void	WriteUNIF	(char *,char *,int,int,int,int);
void	WriteNES	(char *,int,int,int,int);

#define	MSGBOX_TITLE	"CopyNESW - " CMD_NAME

#endif /* COPYNESW_H */