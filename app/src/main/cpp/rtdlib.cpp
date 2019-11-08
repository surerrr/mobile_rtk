//
// Created by LiuYi on 2018/7/19.
//

#include "rtdlib.h"
#include "rtklib.h"
//#include <iostream>
//using namespace std;
//-----------------------------------------------------------------------------
#define MAXSCALE	18
#define MAXMAPPNT	10


int pthread_create(pthread_t *thread,const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);
//--------------------传出结果-----------------------------------------------//
//int SyncFlag;
//long double oldTime;
//
//gtime_t result_time;           /* time (GPST) */
//double result_rr[6];          /* position/velocity (m|m/s) */
///* {x,y,z,vx,vy,vz} or {e,n,u,ve,vn,vu} */
//float  result_qr[6];          /* position variance/covariance (m^2) */
///* {c_xx,c_yy,c_zz,c_xy,c_yz,c_zx} or */
///* {c_ee,c_nn,c_uu,c_en,c_nu,c_ue} */
//float  result_qv[6];          /* velocity variance/covariance (m^2/s^2) */
//double result_dtr[6];         /* receiver clock bias to time systems (s) */
//unsigned char results_type;   /* type (0:xyz-ecef,1:enu-baseline) */
//unsigned char result_stat;   /* solution status (SOLQ_???) */
//unsigned char result_ns;     /* number of valid satellites */
//float result_age;             /* age of differential (s) */
//float result_ratio;           /* AR ratio factor for valiation */
//float result_thres;           /* AR ratio threshold for valiation */
//chuandi
result_t sol_result;
long double oldTime = 0;        //上一历元时间
int SyncFlag = 0;
string ResultFileName;
//---------------------------------------------------------------------------//

rtksvr_t rtksvr;                        // rtk server struct
stream_t monistr;                       // monitor stream

int PanelStack,PanelMode;
int SvrCycle,SvrBuffSize,Scale,SolBuffSize,NavSelect,SavedSol;
int NmeaReq,NmeaCycle,InTimeTag,OutTimeTag,OutAppend,LogTimeTag,LogAppend;
int TimeoutTime,ReconTime,SbasCorr,DgpsCorr,TideCorr,FileSwapMargin;
int Stream[MAXSTRRTK],StreamC[MAXSTRRTK],Format[MAXSTRRTK];
int CmdEna[3][2],CmdEnaTcp[3][2];
int TimeSys,SolType,PlotType1,FreqType1,PlotType2,FreqType2;
int TrkType1,TrkType2,TrkScale1,TrkScale2,BLMode1,BLMode2;
int MoniPort,OpenPort;

int PSol,PSolS,PSolE,Nsat[2],SolCurrentStat;
int Sat[2][MAXSAT],Snr[2][MAXSAT][NFREQ],Vsat[2][MAXSAT];
double Az[2][MAXSAT],El[2][MAXSAT];
gtime_t *Time;
int *SolStat,*Nvsat;
double *SolRov,*SolRef,*Qr,*VelRov,*Age,*Ratio;
double TrkOri[3];

string Paths[MAXSTRRTK][4],Cmds[3][2],CmdsTcp[3][2];
string InTimeStart,InTimeSpeed,ExSats;
string RcvOpt[3],ProxyAddr;
string OutSwapInterval,LogSwapInterval;

prcopt_t PrcOpt;
solopt_t SolOpt;

//QFont PosFont;

int DebugTraceF,DebugStatusF,OutputGeoidF,BaselineC;
int RovPosTypeF,RefPosTypeF,RovAntPcvF,RefAntPcvF;
string RovAntF,RefAntF,SatPcvFileF,AntPcvFileF;
double RovAntDel[3],RefAntDel[3],RovPos[3],RefPos[3],NmeaPos[2];
double Baseline[2];

string History[10],MntpHist[10];

//QTimer Timer;

string GeoidDataFileF,StaPosFileF,DCBFileF,EOPFileF,TLEFileF;
string TLESatFileF,LocalDirectory,PntName[MAXMAPPNT];

double PntPos[MAXMAPPNT][3];
int NMapPnt;

string MarkerName, MarkerComment;
//---------------------------------------------------------------------------

