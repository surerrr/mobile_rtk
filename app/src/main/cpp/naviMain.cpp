//---------------------------------------------------------------------------
// rtknavi : real-time positioning ap
//
//          Copyright (C) 2007-2014 by T.TAKASU, All rights reserved.
//
// options : rtknavi [-t title][-i file]
//
//           -t title   window title
//           -i file    ini file path
//
// version : $Revision:$ $Date:$
// history : 2008/07/14  1.0 new
//           2010/07/18  1.1 rtklib 2.4.0
//           2010/08/16  1.2 fix bug on setting of satellite antenna model
//           2010/09/04  1.3 fix bug on setting of receiver antenna delta
//           2011/06/10  1.4 rtklib 2.4.1
//           2012/04/03  1.5 rtklib 2.4.2
//           2014/09/06  1.6 rtklib 2.4.3
//---------------------------------------------------------------------------

#include <stdio.h>
#include <math.h>

#include "rtklib.h"
#include "naviMain.h"

//---------------------------------------------------------------------------
naviMain *MainForm;

#define PRGNAME     "RTKNAVI"           // program name
#define TRACEFILE   "rtknavi_%Y%m%d%h%M.trace" // debug trace file
#define STATFILE    "rtknavi_%Y%m%d%h%M.stat"  // solution status file
#define CLORANGE    (TColor)0x00AAFF
#define CLLGRAY     (TColor)0xDDDDDD
#define CHARDEG     0x00B0              // character code of degree
#define SATSIZE     20                  // satellite circle size in skyplot
#define MINSNR      10                  // minimum snr
#define MAXSNR      60                  // maximum snr
#define KEYF6       0x75                // code of function key F6
#define KEYF7       0x76                // code of function key F7
#define KEYF8       0x77                // code of function key F8
#define KEYF9       0x78                // code of function key F9
#define KEYF10      0x79                // code of function key F10
#define POSFONTNAME "Palatino Linotype"
#define POSFONTSIZE 12
#define MINBLLEN    0.01                // minimum baseline length to show

#define KACYCLE     1000                // keep alive cycle (ms)
#define TIMEOUT     10000               // inactive timeout time (ms)
#define DEFAULTPORT 52001               // default monitor port number
#define MAXPORTOFF  9                   // max port number offset

#define SQRT(x)     ((x)<0.0?0.0:sqrt(x))
#define MIN(x,y)    ((x)<(y)?(x):(y))


//---------------------------------------------------------------------------

rtksvr_t rtksvr;                        // rtk server struct
stream_t monistr;                       // monitor stream

//// show message in message area ---------------------------------------------
//extern "C" {
//	extern int showmsg(char *format,...) {return 0;}
//}
// convert degree to deg-min-sec --------------------------------------------
static void degtodms(double deg, double *dms)
{
	double sgn=1.0;
	if (deg<0.0) {deg=-deg; sgn=-1.0;}
	dms[0]=floor(deg);
	dms[1]=floor((deg-dms[0])*60.0);
	dms[2]=(deg-dms[0]-dms[1]/60.0)*3600;
	dms[0]*=sgn;
}
// execute command ----------------------------------------------------------
//int naviMain::ExecCmd(AnsiString cmd, int show)
//{
//	PROCESS_INFORMATION info;
//	STARTUPINFO si={0};
//	si.cb=sizeof(si);
//	const char *p=cmd.c_str();
//	if (!CreateProcess(NULL,(LPWSTR) p,NULL,NULL,false,show?0:CREATE_NO_WINDOW,NULL,
//		NULL,&si,&info)) return 0;
//	CloseHandle(info.hProcess);
//	CloseHandle(info.hThread);
//	return 1;
//}
// constructor --------------------------------------------------------------
naviMain::naviMain(){
	{
		SvrCycle = SvrBuffSize = 0;
		SolBuffSize = 1000;
		for (int i = 0; i < 8; i++) {
			StreamC[i] = Stream[i] = Format[i] = CmdEna[i][0] = CmdEna[i][1] = 0;
		}
		TimeSys = SolType = PlotType1 = PlotType2 = FreqType1 = FreqType2 = 0;
		PSol = PSolS = PSolE = Nsat[0] = Nsat[1] = 0;
		NMapPnt = 0;
		OpenPort = 0;
		Time = NULL;
		SolStat = Nvsat = NULL;
		SolCurrentStat = 0;
		SolRov = SolRef = Qr = VelRov = Age = Ratio = NULL;
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < MAXSAT; j++) {
				Sat[i][j] = Vsat[i][j] = 0;
				Az[i][j] = El[i][j] = 0.0;
				for (int k = 0; k < NFREQ; k++) Snr[i][j][k] = 0;
			}
		PrcOpt = prcopt_default;
		SolOpt = solopt_default;


		rtksvrinit(&rtksvr);
		strinit(&monistr);

		PanelStack = PanelMode = 0;
	}
}
// callback on form create --------------------------------------------------
//void naviMain::FormCreate(TObject *Sender)
//{
//	char *p,*argv[32],buff[1024],file[1024]="rtknavi.exe";
//	int argc=0;
//	AnsiString Caption;
//
//	trace(3,"FormCreate\n");
//
//	::GetModuleFileName(NULL,(LPWSTR) file,sizeof(file));
//	if (!(p=strrchr(file,'.'))) p=file+strlen(file);
//	strcpy(p,".ini");
//	IniFile=file;
//
//	InitSolBuff();
//
//	strinitcom();
//
//	strcpy(buff,(const char *) GetCommandLine());
//
//	for (p=buff;*p&&argc<32;p++) {
//		if (*p==' ') continue;
//		if (*p=='"') {
//			argv[argc++]=p+1;
//			if (!(p=strchr(p+1,'"'))) break;
//		}
//		else {
//			argv[argc++]=p;
//			if (!(p=strchr(p+1,' '))) break;
//		}
//		*p='\0';
//	}
//	for (int i=1;i<argc;i++) {
//		if (!strcmp(argv[i],"-i")&&i+1<argc) IniFile=argv[++i];
//	}
//	LoadOpt();
//
//	for (int i=1;i<argc;i++) {
//		if (!strcmp(argv[i],"-t")&&i+1<argc) Caption=argv[++i];
//	}
//
//
//	OpenMoniPort(MoniPort);
//}
// callback on form show ----------------------------------------------------
void naviMain::FormShow(TObject *Sender)
{
	trace(3,"FormShow\n");

	if (TLEFileF!="") {
		tle_read(TLEFileF.c_str(),&TLEData);
	}
	if (TLESatFileF!="") {
		tle_name_read(TLESatFileF.c_str(),&TLEData);
	}

	UpdateTimeSys();
	UpdateSolType();
	UpdatePos();
}
// callback on form close ---------------------------------------------------
void naviMain::FormClose(TObject *Sender)
{
	trace(3,"FormClose\n");

	if (OpenPort>0) {
		// send disconnect message
		strwrite(&monistr,(unsigned char *)MSG_DISCONN,strlen(MSG_DISCONN));

		strclose(&monistr);
	}
	SaveOpt();
}



