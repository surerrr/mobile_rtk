package com.kubolab.gnss.gnssloggerTestSPP;

public class StaticGnssData {

    public static int[] Gpsprn = new int[32];
    public static int[] Bdsprn = new int[35];
    public static int[] Gloprn = new int[35];         // 后连个系统的卫星数都是按照35算的
    public static int[] Galprn = new int[35];

    public static double[] GpsC1 = new double[32];     //伪距
    public static double[] GpsC2 = new double[32];
    public static double[] BdsC1 = new double[35];
    public static double[] BdsC2 = new double[35];

    public static double[] GloC1 = new double[35];
    public static double[] GloC2 = new double[35];
    public static double[] GalC1 = new double[35];
    public static double[] GalC2 = new double[35];

    public static double[] GpsL1 = new double[32];     //相位
    public static double[] GpsL2 = new double[32];
    public static double[] BdsL1 = new double[35];
    public static double[] BdsL2 = new double[35];

    public static double[] GloL1 = new double[35];
    public static double[] GloL2 = new double[35];
    public static double[] GalL1 = new double[35];
    public static double[] GalL2 = new double[35];

    public static double[] GpsD1 = new double[32];     //多普勒
    public static double[] GpsD2 = new double[32];
    public static double[] BdsD1 = new double[35];
    public static double[] BdsD2 = new double[35];

    public static double[] GloD1 = new double[35];
    public static double[] GloD2 = new double[35];
    public static double[] GalD1 = new double[35];
    public static double[] GalD2 = new double[35];


    public static double[] GpsS1 = new double[32];     //信噪比
    public static double[] GpsS2 = new double[32];
    public static double[] BdsS1 = new double[35];
    public static double[] BdsS2 = new double[35];

    public static double[] GloS1 = new double[35];
    public static double[] GloS2 = new double[35];
    public static double[] GalS1 = new double[35];
    public static double[] GalS2 = new double[35];

    public static int[] GPSTweek = new int[32];                       //时间
    public static double[] GPSTsecond = new double[32];

    public static int[] BDSTweek = new int[32];                       //时间
    public static double[] BDSTsecond = new double[32];

    public static int[] GloTweek = new int[35];                       //时间
    public static double[] GloTsecond = new double[35];

    public static int[] GalTweek = new int[35];                       //时间
    public static double[] GalTsecond = new double[35];

    public static int gpsflag;
    public static int bdsflag;
    public static int flag;
    public static int bdssvnum;
    public static int gpssvnum;


    public static int syns;
    public static int Glosvnum;
    public static int Galsvnum;



    public static void initData()
    {
        gpsflag = 0;
        bdsflag = 0;
        flag = 0;


        for (int i = 0;i<32;i++)
        {
            GPSTweek[i] = 0;
            GPSTsecond[i] = 0;
            Gpsprn[i] = 0;
            GpsC1[i] = 0;
            GpsC2[i] = 0;
            GpsL1[i] = 0;
            GpsL2[i] = 0;
            GpsD1[i] =0;
            GpsD2[i] =0;
        }
        for (int i = 0;i<35;i++)
        {
            BDSTweek[i] = 0;
            BDSTsecond[i] = 0;
            Bdsprn[i] = 0;
            BdsC1[i] = 0;
            BdsC2[i] = 0;
            BdsL1[i] = 0;
            BdsL2[i] = 0;
            BdsD1[i] =0;
            BdsD2[i] =0;
        }

    }


}