extern int showmsg(char *format, ...)
{
    va_list arg;
    va_start(arg,format); vfprintf(stderr,format,arg); va_end(arg);
    fprintf(stderr,"\r");
    return 0;
}
extern void settspan(gtime_t ts, gtime_t te) {}
extern void settime(gtime_t time) {}


//int pthread_create(pthread_t *thread,const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);

//--------------------------初始化-add by liuyi-------------------------------

int rtdrun(string ip_t[8],string type_t[8],string resultfilename_t)
{
    string ip[8],type[8];
    memset(&ip,0x00,sizeof(ip));
    memset(&type,0x00,sizeof(type));
    for(int i = 0;i < 8;i++) {
        ip[i] = ip_t[i];
        type[i] = type_t[i];
    }
    ResultFileName = resultfilename_t;

    //===============================================
    string Paths[MAXSTRRTK][4],Cmds[3][2],CmdsTcp[3][2];
    int   strs[8];
    memset(&Paths,0x00,sizeof(Paths));
    memset(&Cmds,0x00,sizeof(Cmds));
    memset(&CmdsTcp,0x00,sizeof(CmdsTcp));
    memset(strs,0x00,sizeof(strs));

    strs[0] = atoi(type[0].c_str());//7
    Paths[0][1] = ip[0];
    //Paths[0][1] = ":@124.205.216.26:35946/:";
    //Paths[0][1] = ":@10.26.0.100:39151/:";
    strs[1] = atoi(type[1].c_str());
    Paths[1][1] = ip[1];

    //Paths[1][1] = ":@124.205.216.26:35946/:";
    //Paths[1][1] = ":@10.26.0.100:39151/:";
//    strs[2] = atoi(type[2].c_str());
//    Paths[2][1] = ip[2];
//
    strs[2] =7;
    Paths[2][1]="Example:Configs@products.igs-ip.net:2101/RTCM3EPH"; //外接星历写死


    //Paths[2][1] = ":@124.205.216.26:2103/:";
    //"cgscasm004:a97c05d@rtd.ntrip.qxwz.com:8002:RTCM32_GGB"
    // "用户名:密码@IP:PORT:/挂在点"

    strs[3] = STR_FILE;
    Paths[3][2] = ResultFileName;
    strs[4] = 0;
    Paths[4][1] = "";
    strs[5] = 0;
    Paths[5][1] = "";
    strs[6] = 0;
    Paths[6][1] = "";
    strs[7] = 0;
    Paths[7][1] = "";

    solopt_t solopt[2];
    double pos[3],nmeapos[3]={0,0,0};
    int itype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP};
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
    char buff[1024],*p;
    gtime_t time=timeget();
    pcvs_t pcvr,pcvs;
    pcv_t *pcv;
    int stropt[8]={0};


    //+++++++++++++++++++++++++++++初始化部分++++++++++++++++++++++++++++++++++
    for (int i=0;i<MAXSTRRTK;i++) {
        StreamC[i]=Stream[i]=Format[i]=0;
    }
    for (int i=0;i<3;i++) {
        CmdEna[i][0]=CmdEna[i][1]=0;
    }

    TimeSys=SolType=PlotType1=PlotType2=FreqType1=FreqType2=0;
    TrkType1=TrkType2=0;
    TrkScale1=TrkScale2=5;
    BLMode1=BLMode2=0;
    PSol=PSolS=PSolE=Nsat[0]=Nsat[1]=0;
    NMapPnt=0;
    OpenPort=0;
    Time = NULL;
    SolStat=Nvsat = NULL;
    SolCurrentStat=0;
    SolRov=SolRef=Qr=VelRov=Age=Ratio=NULL;

    const char *paths[8],*cmds[3],*rcvopts[3];
    char cmd[3],rcvopt[3];;
    memset(paths,0x00,sizeof(paths));
    memset(cmds,0x00,sizeof(cmds));
    memset(rcvopts,0x00,sizeof(rcvopts));
    memset(&cmd,0x00,sizeof(&cmd));
    memset(&rcvopt,0x00,sizeof(&rcvopt));

    cmds[0]=&cmd[0];
    cmds[1]=&cmd[1];
    cmds[2]=&cmd[2];
    rcvopts[0]=&rcvopt[0];
    rcvopts[1]=&rcvopt[1];
    rcvopts[2]=&rcvopt[2];

    SvrCycle = SvrBuffSize = 0;
    int formats[8]={1,1,1,0,0,0,0,0};


    monistr.tick_i = 5171841;
    rtksvrinit(&rtksvr);
    strinit(&monistr);



    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //SolBuffSize = 1000;
    SvrBuffSize = 32768;
    SvrCycle = 1000;//控制计算次数SvrCycle/1000=t by pers
    //NmeaCycle = 1000;//改成了5000 12.23

    NmeaCycle = 5000;
    NmeaReq = 0;
    TimeoutTime = 20000;
    ReconTime   = 10000;

    solopt[0] = solopt_default;
    solopt[1] = solopt_default;
    solopt[0].posf = SOLF_ENU;//SOLF_LLH

    for (int i=0;i<2;i++){
        for (int j=0;j<MAXSAT;j++)
        {
            Sat[i][j]=Vsat[i][j]=0;
            Az[i][j]=El[i][j]=0.0;
            for (int k=0;k<NFREQ;k++) Snr[i][j][k]=0;
        }
    }

    PrcOpt = prcopt_default;//在rtkcmn.c文件查看默认参数设置
    SolOpt = solopt_default;
    //PrcOpt.posopt[4] = 1;