// callback on button-start -------------------------------------------------
void naviMain::BtnStartClick(TObject *Sender)
{
	trace(3,"BtnStartClick\n");

	SvrStart();
}
// callback on button-stop --------------------------------------------------
void naviMain::BtnStopClick(TObject *Sender)
{
	trace(3,"BtnStopClick\n");

	SvrStop();
}

// callback on button-options -----------------------------------------------
void naviMain::BtnOptClick(TObject *Sender)
{
	int chgmoni=0;

	trace(3,"BtnOptClick\n");
	

	if (SolBuffSize) {
		//SolBuffSize=OptDialog->SolBuffSize;
		InitSolBuff();
		UpdateTime();
		UpdatePos();
	}

	// send disconnect message
	if (OpenPort>0) {
		strwrite(&monistr,(unsigned char *)MSG_DISCONN,strlen(MSG_DISCONN));

		strclose(&monistr);
	}
	// reopen monitor stream
	OpenMoniPort(MoniPort);
}

// confirm overwrite --------------------------------------------------------
int naviMain::ConfOverwrite(const char *path)
{
	AnsiString s;
	char schar[500];
	FILE *fp;
	int itype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP};
	int i;
	char buff1[1024],buff2[1024],*p;

	trace(3,"ConfOverwrite\n");

	strcpy(buff1,path);

	if ((p=strstr(buff1,"::"))) *p='\0';

	if (!(fp=fopen(buff1,"r"))) return 1; // file not exists
	fclose(fp);

	// check overwrite input files
	for (i=0;i<3;i++) {
		if (!StreamC[i]||itype[Stream[i]]!=STR_FILE) continue;

		strcpy(buff2,Paths[i][2].c_str());
		if ((p=strstr(buff2,"::"))) *p='\0';

		if (!strcmp(buff1,buff2)) {
			sprintf(schar,"invalid output %s",buff1);
			s=AnsiString(schar);
			return 0;
		}
	}

	return 1;
}
// callback on button-output-streams ----------------------------------------
void naviMain::BtnOutputStrClick(TObject *Sender)
{
	int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
	int i,str,update[2]={0};
	const char *path;

	trace(3,"BtnOutputStrClick\n");
	

	for (i=3;i<5;i++) {
		if (!update[i-3]) continue;

		rtksvrclosestr(&rtksvr,i);

		if (!StreamC[i]) continue;

		str=otype[Stream[i]];
		if      (str==STR_SERIAL)             path=Paths[i][0].c_str();
		else if (str==STR_FILE  )             path=Paths[i][2].c_str();
		else if (str==STR_FTP||str==STR_HTTP) path=Paths[i][3].c_str();
		else                                  path=Paths[i][1].c_str();
		if (str==STR_FILE&&!ConfOverwrite(path)) {
			StreamC[i]=0;
			continue;
		}
		SolOpt.posf=Format[i];
		rtksvropenstr(&rtksvr,i,str,path,&SolOpt);
	}
}
// callback on button-log-streams -------------------------------------------
void naviMain::BtnLogStrClick(TObject *Sender)
{
	int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
	int i,str,update[3]={0};
	const char *path;

	trace(3,"BtnLogStrClick\n");	

	for (i=5;i<8;i++) {
		if (!update[i-5]) continue;

		rtksvrclosestr(&rtksvr,i);

		if (!StreamC[i]) continue;

		str=otype[Stream[i]];
		if      (str==STR_SERIAL)             path=Paths[i][0].c_str();
		else if (str==STR_FILE  )             path=Paths[i][2].c_str();
		else if (str==STR_FTP||str==STR_HTTP) path=Paths[i][3].c_str();
		else                                  path=Paths[i][1].c_str();
		if (str==STR_FILE&&!ConfOverwrite(path)) {
			StreamC[i]=0;
			continue;
		}
		rtksvropenstr(&rtksvr,i,str,path,&SolOpt);
	}
}

// callback on button-plot-type-1 -------------------------------------------
void naviMain::BtnTimeSysClick(TObject *Sender)
{
	trace(3,"BtnTimeSysClick\n");

	if (++TimeSys>3) TimeSys=0;
	UpdateTimeSys();
}
// callback on button-solution-type -----------------------------------------
void naviMain::BtnSolTypeClick(TObject *Sender)
{
	trace(3,"BtnSolTypeClick\n");

	if (++SolType>4) SolType=0;
	UpdateSolType();
}

