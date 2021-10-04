/***********************************************************************
**
** MQ2RaidUtils.cpp : Defines the entry point for the DLL application.
**
************************************************************************
**
** RaidUtils is inspired by the work Sorcier and the RaidManager macro
** posted in the www.macroquest2.com VIP forums.
**
** Purpose: The purpose of this plugin is to make leading raids a little
** easier by adding an "All" option dz/tasks add and remove commands.
**
** The secondary feature of this plugin is to take raid attendance.
**
** For Non-Raid leaders the option to automatically perform some related
** tasks such as joining chat channels or removing anon / role may be added
** later
**
************************************************************************
**
** Commands:
** /dzadd      <player1> ... <playerN> or "All" - Adds a list of players.
** /dzremove   <player1> ... <playerN> or "All" - Removes a list of players.
** /dzshow                                      - Shows players not in zone.
**
** /taskadd    <player1> ... <playerN> or "All" - Adds a list of players.
** /taskremove <player1> ... <playerN> or "All" - Removes a list of players.
** /taskshow                                    - Shows players not in zone.
**
** Configuration:
**
** /raidtools help                              - Show help
** /raidtools dump                              - Performs raid dump
** /raidtools log on|off                        - Turns auto logging on/off
** /raidtools log File <name>  (1)
** /raidtools log Times <str>   (2)
** /raidtools log Every <interval> (3)
**
** 1: The File name will be parsed using strftime the most common parameters are
**		%a	Abbreviated weekday name
**		%b	Abbreviated month name
**		%d 	Day of month as decimal number (01 � 31)
**		%H 	Hour in 24-hour format (00 � 23)
**		%m 	Month as decimal number (01 � 12)
**		%M 	Minute as decimal number (00 � 59)
**		%S 	Seconds as decimal number (00 � 61)
**		%Y 	Year with century, as decimal number
**		%% 	Percent sign
**    The name can also have the following
**		%G  Guild name
**		%V  Server name
**      %C  Character name
**      %P  Everquest path
**      %Q  Macroquest path
**
** 2: The logTime format for exmpale Mon 20:00-23:00 | Tue 20:00-02:00 | Sun 11:00-15:00
**									 Tue 8pm-11pm, Wed 9pm-1am
**
** 3: The log interval is (1-60) and represents every N minutes dump the raid log.
**
************************************************************************/

#include <mq/Plugin.h>

PreSetup("MQ2RaidUtils");
PLUGIN_VERSION(1.0);

constexpr int LOGTICKCOUNT = 110;

int    IniLoaded = 0;

int    LogIsON = 0;
char   LogFileFormat[256];
char   LogFileName[1024];
char   LogTimes[256];
int    LogEvery = 30;

int    LogRaidStart[20];
int    LogRaidStop[20];
int    LogRaidTime[999];
int    LogRaidTimes = 0;
int    LogLastUpdated = 0;
time_t LogStartTime = 0;
struct tm pTime[1];
int    LogTickCount = 0;

int    LogSaveChannel = 0;


char   RaidStartCmd[MAX_STRING];
char   RaidStopCmd[MAX_STRING];

static char rtLogFmt[] = "%P\\RaidRoster-%Y%m%d-%H%M%S.txt";		// default log format

typedef struct
{
	char *Name;
	int  inRaid;
	int  inDZ;
	int  inTask;
	int  inZone;
} trLIST, *tpLIST;

trLIST   RaidList[0x48];
char    *pTaskLeader = 0;
char     GroupList[6][0x40];



int cmpList (const void * a, const void * b)
{
	tpLIST pa = (tpLIST)a;
	tpLIST pb = (tpLIST)b;
	if (pa->Name == NULL && pb->Name == NULL)
		return 0;
	if (pa->Name == NULL)
		return 1;
	if (pb->Name == NULL)
		return -1;
	return _stricmp( ((tpLIST)a)->Name, ((tpLIST)b)->Name );
}

static char OnStr[] = "On";
static char OffStr[] = "Off";

//                            0    1     2     3      4     5    6      7    8     9    10  11  12   13   14
static char *tokenList[] = { " ","am", "pm" , "sun","mon","tue","wed","thu","fri","sat","+","-","|",",", NULL};

