//
// Created by LiuYi on 2018/9/1.
//

#pragma once
#ifndef PHONEOBSDATA_H
#define PHONEOBSDATA_H


//#include "rtklib.h"
class obsdata_t { ;
    int Gpsprn_t[32];
    int Bdsprn_t[35];

    double GpsC1_t[32];     //伪距
    double GpsC2_t[32];
    double BdsC1_t[35];
    double BdsC2_t[35];

    double GpsL1_t[32];     //相位
    double GpsL2_t[32];
    double BdsL1_t[35];
    double BdsL2_t[35];

    double GpsD1_t[32];     //多普勒
    double GpsD2_t[32];
    double BdsD1_t[35];
    double BdsD2_t[35];

    int GPSTweek;                        //时间
    double GPSTsecond;

    int BDSTweek;                        //时间
    double BDSTsecond;

    int gpsflag;
    int bdsflag;
    int phoneflag;
};

#endif //RTKLIBANDROID_PHONEOBSDATA_H