// callback on button frequency-type-1 --------------------------------------
void naviMain::BtnFreqType1Click(TObject *Sender)
{
	trace(3,"BtnFreqType1Click\n");

	if (++FreqType1>NFREQ+1) FreqType1=0;
	UpdateSolType();
}
// callback on button frequency-type-2 --------------------------------------
void naviMain::BtnFreqType2Click(TObject *Sender)
{
	trace(3,"BtnFreqType2Click\n");

	if (++FreqType2>NFREQ+1) FreqType2=0;
	UpdateSolType();
}

// callback on button-save --------------------------------------------------
void naviMain::BtnSaveClick(TObject *Sender)
{
	trace(3,"BtnSaveClick\n");

	SaveLog();
}

// start rtk server ---------------------------------------------------------
void naviMain::SvrStart(void)
{
	AnsiString s;
	char schar[500];

	filopt_t filopt = { "" };
	nav_t navs = { 0 };          /* navigation data  ADD by LI TO openses*/
	pcvs_t pcvss = { 0 };        /* receiver antenna parameters  ADD by LI TO openses */
	pcvs_t pcvsr = { 0 };        /* satellite antenna parameters   ADD by LI TO openses*/

	solopt_t solopt[2];
	double pos[3],nmeapos[3];
	int itype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP};
	int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
	int i,strs[MAXSTRRTK]={0},sat,ex,stropt[8]={0};
	char buff[1024],*p;
	char file[1024];
	const char *paths[8],*cmds[3]={0}, *rcvopts[3]={0},*type;
	FILE *fp;
	gtime_t time=timeget();
	pcvs_t pcvr={0},pcvs={0};
	pcv_t *pcv;

	trace(3,"SvrStart\n");	

	if (RovPosTypeF<=2) {
		PrcOpt.rovpos=0;
		PrcOpt.ru[0]=RovPos[0];
		PrcOpt.ru[1]=RovPos[1];
		PrcOpt.ru[2]=RovPos[2];
	}
	else {
		PrcOpt.rovpos=4;
		for (i=0;i<3;i++) PrcOpt.ru[i]=0.0;
	}
	if (RefPosTypeF<=2) {
		PrcOpt.refpos=0;
		PrcOpt.rb[0]=RefPos[0];
		PrcOpt.rb[1]=RefPos[1];
		PrcOpt.rb[2]=RefPos[2];
	}
	else {
		PrcOpt.refpos=4;
		for (i=0;i<3;i++) PrcOpt.rb[i]=0.0;
	}
	for (i=0;i<MAXSAT;i++) {
		PrcOpt.exsats[i]=0;
	}
	if (ExSats!="") { // excluded satellites
		strcpy(buff,ExSats.c_str());
		for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
			if (*p=='+') {ex=2; p++;} else ex=1;
			if (!(sat=satid2no(p))) continue;
			PrcOpt.exsats[sat-1]=ex;
		}
	}
	if ((RovAntPcvF||RefAntPcvF)&&!readpcv(AntPcvFileF.c_str(),&pcvr)) {
		sprintf(schar,"rcv ant file read error %s",AntPcvFileF.c_str());
		s=AnsiString(schar);
		return;
	}
	if (RovAntPcvF) {
		type=RovAntF.c_str();
		if ((pcv=searchpcv(0,type,time,&pcvr))) {
			PrcOpt.pcvr[0]=*pcv;
		}
		else {
			sprintf(schar,"no antenna pcv %s",type);
			s=AnsiString(schar);
		}
		for (i=0;i<3;i++) PrcOpt.antdel[0][i]=RovAntDel[i];
	}
	if (RefAntPcvF) {
		type=RefAntF.c_str();
		if ((pcv=searchpcv(0,type,time,&pcvr))) {
			PrcOpt.pcvr[1]=*pcv;
		}
		else {
			sprintf(schar,"no antenna pcv %s",type);
			s=AnsiString(schar);
		}
		for (i=0;i<3;i++) PrcOpt.antdel[1][i]=RefAntDel[i];
	}
	if (RovAntPcvF||RefAntPcvF) {
		free(pcvr.pcv);
	}
	if (PrcOpt.sateph==EPHOPT_PREC||PrcOpt.sateph==EPHOPT_SSRCOM) {
		if (!readpcv(SatPcvFileF.c_str(),&pcvs)) {
			sprintf(schar,"sat ant file read error %s",SatPcvFileF.c_str());
			s=AnsiString(schar);
			return;
		}
		for (i=0;i<MAXSAT;i++) {
			if (!(pcv=searchpcv(i+1,"",time,&pcvs))) continue;
			rtksvr.nav.pcvs[i]=*pcv;
		}
		free(pcvs.pcv);
	}
	if (BaselineC) {
		PrcOpt.baseline[0]=Baseline[0];
		PrcOpt.baseline[1]=Baseline[1];
	}
	else {
		PrcOpt.baseline[0]=0.0;
		PrcOpt.baseline[1]=0.0;
	}
	for (i=0;i<3;i++) strs[i]=StreamC[i]?itype[Stream[i]]:STR_NONE;
	for (i=3;i<5;i++) strs[i]=StreamC[i]?otype[Stream[i]]:STR_NONE;
	for (i=5;i<8;i++) strs[i]=StreamC[i]?otype[Stream[i]]:STR_NONE;
	for (i=0;i<8;i++) {
		if      (strs[i]==STR_NONE  ) paths[i]="";
		else if (strs[i]==STR_SERIAL) paths[i]=Paths[i][0].c_str();
		else if (strs[i]==STR_FILE  ) paths[i]=Paths[i][2].c_str();
		else if (strs[i]==STR_FTP||strs[i]==STR_HTTP) paths[i]=Paths[i][3].c_str();
		else paths[i]=Paths[i][1].c_str();
	}
	for (i=0;i<3;i++) {
		if (strs[i]==STR_SERIAL) {
			if (CmdEna[i][0]) cmds[i]=Cmds[i][0].c_str();
		}
		else if (strs[i]==STR_TCPCLI||strs[i]==STR_TCPSVR||
			strs[i]==STR_NTRIPCLI) {
				if (CmdEnaTcp[i][0]) cmds[i]=CmdsTcp[i][0].c_str();
		}
		rcvopts[i]=RcvOpt[i].c_str();
	}
	NmeaCycle=NmeaCycle<1000?1000:NmeaCycle;
	pos[0]=NmeaPos[0]*D2R;
	pos[1]=NmeaPos[1]*D2R;
	pos[2]=0.0;
	pos2ecef(pos,nmeapos);

	strsetdir(LocalDirectory.c_str());
	strsetproxy(ProxyAddr.c_str());

	for (i=3;i<8;i++) {
		if (strs[i]==STR_FILE&&!ConfOverwrite(paths[i])) return;
	}
	if (DebugTraceF>0) {
		traceopen(TRACEFILE);
		tracelevel(DebugTraceF);
	}
	if (DebugStatusF>0) {
		rtkopenstat(STATFILE,DebugStatusF);
	}
	if (SolOpt.geoid>0&&GeoidDataFileF!="") {
		opengeoid(SolOpt.geoid,GeoidDataFileF.c_str());
	}
	if (DCBFileF!="") {
		readdcb(DCBFileF.c_str(),&rtksvr.nav,NULL);
	}
	for (i=0;i<2;i++) {
		solopt[i]=SolOpt;
		solopt[i].posf=Format[i+3];
	}
	stropt[0]=TimeoutTime;
	stropt[1]=ReconTime;
	stropt[2]=1000;
	stropt[3]=SvrBuffSize;
	stropt[4]=FileSwapMargin;
	strsetopt(stropt);

	const char* tec_file = "mnt/sdcard/igrg.17i";   //    cccc
	strcpy(filopt.iono, tec_file);