void AddRaidTime(int startday,int starthr,int startmin,int stopday,int stophr,int stopmin)
{
	int max;
	int min;

	min = startday*3600 + starthr * 60 + startmin;
	max = stopday*3600  + stophr * 60  + stopmin;

	while ( min <= max && LogRaidTimes < 999)
	{
		LogRaidTime[ LogRaidTimes++ ] = min;
		min += LogEvery;
	}
}

void ParseRaidTimeStr()
{
	char argv[32];
	char szTimes[256];
	char *p = LogTimes;
	char *q = szTimes;
	int  argc=0;
	int  i,t;
	int  v[4],n;
	int  ln;
	int  day = 0;
	int  start = 0;
	int  stop = 0;

	LogRaidTimes = 0;

	while (p && *p)
		*q++ = tolower(*p++);
	*q++ = '|';
	*q++ = 0;

	memset(argv,0,sizeof(argv));

	p = szTimes;

	while (p && (*p == ' ' || *p == '\t')) p++;

	q = p;
	day = 0;
	n = v[0] = v[1] = v[2] = v[3] =  0 ;

	while (p && *p)
	{
		t  = -1;
		ln = 0;
		for (i=0; tokenList[i]; i++)
		{
			if (strncmp(p,tokenList[i],strlen(tokenList[i]))==0)
			{
				ln = strlen(tokenList[i]);
				t=i;
			}
		}

		if (t==-1 && p && *p >= '0' && *p <= '9')
		{
			while ( (*p >= '0' && *p <= '9') || *p==':' )
			{
				if (*p == ':' && n < 4)
					n++;
				else
					v[n] = v[n] * 10 + *p - '0';
				p++;
			}
			t = 14;
		}

		if (t== 11 && n < 4)
			n=2;


		if (t!=-1)
		{
			p+= ln;

			if (t>= 3 && t <= 9) 	// Mon-Fri
				day = t;

			if (t== 2)				// PM
			{
				if ( (n == 0 || n==1 ) && v[0] <= 12)
					v[0] += 12;
				if ( (n == 1 || n==2 ) && v[2] <= 12)
					v[2] += 12;
			}

			if (t==12 || t == 13) 	// | or ,
			{
				int d2 = day;
				if (v[2] < v[0])
					d2 = day+1;
				AddRaidTime(day,v[0],v[1],d2,v[2],v[3]);
				day = n = v[0] = v[1] = v[2] = v[3] = 0;
			}
		}
		else
			p++;
	}
}

void BuildLogFileName()
{
	char TempFileFormat[1024];
	char* pName = GetCharInfo()->Name;
	char* pServ = EQADDR_SERVERNAME;

	char* pGuildID = const_cast<char*>(GetGuildByID(GetCharInfo()->GuildID));
	char* pEQPath = gPathEverQuest;
	char* pMQPath = gPathConfig;

	char* p = LogFileFormat;
	char* q = TempFileFormat;
	char* s = nullptr;

	LogStartTime = time(nullptr);
	localtime_s(pTime, &LogStartTime);

	// First pass copy string and replace %S,%G,%C with EQ specific info
	while (p && *p)
	{
		if (*p=='%')
		{
			switch (p[1])
			{
				case 'V':   p+=2; s = pServ;    break;
				case 'G':   p+=2; s = pGuildID; break;
				case 'C':   p+=2; s = pName;    break;
				case 'P':   p+=2; s = pEQPath;  break;
				case 'Q':   p+=2; s = pMQPath;  break;
				case '%':   *q++ = *p++; // fall through to default
			    default:    *q++ = *p++;        break;
			}

			while (s && *s) *q++ = *s++;
		}
		else
		{
			*q++ = *p++;
		}
	}
	*q++ = NULL;

//	WriteChatf("LogFileName before strftime = [%s]\n",TempFileFormat);
	strftime(LogFileName, 1024, TempFileFormat, pTime);
//	WriteChatf("LogFileName after strftime = [%s]\n",LogFileName);
}