//    PrcOpt.mode = PMODE_DGPS;

    //修改默认参数
   PrcOpt.mode =  PMODE_STATIC;
//    PrcOpt.mode = 0;

 //   PrcOpt.navsys = SYS_CMP|SYS_GPS|SYS_GLO|SYS_GAL;
    PrcOpt.navsys = SYS_CMP|SYS_GPS|SYS_GLO;
//    PrcOpt.navsys = SYS_GPS;
    PrcOpt.nf = 1;
    PrcOpt.elmin = 15*D2R;
//    PrcOpt.dynamics = 1;

    PrcOpt.snrmask.ena[0]=0;
    PrcOpt.snrmask.ena[1]=0;

    for (int i=0;i<NFREQ;i++) for (int j=0;j<9;j++)
    {
            PrcOpt.snrmask.mask[i][j]=0;
        }
    PrcOpt.dynamics =0; // 静态

    PrcOpt.tidecorr =0;

  //  PrcOpt.modear   =3;

    PrcOpt.modear   =1;   //(0:off,1:continuous,2:instantaneous,3:fix and hold,4:ppp-ar)


    PrcOpt.glomodear=0;
    PrcOpt.bdsmodear=0;
    PrcOpt.maxout   =5;
    PrcOpt.minlock  =0;
    PrcOpt.minfix   =10;

    PrcOpt.ionoopt  =1;//广播星历
    PrcOpt.tropopt  =1;//SAAS模型
    PrcOpt.sateph   =0; //brdc = 0  ssrapc=3
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
    PrcOpt.posopt[0]=0;
    PrcOpt.posopt[1]=0;
    PrcOpt.posopt[2]=0;
    PrcOpt.posopt[3]=0;
    PrcOpt.posopt[4]=0;

    PrcOpt.thresar[0] = 100; //ratio



    rtksvr.tick = 284415;
    rtksvr.cycle = SvrCycle;
    rtksvr.nmeacycle = NmeaCycle;
    rtksvr.nmeareq = NmeaReq;
    rtksvr.nmeapos[0] = nmeapos[0];
    rtksvr.nmeapos[1] = nmeapos[1];
    rtksvr.nmeapos[2] = nmeapos[2];
    rtksvr.buffsize = SvrBuffSize;
    rtksvr.format[0] = formats[0];
    rtksvr.format[1] = formats[1];
    rtksvr.format[2] = formats[2];
//    rtksvr.format[3] = formats[3];
//    rtksvr.format[4] = formats[4];
//    rtksvr.format[5] = formats[5];
//    rtksvr.format[6] = formats[6];
//    rtksvr.format[7] = formats[7];
    rtksvr.solopt[0] = solopt[0];
    rtksvr.solopt[1] = solopt[1];
    rtksvr.nb[0] = 128;

    monistr.type=3;
    monistr.mode=3;
    monistr.state=1;
    rtksvr.moni=&monistr;
    //monistr.tick_i = 308503;

    memset(&pcvr,0,sizeof(pcvs_t));
    memset(&pcvs,0,sizeof(pcvs_t));