//	openses(&PrcOpt, &solopt[0], &filopt, &rtksvr.nav, &pcvss, &pcvsr);

    const char *cmdsPeriodic[3] = {0};
    cmdsPeriodic[0] ="";
    cmdsPeriodic[1] ="";
    cmdsPeriodic[2] ="";
    char msg[128] = "";
    //sol_result.oldTime = 0;
    if (!rtksvrstart(&rtksvr,SvrCycle,SvrBuffSize,strs,(char**)paths,Format,NavSelect,
                     (char**)cmds,(char**)cmdsPeriodic,(char**)rcvopts,NmeaCycle,NmeaReq,nmeapos,&PrcOpt,solopt,
                     &monistr,msg))
    {
        traceclose();
        return;
    }

	// start rtk server
//	if (!rtksvrstart(&rtksvr,SvrCycle,SvrBuffSize,strs,(char**) paths,Format,NavSelect,
//		(char**) cmds,(char**)rcvopts,NmeaCycle,NmeaReq,nmeapos,&PrcOpt,solopt,
//		&monistr)) {
//			traceclose();
//			return;
//	}

	PSol=PSolS=PSolE=0;
	SolStat[0]=Nvsat[0]=0;
	for (i=0;i<3;i++) SolRov[i]=SolRef[i]=VelRov[i]=0.0;
	for (i=0;i<9;i++) Qr[i]=0.0;
	Age[0]=Ratio[0]=0.0;
	Nsat[0]=Nsat[1]=0;
	UpdatePos();


}
// strop rtk server ---------------------------------------------------------
void naviMain::SvrStop(void)
{
	const char *cmds[3]={0};
	int i,str;

	trace(3,"SvrStop\n");

	for (i=0;i<3;i++) {
		str=rtksvr.stream[i].type;

		if (str==STR_SERIAL) {
			if (CmdEna[i][1]) cmds[i]=Cmds[i][1].c_str();
		}
		else if (str==STR_TCPCLI||str==STR_TCPSVR||str==STR_NTRIPCLI) {
			if (CmdEnaTcp[i][1]) cmds[i]=CmdsTcp[i][1].c_str();
		}
	}
	rtksvrstop(&rtksvr,(char**)cmds);
	

	if (DebugTraceF>0) traceclose();
	if (DebugStatusF>0) rtkclosestat();
	if (OutputGeoidF>0&&GeoidDataFileF!="") closegeoid();
}
// callback on interval timer -----------------------------------------------
void naviMain::TimerTimer(TObject *Sender)
{
	static int n=0,inactive=0;
	sol_t *sol;
	int i,update=0;
	int Interval;
	Interval=1;////////////// Timer->Interval;

	trace(4,"TimerTimer\n");

	rtksvrlock(&rtksvr);

	for (i=0;i<rtksvr.nsol;i++) {
		sol=rtksvr.solbuf+i;
		UpdateLog(sol->stat,sol->time,sol->rr,sol->qr,rtksvr.rtk.rb,sol->ns,
			sol->age,sol->ratio);
		update=1;
	}
	rtksvr.nsol=0;
	SolCurrentStat=rtksvr.state?rtksvr.rtk.sol.stat:0;

	rtksvrunlock(&rtksvr);

	if (update) {
		UpdateTime();
		UpdatePos();
		inactive=0;
	}
	else {
		if (++inactive*Interval>TIMEOUT) SolCurrentStat=0;
	}

	UpdateStr();

	// keep alive for monitor port
	if (!(++n%(KACYCLE/Interval))&&OpenPort) {
		strwrite(&monistr,(unsigned char *) "\r",1);
	}
}