char *OnOffStr(int status)
{
	if (status)
		return OnStr;
	else
		return OffStr;
}

void ShowHelpStatus(int ShowHelp,int ShowStatus)
{
	if (ShowHelp) {
		WriteChatf("MQ2RaidUtils:: Task Command list");
		WriteChatf("  /dzAdd      [player|all]    : Adds group/raid members to dz");
		WriteChatf("  /taskAdd    [player|all]    : Adds group/raid members to task");
		WriteChatf("  /dzRemove   [player|all]    : Removes members of group/raid from dz");
		WriteChatf("  /taskRemove [player|all]    : Removes members of group/raid from task");
		WriteChatf("  /dzShow                     : Shows members who are not in zone");
		WriteChatf("  /taskShow                   : Shows members who are not in zone");
		WriteChatf("");
		WriteChatf("MQ2RaidUtils:: Raid Log Command list");
		WriteChatf("  /raidtools Log (on/off)      : turn on automatic raid dumps");
		WriteChatf("  /raidtools Log File <Format> : describe the log file name where:");
		WriteChatf("			%%G   Guild name");
		WriteChatf("			%%V   serVer name");
		WriteChatf("			%%C   Character name");
		WriteChatf("			%%P   Everquest Path");
		WriteChatf("			%%Q   MacroQuest path");
		WriteChatf("			%%a   Abbreviated weekday name");
		WriteChatf("			%%b   Abbreviated month name");
		WriteChatf("			%%d   Day of month as decimal number (01 - 31)");
		WriteChatf("			%%H   Hour in 24-hour format (00 - 23)");
		WriteChatf("			%%m   Month as decimal number (01 - 12)");
		WriteChatf("			%%M   Minute as decimal number (00 - 59)");
		WriteChatf("			%%S   Seconds as decimal number (00 - 59)");
		WriteChatf("			%%Y   Year with century, as decimal number");
		WriteChatf("  /raidtools Log Times <Format> : describes when to log");
		WriteChatf("            example: Mon 20:00-23:00 | Tue 20:00-02:00");
		WriteChatf("  /raidtools Log Every <Min>    : take raid dumps every M minutes");
		WriteChatf("-----------------------------------------------------------------------");
	}

	if (ShowStatus) {
		WriteChatf("MQ2RaidUtils:: Log Status");
		WriteChatf("  /raidtools Log (on/off) = [%s].",OnOffStr(LogIsON));
		WriteChatf("  /raidtools Log File     = [%s]",LogFileFormat,LogFileName);
		WriteChatf("             Actual File  = [%s]",LogFileName);
		WriteChatf("  /raidtools Log Times    = [%s]",LogTimes);
		WriteChatf("  /raidtools Log Every    = [%d] min",LogEvery);
	}
}

void LoadINIFile(void)
{
	char szTemp[256];
	char  *pName = GetCharInfo()->Name;

	LogFileName[0] = 0;
	GetPrivateProfileString(pName,"LogIsON"			,"0"		,szTemp			,256,INIFileName);		LogIsON=atoi(szTemp);
	GetPrivateProfileString(pName,"LogEvery"		,"30"		,szTemp			,256,INIFileName);		LogEvery=atoi(szTemp);
	GetPrivateProfileString(pName,"LogFileFormat"	,rtLogFmt	,LogFileFormat	,256,INIFileName);
	GetPrivateProfileString(pName,"LogTimes"		,"none"		,LogTimes		,256,INIFileName);
	BuildLogFileName();
	ParseRaidTimeStr();
	IniLoaded = TRUE;
}

int SaveINIFile(void)
{
	char  *pName = GetCharInfo()->Name;
	char  szTemp1[256];
	char  szTemp2[256];

	sprintf_s(szTemp1,"%d",LogIsON);
	sprintf_s(szTemp2,"%d",LogEvery);

	WritePrivateProfileString(pName, "LogIsON"		, szTemp1		, INIFileName);
	WritePrivateProfileString(pName, "LogEvery"		, szTemp2		, INIFileName);
	WritePrivateProfileString(pName, "LogFileFormat", LogFileFormat	, INIFileName);
	WritePrivateProfileString(pName, "LogTimes"		, LogTimes		, INIFileName);

	return 1;
}