//    RovPos   [0]=3.9325782161665E-12;
//    RefPos   [0]=3.9325782161665E-12;
//    RovPos   [1]=0.0;
//    RefPos   [1]=0.0;
//    RovPos   [2]=21384.6857451801;
//    RefPos   [2]=21384.6857451801;


    RovPosTypeF = 0;
    RefPosTypeF = 0;                            //参考站坐标初始化为RTCM流

    PrcOpt.rovpos=0;

    if (RovPosTypeF<=2)
    {                       // LLH,XYZ
        PrcOpt.rovpos=0;
        PrcOpt.ru[0]=RovPos[0];
        PrcOpt.ru[1]=RovPos[1];
        PrcOpt.ru[2]=RovPos[2];
    }
    else
        {                                      // RTCM position
        PrcOpt.rovpos=4;
        for (int i=0;i<3;i++)
            PrcOpt.ru[i]=0.0;
    }
    if (RefPosTypeF<=2) {                       // LLH,XYZ
        PrcOpt.refpos=0;


//        PrcOpt.rb[0] =-2197958.2180;
//        PrcOpt.rb[1] = 5180835.9775;
//        PrcOpt.rb[2] = 2991563.9414;

        PrcOpt.rb[0] = -2197956.5227;
        PrcOpt.rb[1] = 5180832.0759;
        PrcOpt.rb[2] = 2991562.8309;
//        PrcOpt.rb[0] = -2198037.884000;
//        PrcOpt.rb[1] = 5179124.776000;
//        PrcOpt.rb[2] = 2994385.687000;  // cors

        //-2197958.2180        5180835.9775        2991563.9414

    }
    else if (RefPosTypeF==3) {                  // RTCM position
        PrcOpt.refpos=4;
        for (int i=0;i<3;i++)
            PrcOpt.rb[i]=0.0;
    }else {                                     // average of single position
        PrcOpt.refpos=1;
        for (int i=0;i<3;i++)
            PrcOpt.rb[i]=0.0;
    }

    /*------------------------------------------------------------*/
    for (int i = 0; i < 8; ++i) Format[i] = formats[i];


    for (int i=0;i<8;i++) {
        if      (strs[i]==STR_NONE  ) paths[i]="";
        else if (strs[i]==STR_SERIAL) paths[i]=Paths[i][0].c_str();
        else if (strs[i]==STR_FILE  ) paths[i]=Paths[i][2].c_str();
        else if (strs[i]==STR_FTP||strs[i]==STR_HTTP) paths[i]=Paths[i][3].c_str();
        else paths[i]=Paths[i][1].c_str();
    }

    //===============================================

    stropt[0] = TimeoutTime;
    stropt[1] = ReconTime;
    stropt[2] = 1000;
    stropt[3] = SvrBuffSize;
    stropt[4] = FileSwapMargin;
    strsetopt(stropt);


    char *cmdsPeriodic[3] = {0};
    cmdsPeriodic[0] = cmdsPeriodic[1] = cmdsPeriodic[2] = "";
    char msg[128] = "";
    //sol_result.oldTime = 0;
    if (!rtksvrstart(&rtksvr,SvrCycle,SvrBuffSize,strs,(char**)paths,Format,NavSelect,
                     (char**)cmds,cmdsPeriodic,(char**)rcvopts,NmeaCycle,NmeaReq,nmeapos,&PrcOpt,solopt,
                     &monistr,msg))
    {
        traceclose();
        for (int i=0;i<8;i++) delete[] paths[i];
        for (int i=0;i<3;i++) delete[] rcvopts[i];
        for (int i=0;i<3;i++) delete[] cmds[i];

        return 0;
    }

//    SyncFlag = 1;
    /**
     * 传递结果线程，resultPos为函数（固定格式）
     */
    pthread_t t1;
    int err = pthread_create(&t1,NULL,resultPos,NULL);