// update time-system -------------------------------------------------------
void naviMain::UpdateTimeSys(void)
{
	AnsiString label[]={"GPST","UTC","LT","GPST"};

	trace(3,"UpdateTimeSys\n");

	UpdateTime();
}
// update solution type -----------------------------------------------------
void naviMain::UpdateSolType(void)
{
	AnsiString label[]={
		"Lat/Lon/Height","Lat/Lon/Height","X/Y/Z-ECEF","E/N/U-Baseline",
		"Pitch/Yaw/Length-Baseline",""
	};
	trace(3,"UpdateSolType\n");

	UpdatePos();
}
// update log ---------------------------------------------------------------
void naviMain::UpdateLog(int stat, gtime_t time, double *rr,
									 float *qr, double *rb, int ns, double age, double ratio)
{
	int i;

	if (!stat) return;

	trace(4,"UpdateLog\n");

	SolStat[PSolE]=stat; Time[PSolE]=time; Nvsat[PSolE]=ns; Age[PSolE]=age;
	Ratio[PSolE]=ratio;
	for (i=0;i<3;i++) {
		SolRov[i+PSolE*3]=rr[i];
		SolRef[i+PSolE*3]=rb[i];
		VelRov[i+PSolE*3]=rr[i+3];
	}
	Qr[  PSolE*9]=qr[0];
	Qr[4+PSolE*9]=qr[1];
	Qr[8+PSolE*9]=qr[2];
	Qr[1+PSolE*9]=Qr[3+PSolE*9]=qr[3];
	Qr[5+PSolE*9]=Qr[7+PSolE*9]=qr[4];
	Qr[2+PSolE*9]=Qr[6+PSolE*9]=qr[5];

	PSol=PSolE;
	if (++PSolE>=SolBuffSize) PSolE=0;
	if (PSolE==PSolS&&++PSolS>=SolBuffSize) PSolS=0;
}

// update time --------------------------------------------------------------
void naviMain::UpdateTime(void)
{
	gtime_t time=Time[PSol];
	struct tm *t;
	double tow;
	int week;
	char tstr[64];

	trace(4,"UpdateTime\n");

	if      (TimeSys==0) time2str(time,tstr,1);
	else if (TimeSys==1) time2str(gpst2utc(time),tstr,1);
	else if (TimeSys==2) {
		time=gpst2utc(time);
		if (!(t=localtime(&time.time))) strcpy(tstr,"2000/01/01 00:00:00.0");
		else sprintf(tstr,"%04d/%02d/%02d %02d:%02d:%02d.%d",t->tm_year+1900,
			t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,
			(int)(time.sec*10));
	}
	else if (TimeSys==3) {
		tow=time2gpst(time,&week); sprintf(tstr,"week %04d %8.1f s",week,tow);
	}

}
// update solution display --------------------------------------------------
void naviMain::UpdatePos(void)
{
	AnsiString sol[]={"----","FIX","FLOAT","SBAS","DGPS","SINGLE","PPP"};
	char s[8][500];
	AnsiString s0;

	
	gtime_t time;
	double *rr=SolRov+PSol*3,*rb=SolRef+PSol*3,*qr=Qr+PSol*9,pos[3]={0},Qe[9]={0};
	double dms1[3]={0},dms2[3]={0},bl[3]={0},enu[3]={0},pitch=0.0,yaw=0.0,len;
	int i,stat=SolStat[PSol];

	trace(4,"UpdatePos\n");

	//Solution->Caption=sol[stat];
	
	if (norm(rr,3)>0.0&&norm(rb,3)>0.0) {
		for (i=0;i<3;i++) bl[i]=rr[i]-rb[i];
	}
	len=norm(bl,3);
	if (SolType==0) {
		if (norm(rr,3)>0.0) {
			ecef2pos(rr,pos); covenu(pos,qr,Qe);
			degtodms(pos[0]*R2D,dms1);
			degtodms(pos[1]*R2D,dms2);
			if (SolOpt.height==1) pos[2]-=geoidh(pos); /* geodetic */
		}
		s0=pos[0]<0?"S:":"N:";
		strcpy(s[0],s0.c_str());
		s0=pos[1]<0?"W:":"E:";
		strcpy(s[1],s0.c_str());
		s0=SolOpt.height==1?"H:":"He:";
		strcpy(s[2],s0.c_str());
		sprintf(s[3],"%.0f%c %02.0f' %07.4f\"",fabs(dms1[0]),CHARDEG,dms1[1],dms1[2]);
		sprintf(s[4],"%.0f%c %02.0f' %07.4f\"",fabs(dms2[0]),CHARDEG,dms2[1],dms2[2]);
		sprintf(s[5],"%.3f m",pos[2]);
		sprintf(s[6],"N:%6.3f E:%6.3f U:%6.3f m",SQRT(Qe[4]),SQRT(Qe[0]),SQRT(Qe[8]));
	}
	else if (SolType==1) {
		if (norm(rr,3)>0.0) {
			ecef2pos(rr,pos); covenu(pos,qr,Qe);
			if (SolOpt.height==1) pos[2]-=geoidh(pos); /* geodetic */
		}
		s0=pos[0]<0?"S:":"N:"; 
		strcpy(s[0],s0.c_str());
		s0=pos[1]<0?"W:":"E:";
		strcpy(s[1],s0.c_str());
		s0=SolOpt.height==1?"H:":"He:";
		strcpy(s[2],s0.c_str());
		sprintf(s[3],"%.8f %c",fabs(pos[0])*R2D,CHARDEG);
		sprintf(s[4],"%.8f %c",fabs(pos[1])*R2D,CHARDEG);
		sprintf(s[5],"%.3f m",pos[2]);
		sprintf(s[6],"E:%6.3f N:%6.3f U:%6.3f m",SQRT(Qe[0]),SQRT(Qe[4]),SQRT(Qe[8]));
	}
	else if (SolType==2) {
		s0="X:"; 
		strcpy(s[0],s0.c_str());
		s0="Y:"; 
		strcpy(s[1],s0.c_str());
		s0="Z:";
		strcpy(s[2],s0.c_str());
		
		sprintf(s[3],"%.3f m",rr[0]);
		sprintf(s[4],"%.3f m",rr[1]);
		sprintf(s[5],"%.3f m",rr[2]);
		sprintf(s[6],"X:%6.3f Y:%6.3f Z:%6.3f m",SQRT(qr[0]),SQRT(qr[4]),SQRT(qr[8]));
	}
	else if (SolType==3) {
		if (len>0.0) {
			ecef2pos(rb,pos); ecef2enu(pos,bl,enu); covenu(pos,qr,Qe);
		}
		s0="N:"; 
		strcpy(s[0],s0.c_str());
		s0="E:"; 
		strcpy(s[1],s0.c_str());
		s0="U:";
		strcpy(s[2],s0.c_str());
		sprintf(s[3],"%.3f m",enu[0]);
		sprintf(s[4],"%.3f m",enu[1]);
		sprintf(s[5],"%.3f m",enu[2]);
		sprintf(s[6],"E:%6.3f N:%6.3f U:%6.3f m",SQRT(Qe[0]),SQRT(Qe[4]),SQRT(Qe[8]));
	}
	else {
		if (len>0.0) {
			ecef2pos(rb,pos); ecef2enu(pos,bl,enu); covenu(pos,qr,Qe);
			pitch=asin(enu[2]/len);
			yaw=atan2(enu[0],enu[1]); if (yaw<0.0) yaw+=2.0*PI;
		}
		s0="P:"; 
		strcpy(s[0],s0.c_str());
		s0="Y:"; 
		strcpy(s[1],s0.c_str());
		s0="L:";
		strcpy(s[2],s0.c_str());

		sprintf(s[3],"%.3f %c",pitch*R2D,CHARDEG);
		sprintf(s[4],"%.3f %c",yaw*R2D,CHARDEG);
		sprintf(s[5],"%.3f m",len);
		sprintf(s[6],"E:%6.3f N:%6.3f U:%6.3f m",SQRT(Qe[0]),SQRT(Qe[4]),SQRT(Qe[8]));
	}
	sprintf(s[7], "Age:%4.1f s Ratio:%4.1f # Sat:%2d",Age[PSol],Ratio[PSol],Nvsat[PSol]);


}
// update stream status indicators ------------------------------------------
void naviMain::UpdateStr(void)
{
	int sstat[MAXSTRRTK]={0};
	char msg[MAXSTRMSG]="";

	trace(4,"UpdateStr\n");

	rtksvrsstat(&rtksvr,sstat,msg);	
}