int SetLog(int on)
{
	LogIsON = on;
	WriteChatf("  /raidtools Log (on/off) = [%s].",OnOffStr(LogIsON));
	return (SaveINIFile());
}

int SetLogEvery(char *s)
{
	int t;
	if (!s) return 0;

	t = atoi(s);
	if (t >= 1 && t<=60)
	{
		LogEvery = t;
		ParseRaidTimeStr();
		WriteChatf("  /raidtools Log Every    = [%d] min",LogEvery);
		return (SaveINIFile());
	}
	return 0;
}

int SetTimes(char *s)
{
	if (!s) return 0;
	strcpy_s(LogTimes,s);
	ParseRaidTimeStr();
	WriteChatf("  /raidtools Log Times    = [%s]",LogTimes);
	return (SaveINIFile());
}

int SetFileName(char *s)
{
	if (!s) return 0;
	strcpy_s(LogFileFormat,s);
	BuildLogFileName();
	WriteChatf("  /raidtools Log File     = [%s]",LogFileFormat,LogFileName);
	WriteChatf("             Actual File  = [%s]",LogFileName);
	return (SaveINIFile());
}

void DoRaidDump(void);

void rtCommand(PSPAWNINFO pCHAR, PCHAR zLine)
{
	int  OK = 0;
	int  ShowHelp = 0;
	int  ShowStat = 0;

	char Arg1[MAX_STRING]; GetArg(Arg1,zLine,1);
	char Arg2[MAX_STRING]; GetArg(Arg2,zLine,2);
	char Arg3[MAX_STRING]; GetArg(Arg3,zLine,3);
	char *pArg3 = strstr(zLine,Arg3);

	if (_stricmp(Arg1,"")==0)		ShowHelp = 1;
	if (_stricmp(Arg1,"Help")==0)	ShowHelp = 1;
	if (_stricmp(Arg1,"Status")==0)	ShowStat = 1;
	if (_stricmp(Arg1,"Dump")==0)	DoRaidDump();

	if (_stricmp(Arg1,"Log")==0)
	{
		if (_stricmp(Arg2,"On")==0)		OK = SetLog(1);
		if (_stricmp(Arg2,"Off")==0)		OK = SetLog(0);
		if (_stricmp(Arg2,"Every")==0) 	OK = SetLogEvery(pArg3);
		if (_stricmp(Arg2,"Times")==0)	OK = SetTimes(pArg3);
		if (_stricmp(Arg2,"File")==0)	OK = SetFileName(pArg3);
		if (!OK)						ShowStat = 1;
	}
	ShowHelpStatus(ShowHelp ,ShowStat);
}

void DoRaidDump(void)
{
	int i,m;
	char cmd[256];
	FILE *fp;
	EQRAIDMEMBER *p;
	BuildLogFileName();
	WriteChatf("RaidTools:: Taking raid attendance: %s",LogFileName);
	EzCommand("/popup Taking raid attendance");
	fopen_s(&fp,LogFileName,"a");
	if (!fp)
	{
		WriteChatf("Failed to open %s , aborting log.",LogFileName);
		return;
	}
	if (pRaid && pRaid->RaidMemberCount>0)
		for (int m=0; m<72; m++)
			if (pRaid->RaidMemberUsed[m])
			{
				p = &pRaid->RaidMember[m];
				fprintf(fp,"%d\t%s\t%d\t%s\t%s\n",p->GroupNumber,p->Name,p->nLevel,ClassInfo[p->nClass].Name,p->RaidNote);
			}
	fprintf(fp,"------------------------\n");
	fclose(fp);
	LogSaveChannel = GetTickCount();

	m = 0;
	if(pEverQuest->ChatService)
		m = pEverQuest->ChatService->ActiveChannels;

	for (i=1; i<=m; i++)
	{
		sprintf_s(cmd,"/timed 1 /list %d",i);
		EzCommand(cmd);
	}
	EzCommand("/timed 20 /");
}

