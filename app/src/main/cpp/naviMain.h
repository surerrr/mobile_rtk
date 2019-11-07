//---------------------------------------------------------------------------
#ifndef naviMain_H
#define naviMain_H
//---------------------------------------------------------------------------

#include "rtklib.h"

#define MAXSCALE	18
#define MAXMAPPNT	10

#include <string.h>
#include <sstream>
#include <pthread.h>
using namespace std;

typedef string AnsiString;
#define TObject  void
//rtksvr_t rtksvr;                        // rtk server struct
//stream_t monistr;
//---------------------------------------------------------------------------
class naviMain 
{
	
private:
	tle_t TLEData;

	void  UpdateLog    (int stat, gtime_t time, double *rr, float *qr,
		double *rb, int ns, double age, double ratio);

	void  UpdateTimeSys(void);
	void  UpdateSolType(void);
	void  UpdateTime   (void);
	void  UpdatePos    (void);
	void  UpdateStr    (void);
	int   ConfOverwrite(const char *path);
	
	void  SaveLog      (void);

	
	void  SaveOpt      (void);
	int   ExecCmd      (AnsiString cmd, int show);

public:
	AnsiString IniFile;

	int PanelStack,PanelMode;
	int SvrCycle,SvrBuffSize,Scale,SolBuffSize,NavSelect,SavedSol;
	int NmeaReq,NmeaCycle,InTimeTag,OutTimeTag,OutAppend,LogTimeTag,LogAppend;
	int TimeoutTime,ReconTime,SbasCorr,DgpsCorr,TideCorr,FileSwapMargin;
	int Stream[MAXSTRRTK],StreamC[MAXSTRRTK],Format[MAXSTRRTK];
	int CmdEna[3][2],CmdEnaTcp[3][2];
	int TimeSys,SolType,PlotType1,FreqType1,PlotType2,FreqType2;
	int MoniPort,OpenPort;

	int PSol,PSolS,PSolE,Nsat[2],SolCurrentStat;
	int Sat[2][MAXSAT],Snr[2][MAXSAT][NFREQ],Vsat[2][MAXSAT];
	double Az[2][MAXSAT],El[2][MAXSAT];
	gtime_t *Time;
	int *SolStat,*Nvsat;
	double *SolRov,*SolRef,*Qr,*VelRov,*Age,*Ratio;
	AnsiString Paths[MAXSTRRTK][4],Cmds[3][2],CmdsTcp[3][2];
	AnsiString InTimeStart,InTimeSpeed,ExSats;
	AnsiString RcvOpt[3],ProxyAddr;
	AnsiString OutSwapInterval,LogSwapInterval;
	prcopt_t PrcOpt;
	solopt_t SolOpt;

	int DebugTraceF,DebugStatusF,OutputGeoidF,BaselineC;
	int RovPosTypeF,RefPosTypeF,RovAntPcvF,RefAntPcvF;
	AnsiString RovAntF,RefAntF,SatPcvFileF,AntPcvFileF;
	double RovAntDel[3],RefAntDel[3],RovPos[3],RefPos[3],NmeaPos[2];
	double Baseline[2];
	AnsiString History[10],MntpHist[10];

	AnsiString GeoidDataFileF,StaPosFileF,DCBFileF,EOPFileF,TLEFileF;
	AnsiString TLESatFileF,LocalDirectory,PntName[MAXMAPPNT];
	double PntPos[MAXMAPPNT][3];
	int NMapPnt;



	 naviMain();

	void  LoadOpt      (void);
	void  FormCreate(TObject *Sender);
	void  FormShow(TObject *Sender);
	void  FormClose(TObject *Sender);
	void  BtnStartClick(TObject *Sender);
	void  BtnStopClick(TObject *Sender);
	void  BtnOptClick(TObject *Sender);
	
	void  BtnOutputStrClick(TObject *Sender);
	void  BtnLogStrClick(TObject *Sender);
	void  BtnTimeSysClick(TObject *Sender);
	void  BtnSolTypeClick(TObject *Sender);
	void  BtnFreqType1Click(TObject *Sender);
	void  BtnFreqType2Click(TObject *Sender);
	void  BtnSaveClick(TObject *Sender);	
	void  TimerTimer(TObject *Sender);	

	void  OpenMoniPort (int port);
	void  InitSolBuff  (void);
	void  SvrStart     (void);
	void  SvrStop      (void);
	
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#endif
