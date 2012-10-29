#include "StdAfx.h"
#include <shlobj.h>

int	HWVer;
int	ParPort, ParAddr, ParECP;
BOOL	SaveCRC, SaveFiles;
char	Path_MAIN[MAX_PATH], Path_PRG[MAX_PATH], Path_CHR[MAX_PATH], Path_WRAM[MAX_PATH],
	Path_NES[MAX_PATH], Path_CRC[MAX_PATH], Path_NSF[MAX_PATH], Path_PLUG[MAX_PATH];

/* Note: base path must have a trailing backslash */
char *	_relpath (char *abspath, char *basepath)
{
	int i, j, k;
	char tmp[MAX_PATH];
	tmp[0] = 0;
	i = 0;
	
	while ((tolower(abspath[i]) == tolower(basepath[i])) && (basepath[i] != 0))
		i++;
	if (basepath[i])
	{
		k = 1;
		for (j = i; basepath[j] != 0; j++)
			if (basepath[j] == '\\')
				k++;
		for (j = 1; j < k; j++)
			strcat(tmp,"..\\");
	}
	strcat(tmp,&abspath[i]);
	if (strlen(tmp) == 0)
		strcpy(tmp,".");
	strcpy(abspath,tmp);
	return abspath;
}
char *	addSlash (char *path)
{
	int x = strlen(path);
	if (path[x-1] == '\\')
		return path;
	path[x] = '\\';
	path[x+1] = 0;
	return path;
}

void	GetProgPath (void)
{
	int i;
	GetModuleFileName(NULL,Path_MAIN,MAX_PATH);
	i = strlen(Path_MAIN);
	while (i > 0)
		if (Path_MAIN[--i] == '\\')
		{
			Path_MAIN[++i] = 0;
			return;
		}
}

void	GetConfig (void)
{
	char Config[MAX_PATH], tmpdir[MAX_PATH], tmpstr[16];
	strcpy(Config,Path_MAIN);
	strcat(Config,"CopyNESW.ini");
	SaveCRC = GetPrivateProfileInt("CopyNES","SaveCRC",0,Config);
	SaveFiles = GetPrivateProfileInt("CopyNES","SaveFiles",0,Config);
	GetPrivateProfileString("CopyNES","PRGPath","Parts",Path_PRG,MAX_PATH,Config);
	GetPrivateProfileString("CopyNES","CHRPath","Parts",Path_CHR,MAX_PATH,Config);
	GetPrivateProfileString("CopyNES","WRAMPath","Finished",Path_WRAM,MAX_PATH,Config);
	GetPrivateProfileString("CopyNES","NESPath","Finished",Path_NES,MAX_PATH,Config);
	GetPrivateProfileString("CopyNES","CRCPath","CRC",Path_CRC,MAX_PATH,Config);
	GetPrivateProfileString("CopyNES","NSFPath","NSF",Path_NSF,MAX_PATH,Config);
	GetPrivateProfileString("CopyNES","PluginPath","Plugdone",Path_PLUG,MAX_PATH,Config);
	strcpy(Path_PRG,addSlash(_fullpath(strcpy(tmpdir,Path_MAIN),Path_PRG,MAX_PATH)));
	strcpy(Path_CHR,addSlash(_fullpath(strcpy(tmpdir,Path_MAIN),Path_CHR,MAX_PATH)));
	strcpy(Path_WRAM,addSlash(_fullpath(strcpy(tmpdir,Path_MAIN),Path_WRAM,MAX_PATH)));
	strcpy(Path_NES,addSlash(_fullpath(strcpy(tmpdir,Path_MAIN),Path_NES,MAX_PATH)));
	strcpy(Path_CRC,addSlash(_fullpath(strcpy(tmpdir,Path_MAIN),Path_CRC,MAX_PATH)));
	strcpy(Path_NSF,addSlash(_fullpath(strcpy(tmpdir,Path_MAIN),Path_NSF,MAX_PATH)));
	strcpy(Path_PLUG,addSlash(_fullpath(strcpy(tmpdir,Path_MAIN),Path_PLUG,MAX_PATH)));
	ParPort = GetPrivateProfileInt("CopyNES","ParPort",0,Config);
	GetPrivateProfileString("CopyNES","ParAddr","0",tmpstr,16,Config);
	sscanf(tmpstr,"%X",&ParAddr);
	GetPrivateProfileString("CopyNES","ParECP","0",tmpstr,16,Config);
	sscanf(tmpstr,"%X",&ParECP);
	if ((ParAddr == 0) && (ParPort > 0))
	{
		// update old config data
		if (ParPort == 1)
			ParAddr = 0x378;
		if (ParPort == 2)
			ParAddr = 0x278;
		if (ParPort == 3)
			ParAddr = 0x3BC;
		if ((ParPort == 4) || (ParPort == 9))
			ParAddr = 0xD800;
		if (ParPort == 5)
			ParAddr = 0xE000;
		if (ParPort == 6)
			ParAddr = 0xE800;
		if (ParPort > 4)
			ParPort = 4;
		WriteConfig();
	}
	if ((ParAddr > 0) && (ParECP == 0))
	{
		ParECP = 0x400;
		WriteConfig();
	}
}
void	WriteConfig (void)
{
	char Config[MAX_PATH], tmpdir[MAX_PATH], tmpstr[16];
	strcpy(Config,Path_MAIN);
	strcat(Config,"CopyNESW.ini");
	sprintf(tmpstr,"%i",SaveCRC);
	WritePrivateProfileString("CopyNES","SaveCRC",tmpstr,Config);
	sprintf(tmpstr,"%i",SaveFiles);
	WritePrivateProfileString("CopyNES","SaveFiles",tmpstr,Config);
	WritePrivateProfileString("CopyNES","PRGPath",_relpath(strcpy(tmpdir,Path_PRG),Path_MAIN),Config);
	WritePrivateProfileString("CopyNES","CHRPath",_relpath(strcpy(tmpdir,Path_CHR),Path_MAIN),Config);
	WritePrivateProfileString("CopyNES","WRAMPath",_relpath(strcpy(tmpdir,Path_WRAM),Path_MAIN),Config);
	WritePrivateProfileString("CopyNES","NESPath",_relpath(strcpy(tmpdir,Path_NES),Path_MAIN),Config);
	WritePrivateProfileString("CopyNES","CRCPath",_relpath(strcpy(tmpdir,Path_CRC),Path_MAIN),Config);
	WritePrivateProfileString("CopyNES","NSFPath",_relpath(strcpy(tmpdir,Path_NSF),Path_MAIN),Config);
	WritePrivateProfileString("CopyNES","PluginPath",_relpath(strcpy(tmpdir,Path_PLUG),Path_MAIN),Config);
	sprintf(tmpstr,"%i",ParPort);
	WritePrivateProfileString("CopyNES","ParPort",tmpstr,Config);
	sprintf(tmpstr,"%X",ParAddr);
	WritePrivateProfileString("CopyNES","ParAddr",tmpstr,Config);
	sprintf(tmpstr,"%X",ParECP);
	WritePrivateProfileString("CopyNES","ParECP",tmpstr,Config);
}