void GetRaidList(PSPAWNINFO pCHAR, int ShowList)
{
	int m,n=0;
	int max=0;
	trLIST pKey[1],*pList;
	memset(RaidList,0,sizeof(RaidList));
	if (pRaid && pRaid->RaidMemberCount>0)
	{
		for (m=0; m<72; m++)
			if (pRaid->RaidMemberUsed[m])
			{
				RaidList[m].Name = pRaid->RaidMember[m].Name;
				RaidList[m].inRaid = 1;
				max++;
			}
	}
	else
	{
		PCHARINFO pChar=GetCharInfo();
		memset(GroupList,0,sizeof(GroupList));

		for (int i = 0; i < MAX_GROUP_SIZE; i++)
		{
			if (CGroupMember* pGroupMember = pLocalPC->Group->GetGroupMember(i))
			{
				strcpy_s(GroupList[i], pGroupMember->GetName());
				RaidList[i].Name = (char*)GroupList[i];
			}
		}
	}


	qsort(RaidList,72, sizeof(trLIST), (int(*)(const void*,const void*)) cmpList);

	DynamicZonePlayerInfo* pDZList = pDZMember;
	while(pDZList)
	{
		pKey->Name = pDZList->Name;
		pList = (trLIST *)bsearch(pKey,RaidList, max, sizeof(trLIST) , (int(*)(const void*,const void*)) cmpList);
		if (pList)
			pList->inDZ = 1;
		else
		{
			RaidList[max+n].Name = pDZList->Name;
			RaidList[max+n].inDZ = 1;
			n++;
		}
		pDZList=pDZList->pNext;
	}
	max += n;
	n = 0;
	qsort(RaidList,72, sizeof(trLIST), (int(*)(const void*,const void*)) cmpList);

	pTaskLeader = 0;
	SharedTaskPlayerInfo* pTaskList = pTaskMember;
	while(pTaskList)
	{
		pKey->Name = pTaskList->Name;
		pList = (trLIST *)bsearch(pKey,RaidList, max, sizeof(trLIST) , (int(*)(const void*,const void*)) cmpList);
		if (pList)
			pList->inTask = 1;
		else
		{
			RaidList[max+n].Name = pTaskList->Name;
			RaidList[max+n].inTask = 1;
			n++;
		}
		if (pTaskList->IsLeader)
			pTaskLeader = pTaskList->Name;

		pTaskList=pTaskList->pNext;
	}
	max += n;
	n = 0;
	qsort(RaidList,72, sizeof(trLIST), (int(*)(const void*,const void*)) cmpList);

    PSPAWNINFO pSpawn=(PSPAWNINFO)pSpawnList;
    while(pSpawn)
    {
		if(pSpawn->Type==SPAWN_PLAYER)
		{
			pKey->Name = pSpawn->Name;
			pList = (trLIST *)bsearch(pKey,RaidList, max, sizeof(trLIST) , (int(*)(const void*,const void*)) cmpList);
			if (pList)
				pList->inZone = 1;
		}
		pSpawn=pSpawn->pNext;
    }

	if (ShowList)
	{
		WriteChatf("Raid Dz Task Zone Name ");
		for (m=0; m<72; m++)
			if (RaidList[m].Name)
				WriteChatf("%3d %3d %3d %4d  %s",RaidList[m].inRaid,RaidList[m].inDZ,RaidList[m].inTask,RaidList[m].inZone,RaidList[m].Name);
	}
}

// Stub for /dzAddPlayer <>
void rtDZAdd(PSPAWNINFO pCHAR, PCHAR zLine)
{
	int i;
	char arg[MAX_STRING];
	char cmd[MAX_STRING];

	if (strcmp((char*)instExpeditionLeader,pCHAR->Name)!=0)
	{
		WriteChatf("You are not the DZ Leader");
		return;
	}

	i = 1;
	GetArg(arg,zLine,i++);

	if (_stricmp(arg,"all")==0 || _stricmp(arg,"raid")==0)
	{
		GetRaidList(pCHAR,FALSE);
		for (i=0; i<72; i++)
		{
			if (RaidList[i].Name && !RaidList[i].inDZ)
			{
				sprintf_s(cmd,"/dzaddplayer %s",RaidList[i].Name);
				EzCommand(cmd);
			}
		}
		return;
	}

	while (arg[0]!=0)
	{
		sprintf_s(cmd,"/dzaddplayer %s",arg);
		EzCommand(cmd);
		GetArg(arg,zLine,i++);
	}
}

