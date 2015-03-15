#include "CopyNESW.h"

int	HWVer;
int	ParPort, ParAddr, ParECP;
BOOL	SaveCRC, SaveFiles, MakeUNIF;
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
	MakeUNIF = GetPrivateProfileInt("CopyNES","MakeUNIF",0,Config);
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
	if ((ParPort > 0) && (ParAddr == 0))
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
		if (ParECP == 0)
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
	sprintf(tmpstr,"%i",MakeUNIF);
	WritePrivateProfileString("CopyNES","MakeUNIF",tmpstr,Config);
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
	if (!ReadByteEx(i,3,FALSE))
	{
		if (ParPort == -1)
		{
			StatusText("Version reply not received!");
			StatusText("Make sure your CopyNES is connected and turned on!");
			StatusOK();
			return 0;	// write failed, device not present
		}
		else
		{
			StatusText("Version reply not received! Assuming version 1 BIOS.");
			Sleep(SLEEP_LONG);
			CloseStatus();
			InitPort();
			ResetNES(RESET_COPYMODE);
			return 1;
		}
	}
	if ((i == 0xA2) && (ParPort != -1))
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
	if (ParPort != -1)
		InitPort();
	ResetNES(RESET_COPYMODE);
	return i;
}

vector<Category *> Plugins;

static	void	trim (char *str)
{
	int i;
	for (i = strlen(str) - 1; i >= 0; i--)
	{
		if ((str[i] == ' ') || (str[i] == '\r') || (str[i] == '\n'))
			str[i] = 0;
		else	break;
	}
}

Plugin *makePlugin (const char *name, const char *filename, int num, const char *desc)
{
	// Special case for mapper 998 - return blank plugin
	if (num == 998)
	{
		Plugin *plugin = new Plugin;
		plugin->name = name;
		plugin->desc = desc;
		plugin->num = num;
		return plugin;
	}
	FILE *in = fopen(filename, "rb");
	if (!in)
	{
		in = fopen((string(Path_PLUG) + filename).c_str(), "rb");
		if (!in)
			return NULL;
	}
	fseek(in, 0, SEEK_END);
	int len = ftell(in);
	if (len < 1152)
	{
		fclose(in);
		return NULL;
	}
	fseek(in, 0, SEEK_SET);
	Plugin *plugin = new Plugin;
	plugin->name = name;
	plugin->desc = desc;
	plugin->num = num;
	fread(plugin->header, 1, 128, in);
	fread(plugin->data, 1, 1024, in);
	len -= 1152;
	// check for ROMstring
	if (len)
	{
		char *data = new char[len + 1];
		data[len] = 0;
		fread(data, 1, len, in);
		plugin->romstring = data;
		delete[] data;
	}
	fclose(in);
	return plugin;
}


BOOL	Startup	(void)
{
	char mapfile[MAX_PATH];
	FILE *PlugList;
	char *C1, *C2, *C3, *C4;

	InitCRC();
	GetProgPath();
	GetConfig();

	sprintf(mapfile,"%s%s",Path_MAIN, "mappers.dat");
	PlugList = fopen(mapfile, "rb");
	if (PlugList == NULL)
	{
		MessageBox(topHWnd,"Unable to open mappers.dat plugin list!", "CopyNES", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// step 1 - read header data

	// Header consists of length (in bytes) followed by column offsets (1-based) and category sizes
	char num[4];
	fread(num, 1, 4, PlugList);
	int header_len = atoi(num) - 4;

	int col0, col1, col2, col3, col4;

	char *header = new char[header_len + 1];
	fread(header, 1, header_len, PlugList);
	sscanf(header, "%i %i %i %i %i", &col0, &col1, &col2, &col3, &col4);
	free(header);

	C1 = new char[col1 - col0 + 1];	C1[col1 - col0] = 0;
	C2 = new char[col2 - col1 + 1];	C2[col2 - col1] = 0;
	C3 = new char[col3 - col2 + 1];	C3[col3 - col2] = 0;
	C4 = new char[col4 - col3 + 1];	C4[col4 - col3] = 0;
	
	// Next comes the actual plugin data

	Category *cat;
	Plugin *plug;
	while (!feof(PlugList))
	{
		fread(C1, 1, col1 - col0, PlugList);
		fread(C2, 1, col2 - col1, PlugList);
		fread(C3, 1, col3 - col2, PlugList);
		fread(C4, 1, col4 - col3, PlugList);
		trim(C1);
		trim(C2);
		trim(C3);
		trim(C4);
		if (!strcmp(C1,"*"))
		{
			if (!strcmp(C4,"end"))
				break;
			cat = new Category;
			cat->type = (plugin_type)atoi(C3);
			cat->desc = C4;
			Plugins.push_back(cat);
		}
		else
		{
			plug = makePlugin(C1, C2, atoi(C3), C4);
			if (plug)	cat->list.push_back(plug);
		}
	}

	fclose(PlugList);
	delete[] C1;
	delete[] C2;
	delete[] C3;
	delete[] C4;

	// RAMCART - create new category for upload plugins
	cat = new Category;
	cat->desc = "RAM/Flash cartridge programmer";
	cat->type = PLUG_UPLOAD;
	Plugins.push_back(cat);

	plug = makePlugin("NRAM", "ram.bin", 0, "NROM cart with 32K RAM for PRG and 8K RAM for CHR");
	if (plug)	cat->list.push_back(plug);

	plug = makePlugin("CNRAM", "cnram.bin", 1, "CNROM cart with 32K of PRG and CHR RAM");
	if (plug)	cat->list.push_back(plug);

	plug = makePlugin("UfROM", "uxram.bin", 2, "Memblers's flash cart for UNROM fun");
	if (plug)	cat->list.push_back(plug);

	plug = makePlugin("PowerPak Lite", "pplite.bin", 3, "PowerPak Lite RAM Cart loader");
	if (plug)	cat->list.push_back(plug);

	plug = makePlugin("PowerPak Boot", "pp.bin", 4, "PowerPak Boot Flasher");
	if (plug)	cat->list.push_back(plug);

	plug = makePlugin("Glider Flasher", "glider.bin", 5, "Glider House Flasher");
	if (plug)	cat->list.push_back(plug);
	// END RAMCART

	if (!OpenPort(ParPort, ParAddr, ParECP))
	{
		HWVer = 0;
		return TRUE;
	}
	if (ParPort != -1)
		InitPort();
	ResetNES(RESET_COPYMODE);
	Sleep(SLEEP_LONG);
	ResetNES(RESET_COPYMODE);
	HWVer = FindVersion();
	return TRUE;
}

BOOL	Shutdown (void)
{
	WriteConfig();
	while (Plugins.size())
	{
		Category *cat = Plugins.back();
		while (cat->list.size())
		{
			Plugin *plug = cat->list.back();
			delete plug;
			cat->list.pop_back();
		}
		delete cat;
		Plugins.pop_back();
	}
	ResetNES(RESET_PLAYMODE);
	ClosePort();
	return TRUE;
}