// open monitor port --------------------------------------------------------
void naviMain::OpenMoniPort(int port)
{
	AnsiString s;
	char schar[500];
	int i;
	char path[64];

	if (port<=0) return;

	trace(3,"OpenMoniPort: port=%d\n",port);

	for (i=0;i<=MAXPORTOFF;i++) {

		sprintf(path,":%d",port+i);

		if (stropen(&monistr,STR_TCPSVR,STR_MODE_RW,path)) {
			strsettimeout(&monistr,TimeoutTime,ReconTime);
			if (i>0) { 
				sprintf(schar,"%s ver.%s (%d)",PRGNAME,VER_RTKLIB,i+1);
				s=AnsiString(schar);
			}
			OpenPort=MoniPort+i;
			return;
		}
	}
	sprintf(schar,"monitor port %d-%d open error",port,port+MAXPORTOFF);
	s=AnsiString(schar);
	OpenPort=0;
}
// initialize solution buffer -----------------------------------------------
void naviMain::InitSolBuff(void)
{
	double ep[]={2000,1,1,0,0,0};
	int i,j;

	trace(3,"InitSolBuff\n");

	delete [] Time;   delete [] SolStat; delete [] Nvsat;  delete [] SolRov;
	delete [] SolRef; delete [] Qr;      delete [] VelRov; delete [] Age;
	delete [] Ratio;

	if (SolBuffSize<=0) SolBuffSize=1;
	Time   =new gtime_t[SolBuffSize];
	SolStat=new int[SolBuffSize];
	Nvsat  =new int[SolBuffSize];
	SolRov =new double[SolBuffSize*3];
	SolRef =new double[SolBuffSize*3];
	VelRov =new double[SolBuffSize*3];
	Qr     =new double[SolBuffSize*9];
	Age    =new double[SolBuffSize];
	Ratio  =new double[SolBuffSize];
	PSol=PSolS=PSolE=0;
	for (i=0;i<SolBuffSize;i++) {
		Time[i]=epoch2time(ep);
		SolStat[i]=Nvsat[i]=0;
		for (j=0;j<3;j++) SolRov[j+i*3]=SolRef[j+i*3]=VelRov[j+i*3]=0.0;
		for (j=0;j<9;j++) Qr[j+i*9]=0.0;
		Age[i]=Ratio[i]=0.0;
	}
	
}
// save log file ------------------------------------------------------------
void naviMain::SaveLog(void)
{
	AnsiString FileName="SaveLog.txt";
	AnsiString SaveDialog_FileName=FileName;
	FILE *fp;
	int posf[]={SOLF_LLH,SOLF_LLH,SOLF_XYZ,SOLF_ENU,SOLF_ENU,SOLF_LLH};
	solopt_t opt;
	double  ep[6],pos[3];
	char file[1024];

	trace(3,"SaveLog\n");

	time2epoch(timeget(),ep);
	sprintf(file,"rtk_%04.0f%02.0f%02.0f%02.0f%02.0f%02.0f.txt",
		ep[0],ep[1],ep[2],ep[3],ep[4],ep[5]);
	
	
	if (!(fp=fopen(SaveDialog_FileName.c_str(),"wt"))) {
		AnsiString Caption="log file open error";
		return;
	}
	opt=SolOpt;
	opt.posf=posf[SolType];
	if (SolOpt.outhead) {
		fprintf(fp,"%% program   : %s ver.%s\n",PRGNAME,VER_RTKLIB);
		if (PrcOpt.mode==PMODE_DGPS||PrcOpt.mode==PMODE_KINEMA||
			PrcOpt.mode==PMODE_STATIC) {
				ecef2pos(PrcOpt.rb,pos);
				fprintf(fp,"%% ref pos   :%13.9f %14.9f %10.4f\n",pos[0]*R2D,
					pos[1]*R2D,pos[2]);
		}
		fprintf(fp,"%%\n");
	}
	outsolhead(fp,&opt);
	fclose(fp);
}