void rtDZRemove(PSPAWNINFO pCHAR, PCHAR zLine)
{
	int i;
	char arg[MAX_STRING];
	char cmd[MAX_STRING];

	if (strcmp((char*)instExpeditionLeader,pCHAR->Name)!=0)
	{
		WriteChatf("You are not the DZ Leader");
		return;
	}

	i = 1;
	GetArg(arg,zLine,i++);

	if (_stricmp(arg,"all")==0 || _stricmp(arg,"raid")==0)
	{
		GetRaidList(pCHAR,FALSE);
		for (i=0; i<72; i++)
		{
			if (RaidList[i].Name && RaidList[i].inDZ && _stricmp(pCHAR->Name,RaidList[i].Name)!=0)
			{
				sprintf_s(cmd,"/dzremoveplayer %s",RaidList[i].Name);
				EzCommand(cmd);
			}
		}
		EzCommand("/dzquit");
		return;
	}

	while (arg[0]!=0)
	{
		sprintf_s(cmd,"/dzremoveplayer %s",arg);
		EzCommand(cmd);
		GetArg(arg,zLine,i++);
	}
}

void rtTaskAdd(PSPAWNINFO pCHAR, PCHAR zLine)
{
	int i;
	char arg[MAX_STRING];
	char cmd[MAX_STRING];

	GetRaidList(pCHAR,FALSE);

	if (!pTaskLeader || strcmp(pTaskLeader,pCHAR->Name)!=0)
	{
		WriteChatf("You are not the Task Leader");
		return;
	}

	i = 1;
	GetArg(arg,zLine,i++);

	if (_stricmp(arg,"all")==0 || _stricmp(arg,"raid")==0)
	{
		for (i=0; i<72; i++)
		{
			if (RaidList[i].Name && !RaidList[i].inTask)
			{
				sprintf_s(cmd,"/taskaddplayer %s",RaidList[i].Name);
				EzCommand(cmd);
				WriteChatf(cmd);
			}
		}
		return;
	}

	while (arg[0]!=0)
	{
		sprintf_s(cmd,"/taskaddplayer %s",arg);
		EzCommand(cmd);
		WriteChatf(cmd);
		GetArg(arg,zLine,i++);
	}
}

void rtTaskRemove(PSPAWNINFO pCHAR, PCHAR zLine)
{
	int i;
	char arg[MAX_STRING];
	char cmd[MAX_STRING];

	GetRaidList(pCHAR,FALSE);

	if (!pTaskLeader || strcmp(pTaskLeader,pCHAR->Name)!=0)
	{
		WriteChatf("You are not the Task Leader");
		return;
	}

	i = 1;
	GetArg(arg,zLine,i++);

	if (_stricmp(arg,"all")==0 || _stricmp(arg,"raid")==0)
	{
		for (i=0; i<72; i++)
		{
			if (RaidList[i].inTask && _stricmp(pCHAR->Name,RaidList[i].Name)!=0)
			{
				sprintf_s(cmd,"/taskremoveplayer %s",RaidList[i].Name);
				EzCommand(cmd);
				WriteChatf(cmd);
			}
		}
		EzCommand("/taskquit");
		return;
	}

	while (arg[0]!=0)
	{
		sprintf_s(cmd,"/taskremoveplayer %s",arg);
		EzCommand(cmd);
		WriteChatf(cmd);
		GetArg(arg,zLine,i++);
	}
}

void rtShowMissing(PSPAWNINFO pCHAR, PCHAR zLine)
{
	int i;
	GetRaidList(pCHAR,TRUE);
	WriteChatf("------------------------");
	WriteChatf("Players not in Zone:");
	WriteChatf("Raid Dz Task Zone Name ");
	for (i=0; i<72; i++)
	{
		if (RaidList[i].Name && (RaidList[i].inDZ || RaidList[i].inTask) && !RaidList[i].inZone)
			WriteChatf("%3d %3d %3d %4d  %s",RaidList[i].inRaid,RaidList[i].inDZ,RaidList[i].inTask,RaidList[i].inZone,RaidList[i].Name);
	}
	WriteChatf("------------------------");
}