int	FindVersion (void)
{
	BYTE i;
        OpenStatus(topHWnd);
	StatusText("Querying CopyNES BIOS version...");
	if (!WriteByteEx(0xA2,3,FALSE))
	{
		StatusText("Failed to send version request!");
		StatusText("Make sure your CopyNES is connected and turned on!");
		StatusOK();
		return 0;	// write failed, device not present
	}
	StatusText("Waiting for reply...");
	if (!ReadByteEx(&i,3,FALSE))
	{
		StatusText("Version reply not received! Assuming version 1 BIOS.");
		Sleep(SLEEP_LONG);
		CloseStatus();
		InitPort();
		ResetNES(RESET_COPYMODE);
		return 1;
	}
	if (i == 0xA2)
	{
		StatusText("Your parallel port does not support bidirectional communication!");
		StatusText("Please correct your BIOS settings and try again.");
		StatusOK();
		return 0;
	}
	StatusText("CopyNES identified as version %i.",i);
	Sleep(SLEEP_LONG);
	CloseStatus();
	// technically, these shouldn't be needed
	InitPort();
	ResetNES(RESET_COPYMODE);
	return i;
}

PCategory *Plugins = NULL;

static	void	trim (char *str)
{
	int i;
	for (i = strlen(str) - 1; i >= 0; i--)
	{
		if (str[i] == ' ')
			str[i] = 0;
		else	break;
	}
}