// load option from ini file ------------------------------------------------
void naviMain::LoadOpt(void)
{	
	AnsiString s;
	int i,j,strno[]={0,1,6,2,3,4,5,7};
	char *p;

	trace(3,"LoadOpt\n");

	//stream
	//no=0;
	StreamC[0]=1;
	Stream [0]=3;//3ntrip  1tcp
	Format [0]=1;
	Paths[0][0]="";
	Paths[0][1]="Example:Configs@www.igs-ip.net:80/FFMJ1:";//Example:Configs@www.igs-ip.net:80/FFMJ1:
	Paths[0][2]="::x1";
	Paths[0][3]="";

	//no=1;
	StreamC[1]=1;
	Stream [1]=3;
	Format [1]=1;
	Paths[1][0]="";
	Paths[1][1]="Example:Configs@products.igs-ip.net:2101/RTCM3EPH:";//"Example:Configs@products.igs-ip.net:2101/RTCM3EPH:"
	Paths[1][2]="::x1";
	Paths[1][3]="";

	//no=6;
	StreamC[2]=1;
	Stream [2]=3;
	Format [2]=1;
	Paths[2][0]="";
	Paths[2][1]="lq:12345@124.207.244.214:2101/CLK93:";
	Paths[2][2]="::x1";
	Paths[2][3]="";

	//no=2;
	StreamC[3]=1;//old StreamC[3]=1 ;不使用文件;add by lj 17-4-26;
	Stream [3]=4;
	Format [3]=0;
	Paths[3][0]="";
	Paths[3][1]="";
	Paths[3][2]="mnt/sdcard/rover.pos";
	Paths[3][3]="";

	//no=3;
	StreamC[4]=1;//1表示三目运算符真
	Stream [4]=4;
	Format [4]=0;
	Paths[4][0]="";
	Paths[4][1]="";
	Paths[4][2]="mnt/sdcard/roverRtcm1.dat";///storage/sdcard0/
	Paths[4][3]="";

	//no=4;
	StreamC[5]=1;
	Stream [5]=4;
	Format [5]=0;
	Paths[5][0]="";
	Paths[5][1]="";
	Paths[5][2]="mnt/sdcard/roverRtcm2.dat";
	Paths[5][3]="";

	//no=5;
	StreamC[6]=1;
	Stream [6]=4;
	Format [6]=0;
	Paths[6][0]="";
	Paths[6][1]="";
	Paths[6][2]="mnt/sdcard/roverRtcm3.dat";
	Paths[6][3]="";

	//no=7;
	StreamC[7]=0;//old StreamC[7]=1 ;不使用文件;add by lj 17-4-26;
	Stream [7]=4;
	Format [7]=0;
	Paths[7][0]="";
	Paths[7][1]="";
	Paths[7][2]="";
	Paths[7][3]="";
	
	RcvOpt [0]="";
	RcvOpt [1]="";
	RcvOpt [2]="";

	//serial
	Cmds  [0][0]="";
	CmdEna[0][0]=0;
	Cmds  [0][1]="";
	CmdEna[0][1]=0;

	Cmds  [1][0]="";
	CmdEna[1][0]=0;
	Cmds  [1][1]="";
	CmdEna[1][1]=0;

	Cmds  [2][0]="";
	CmdEna[2][0]=0;
	Cmds  [2][1]="";
	CmdEna[2][1]=0;	
	
	for (i=0;i<3;i++) for (j=0;j<2;j++) {		
		for (p=(char*) Cmds[i][j].c_str();*p;p++) {
			if ((p=strstr(p,"@@"))) strncpy(p,"\r\n",2); else break;
		}
	}

	//tcpip
	CmdsTcp  [0][0]="";
	CmdEnaTcp[0][0]=0;
	CmdsTcp  [0][1]="";
	CmdEnaTcp[0][1]=0;

	CmdsTcp  [1][0]="";
	CmdEnaTcp[1][0]=0;
	CmdsTcp  [1][1]="";
	CmdEnaTcp[1][1]=0;

	CmdsTcp  [2][0]="";
	CmdEnaTcp[2][0]=0;
	CmdsTcp  [2][1]="";
	CmdEnaTcp[2][1]=0;

	for (i=0;i<3;i++) for (j=0;j<2;j++) {		
		for (p=(char*) CmdsTcp[i][j].c_str();*p;p++) {
			if ((p=strstr(p,"@@"))) strncpy(p,"\r\n",2); else break;
		}
	}

	PrcOpt.mode     =7;
	PrcOpt.nf       =1;
	PrcOpt.elmin    =0.261799387799149;//15.0*D2R
	PrcOpt.snrmask.ena[0]=0;
	PrcOpt.snrmask.ena[1]=0;
	for (i=0;i<NFREQ;i++) for (j=0;j<9;j++) {
		PrcOpt.snrmask.mask[i][j]=0;
	}
	PrcOpt.dynamics =0;
	PrcOpt.tidecorr =0;
	PrcOpt.modear   =1;
	PrcOpt.glomodear=0;
	PrcOpt.bdsmodear=0;
	PrcOpt.maxout   =5;
	PrcOpt.minlock  =0;
	PrcOpt.minfix   =10;
	PrcOpt.ionoopt  =IONOOPT_OFF;
	PrcOpt.tropopt  =1;
	PrcOpt.sateph   =EPHOPT_SSRAPC; //3
	PrcOpt.niter    =1;
	PrcOpt.eratio[0]=100;
	PrcOpt.eratio[1]=100;
	PrcOpt.err[1]   =0.003;
	PrcOpt.err[2]   =0.003;
	PrcOpt.err[3]   =0;
	PrcOpt.err[4]   =1;
	PrcOpt.prn[0]   =1E-4;
	PrcOpt.prn[1]   =1E-3;
	PrcOpt.prn[2]   =1E-4;
	PrcOpt.prn[3]   =10;
	PrcOpt.prn[4]   =10;
	PrcOpt.sclkstab =5E-12;
	PrcOpt.thresar[0]=3;
	PrcOpt.elmaskar =0.0;
	PrcOpt.elmaskhold=0.0;
	PrcOpt.thresslip=0.05;
	PrcOpt.maxtdiff =30.0;
	PrcOpt.maxgdop  =30.0;
	PrcOpt.maxinno  =30.0;
	PrcOpt.syncsol  = 0;
	ExSats          ="";
	PrcOpt.navsys   =SYS_GPS;
	PrcOpt.posopt[0]=0;
	PrcOpt.posopt[1]=0;
	PrcOpt.posopt[2]=0;
	PrcOpt.posopt[3]=0;
	PrcOpt.posopt[4]=0;

	BaselineC       =  0;
	Baseline[0]     =0.0;
	Baseline[1]     =0.0;

	SolOpt.posf     =0;
	SolOpt.times    =0;
	SolOpt.timef    =1;
	SolOpt.timeu    =3;
	SolOpt.degf     =0;
	s="";
	strcpy(SolOpt.sep,s.c_str());
	SolOpt.outhead  =0;
	SolOpt.outopt   =0;
	SolOpt.datum    =0;
	SolOpt.height   =0;
	SolOpt.geoid    =0;
	SolOpt.nmeaintv[0]=0.0;
	SolOpt.nmeaintv[1]=0.0;

	//seting
	DebugStatusF    =  0;
	DebugTraceF     =  0;

	RovPosTypeF     =  0;
	RefPosTypeF     =  0;
	RovAntPcvF      =  0;
	RefAntPcvF      =  0;
	RovAntF         = "";
	RefAntF         = "";
	SatPcvFileF     = "";
	AntPcvFileF     = "";
	StaPosFileF     = "";
	GeoidDataFileF  = "";
	DCBFileF        = "";
	EOPFileF        = "";
	TLEFileF        = "";
	TLESatFileF     = "";
	LocalDirectory  = "";

	SvrCycle        =    10;
	TimeoutTime     = 10000;
	ReconTime       = 10000;
	NmeaCycle       =  5000;
	SvrBuffSize     = 32768;
	SolBuffSize     =  1000;
	SavedSol        =   100;
	NavSelect       =     0;
	PrcOpt.sbassatsel=     0;
	DgpsCorr        =     0;
	SbasCorr        =     0;

	NmeaReq         =   0;
	InTimeTag       =   0;
	InTimeSpeed     ="x1";
	InTimeStart     = "0";
	OutTimeTag      =   0;
	OutAppend       =   0;
	OutSwapInterval =  "";
	LogTimeTag      =   0;
	LogAppend       =   0;
	LogSwapInterval = "" ;
	NmeaPos[0]      = 0.0;
	NmeaPos[1]      = 0.0;
	FileSwapMargin  =  30;

	TimeSys         =2  ;
	SolType         =0  ;
	PlotType1       =0  ;
	PlotType2       =0  ;
	PanelMode       =0  ;
	ProxyAddr       ="" ;
	MoniPort        =DEFAULTPORT ;
	PanelStack      =0 ;

	for (i=0;i<3;i++) {
		RovAntDel[i]=0.0;
		RefAntDel[i]=0.0;
		RovPos   [i]=0.0;
		RefPos   [i]=0.0;
	}
	RovPos   [0]=3.9325782161665E-12;
	RefPos   [0]=3.9325782161665E-12;
	RovPos   [2]=21384.6857451801;
	RefPos   [2]=21384.6857451801;

	//[tcpopt]
	for (i=0;i<10;i++) {
		History[i]="";
	}
	History[0]="products.igs-ip.net";
	History[1]="www.igs-ip.net";
	for (i=0;i<10;i++) {
		MntpHist[i]="";
	}
	MntpHist[0]="RTCM3EPH";
	MntpHist[1]="IGS03";

	//[mapopt]
	NMapPnt        =0;
	for (i=0;i<NMapPnt;i++) {
		PntName[i]="";
		AnsiString pos="0,0,0";
		PntPos[i][0]=PntPos[i][1]=PntPos[i][2]=0.0;
		sscanf(pos.c_str(),"%lf,%lf,%lf",PntPos[i],PntPos[i]+1,PntPos[i]+2);
	}

}
// save option to ini file --------------------------------------------------
void naviMain::SaveOpt(void)
{
	
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------