// This is called every time MQ pulses
PLUGIN_API void OnPulse()
{
	int    n,t;
	static char cmd[256];

	LogTickCount++;
	if (LogTickCount < LOGTICKCOUNT)
		return;

	LogTickCount=0;
	LogStartTime = time(NULL);
	localtime_s(pTime,&LogStartTime);
	// pTime->tm_wday (0=sun,1=mon,..) vs (3=sun,4=mon...

	t = (pTime->tm_wday + 3)*3600 + (pTime->tm_hour)*60 + (pTime->tm_min);
	for (n=0; n<LogRaidTimes; n++)
	{
		if ( t==LogRaidTime[n] && LogLastUpdated != t )
		{
			DoRaidDump();
			LogLastUpdated = t;
			return;
		}
	}
}

PLUGIN_API bool OnIncomingChat(const char* Line, DWORD Color)
{
	static int flag = 0;
	static int flag2 = 0;
	FILE *fp;
	char szTime[256];



	DebugSpewAlways("MQ2ChatFilter::OnIncomingChat(%s)",Line);

	if (gGameState==GAMESTATE_INGAME)
	{
		if (GetTickCount() - LogSaveChannel > 60000 )
			LogSaveChannel = 0;

		if (LogSaveChannel == 0) return NULL;



		if (strncmp(Line,"Players in EverQuest",10)==0 || flag2 )
		{
//			WriteChatf("[RT] (%s)",Line);

			fopen_s(&fp,LogFileName,"a");
			if (fp)
			{
				if (flag2==0)	fprintf(fp,"\n");
				strftime(szTime,256,"[%a %b %d %H:%M:%S]",pTime);
				if (Line[0]=='-')
					fprintf(fp,"%s %s\n",szTime,"-------------------");
				else
					fprintf(fp,"%s %s\n",szTime,Line);
				fclose(fp);
			}

			flag2 = 1;
			if (strncmp(Line,"There are ",8)==0)
				flag2 = 0;
			return 1;
		}

		if (strncmp(Line,"Channel ",8)==0 || (flag && Line[0]==' ') )
		{
			flag++;

//			WriteChatf("[RT] (%s)",Line);

			fopen_s(&fp,LogFileName,"a");
			if (fp)
			{
				if (Line[0]!=' ') fprintf(fp,"\n");
				strftime(szTime,256,"[%a %b %d %H:%M:%S]",pTime);
				fprintf(fp,"%s %s\n",szTime,Line);
				fclose(fp);
			}

			return 1;
		}
		else
			flag = 0;

	}
	return NULL;
}



// Called once, when the plugin is to initialize
PLUGIN_API void InitializePlugin()
{
	DebugSpewAlways("Initializing MQ2RaidUtils");

	AddCommand("/raidtools",rtCommand);
	AddCommand("/dzadd",rtDZAdd);
	AddCommand("/dzRemove",rtDZRemove);
	AddCommand("/dzShow",rtShowMissing);

	AddCommand("/taskAdd",rtTaskAdd);
	AddCommand("/taskRemove",rtTaskRemove);
	AddCommand("/taskShow",rtShowMissing);

	if (gGameState == GAMESTATE_INGAME)
		LoadINIFile();
}

PLUGIN_API void ShutdownPlugin()
{
	DebugSpewAlways("Shutting down MQ2RaidUtils");

	RemoveCommand("/raidtools");
	RemoveCommand("/dzAdd");
	RemoveCommand("/dzRemove");
	RemoveCommand("/dzShow");
	RemoveCommand("/taskAdd");
	RemoveCommand("/taskRemove");
	RemoveCommand("/taskShow");
}

PLUGIN_API VOID SetGameState(DWORD GameState)
{
	if(GameState==GAMESTATE_INGAME && IniLoaded==0)
		LoadINIFile();
}