//    for (int i=0;i<8;i++) delete[] paths[i];
//    for (int i=0;i<3;i++) delete[] rcvopts[i];
//    for (int i=0;i<3;i++) delete[] cmds[i];
//    PSol=PSolS=PSolE=0;
//    SolStat[0]=Nvsat[0]=0;
//    for (int i=0;i<3;i++) SolRov[i]=SolRef[i]=VelRov[i]=0.0;
//    for (int i=0;i<9;i++) Qr[i]=0.0;
//    Age[0]=Ratio[0]=0.0;
//    Nsat[0]=Nsat[1]=0;
    //rtksvrstop(&rtksvr,cmds);


//    rtksvrlock(&rtksvr);
//    matcpy(rr,svr.rtk.sol.rr,3,1);
//    rtksvrsstat(&rtksvr,sstat,msg);
//    rtksvrunlock(&rtksvr);


    return 1;
}

void * resultPos(void * arg)
{
    long double tempTime = 0;
    while (1)
    {
        //PassingResults(sol_t  sol,sol_t  *sol_result)
        if (rtksvr.state == 1) {
            tempTime = rtksvr.rtk.sol.time.time + rtksvr.rtk.sol.time.sec;
            if (oldTime != tempTime) {
                passingResults();
                oldTime = tempTime;
                //SyncFlag = 0 ;
            }
        }
        sleepms(100);
    }
    return NULL;
}
//------------------------------传递结果---------------------------------------

int passingResults()
{
//    if(SyncFlag == 0) {
    sol_result.result_time.time = rtksvr.rtk.sol.time.time;           /* time (GPST) */
    sol_result.result_time.sec = rtksvr.rtk.sol.time.sec;

    double gpsSec = 0;
    int gpsWeek = 0;
    double ep[6] = {0};
    gpsSec = time2gpst(sol_result.result_time, &gpsWeek);
    time2epoch(sol_result.result_time,ep);
    sol_result.gpsWeek = gpsWeek;
    sol_result.gpsSec = gpsSec;
    sol_result.results_type = rtksvr.rtk.sol.type;                     //系统变量冲突
    sol_result.result_stat = rtksvr.rtk.sol.stat;
    sol_result.result_ns = rtksvr.rtk.sol.ns;
    sol_result.result_age = rtksvr.rtk.sol.age;
    sol_result.result_ratio = rtksvr.rtk.sol.ratio;
    sol_result.result_thres = rtksvr.rtk.sol.thres;

    for (int i = 0; i < 6; i++)
    {
        sol_result.result_rr[i] = rtksvr.rtk.sol.rr[i];
        sol_result.result_qr[i] = rtksvr.rtk.sol.qr[i];
        sol_result.result_qv[i] = rtksvr.rtk.sol.qv[i];
        sol_result.result_dtr[i] = rtksvr.rtk.sol.dtr[i];
        sol_result.satnum[i] =  rtksvr.rtk.sol.satnum[i];
    }
    for (int i = 0; i < 4; i++) {
        sol_result.dop[i] =  rtksvr.rtk.sol.dop[i];
    }
//        SyncFlag = 1;
//   }

    double Pos[3] = {0};
    memset(Pos,0x00, sizeof(Pos));
    ecef2pos(sol_result.result_rr,Pos);

//    if(sol_result.result_stat==5||sol_result.result_stat==4) {
//
//        FILE *fpresult;
//        char outfile[128];
//        memcpy(outfile,ResultFileName.c_str(), 128);
//        fpresult=fopen(outfile,"a+");
//        if(sol_result.result_stat==4)fprintf(fpresult,"D ");
//        if(sol_result.result_stat==5)fprintf(fpresult,"S ");
//        fprintf(fpresult,"%4.4d %6.6d %4.4d %2.2d %2.2d %2.2d %2.2d %2.2d ",
//                gpsWeek,(int)gpsSec,(int)ep[0],(int)ep[1],(int)ep[2],(int)ep[3],(int)ep[4],(int)ep[5]);
//        fprintf(fpresult,"%13.3f %13.3f %13.3f %13.8f %13.8f %8.3f \n",
//                sol_result.result_rr[0],sol_result.result_rr[1],sol_result.result_rr[2],
//                Pos[0]*R2D,Pos[1]*R2D,Pos[2]);
//        fclose(fpresult);
//
//    }

    return 1;
}