BOOL	Startup	(void)
{
	char mapfile[MAX_PATH];
	FILE *PlugList;
	char *Data, *C1, *C2, *C3, *C4;
	int i, j;

	int numcats;
	int col0, col1, col2, col3, col4;

	InitCRC();
	GetProgPath();
	GetConfig();

	sprintf(mapfile,"%s%s",Path_MAIN, "mappers.dat");
	PlugList = fopen(mapfile, "rt");
	if (PlugList == NULL)
	{
		MessageBox(topHWnd,"Unable to open plugin list!", "CopyNES", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	// step 1 - count how many categories we have

	fscanf(PlugList,"%i %i %i %i %i %i\n", &i, &col0, &col1, &col2, &col3, &col4);
	col0--; col1--; col2--; col3--; col4--;
	C1 = (char *)malloc(col1 - col0 + 1);	C1[col1 - col0] = 0;
	C2 = (char *)malloc(col2 - col1 + 1);	C2[col2 - col1] = 0;
	C3 = (char *)malloc(col3 - col2 + 1);	C3[col3 - col2] = 0;
	C4 = (char *)malloc(col4 - col3 + 1);	C4[col4 - col3] = 0;
	Data = (char *)malloc(col4);

	numcats = (i / 4) - 7;

	Plugins = (PCategory *)malloc(numcats * sizeof(PCategory));
	memset(Plugins, 0, numcats * sizeof(PCategory));

	// step 2 - count how many plugins are in each category

	fscanf(PlugList, "%i", &j);
	for (i = 0; i < numcats - 1; i++)
	{
		int point, numplugs;
		fscanf(PlugList, "%i", &point);
		numplugs = point - j;
		Plugins[i] = (PCategory)malloc(sizeof(TCategory));
		Plugins[i]->list = (PPlugin *)malloc(numplugs * sizeof(PPlugin));
		memset(Plugins[i]->list, 0, numplugs * sizeof(PPlugin));
		j = point;
	}
	while (fgetc(PlugList) != '\n')
		;

	// step 3 - read in the list

	i = 0;
	j = -1;
	while (!feof(PlugList))
	{
		fscanf(PlugList,"%[^\n]\n",Data);
		memcpy(C1,Data + col0, col1 - col0);
		memcpy(C2,Data + col1, col2 - col1);
		memcpy(C3,Data + col2, col3 - col2);
		memcpy(C4,Data + col3, col4 - col3);
		trim(C1);
		trim(C2);
		trim(C3);
		trim(C4);
		if (!strcmp(C1,"*"))
		{
			if (!strcmp(C4,"end"))
				break;
			j++;
			Plugins[j]->type = atoi(C3);
			Plugins[j]->desc = strdup(C4);
			i = 0;
		}
		else
		{
			Plugins[j]->list[i] = (PPlugin)malloc(sizeof(TPlugin));
			Plugins[j]->list[i]->name = strdup(C1);
			Plugins[j]->list[i]->file = strdup(C2);
			Plugins[j]->list[i]->num = atoi(C3);
			Plugins[j]->list[i]->desc = strdup(C4);
			i++;
		}
	}

	fclose(PlugList);
	free(C1);
	free(C2);
	free(C3);
	free(C4);
	free(Data);

	// RAMCART - create new category for upload plugins
	numcats++;
	Plugins = (PCategory *)realloc(Plugins, numcats * sizeof(PCategory));	// allocate another slot
	memset(&Plugins[numcats-1], 0, sizeof(PCategory));		// clear the new one at the end

	i = numcats - 2;						// and then populate the one 2nd from the end
	j = 3;			// number of plugins below
	Plugins[i] = (PCategory)malloc(sizeof(TCategory));
	Plugins[i]->list = (PPlugin *)malloc((j + 1) * sizeof(PPlugin));
	memset(Plugins[i]->list, 0, (j + 1) * sizeof(PPlugin));
	Plugins[i]->type = PLUG_UPLOAD;
	Plugins[i]->desc = strdup("RAM/Flash cartridge programmer");

	Plugins[i]->list[0] = (PPlugin)malloc(sizeof(TPlugin));
	Plugins[i]->list[0]->name = strdup("NRAM");
	Plugins[i]->list[0]->file = strdup("ram.bin");
	Plugins[i]->list[0]->num = 0;
	Plugins[i]->list[0]->desc = strdup("NROM cart with 32K RAM for PRG and 8K RAM for CHR");

	Plugins[i]->list[1] = (PPlugin)malloc(sizeof(TPlugin));
	Plugins[i]->list[1]->name = strdup("CNRAM");
	Plugins[i]->list[1]->file = strdup("cnram.bin");
	Plugins[i]->list[1]->num = 1;
	Plugins[i]->list[1]->desc = strdup("CNROM cart with 32K of PRG and CHR RAM");

	Plugins[i]->list[2] = (PPlugin)malloc(sizeof(TPlugin));
	Plugins[i]->list[2]->name = strdup("UfROM");
	Plugins[i]->list[2]->file = strdup("uxram.bin");
	Plugins[i]->list[2]->num = 2;
	Plugins[i]->list[2]->desc = strdup("Membler's flash cart for UNROM fun");
	// RAMCART

	if (!OpenPort(ParPort, ParAddr, ParECP))
	{
		HWVer = 0;
		return TRUE;
	}
	InitPort();
	ResetNES(RESET_COPYMODE);
	HWVer = FindVersion();
	return TRUE;
}

BOOL	Shutdown (void)
{
	int i, j;
	if (Plugins)
	{
		for (i = 0; Plugins[i] != NULL; i++)
		{
			for (j = 0; Plugins[i]->list[j] != NULL; j++)
			{
				free(Plugins[i]->list[j]->desc);
				free(Plugins[i]->list[j]->file);
				free(Plugins[i]->list[j]->name);
				free(Plugins[i]->list[j]);
			}
			free(Plugins[i]->desc);
			free(Plugins[i]);
		}
		free(Plugins);
	}
	ClosePort();
	return TRUE;
}
