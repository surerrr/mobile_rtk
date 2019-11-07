//
// Created by LiuYi on 2018/7/19.
//

#pragma once

#ifndef RTKLIBANDROID_RTDLIB_H
#define RTKLIBANDROID_RTDLIB_H

#include <stdio.h>
#include "rtklib.h"
#include <string>
#include <pthread.h>
#include <errno.h>
//#include <iostream>
//#include <sstream>


class result_t
{
public:
    int SyncFlag;
    gtime_t result_time;           /* time (GPST) */
    double result_rr[6];          /* position/velocity (m|m/s) */
/* {x,y,z,vx,vy,vz} or {e,n,u,ve,vn,vu} */
    float  result_qr[6];          /* position variance/covariance (m^2) */
/* {c_xx,c_yy,c_zz,c_xy,c_yz,c_zx} or */
/* {c_ee,c_nn,c_uu,c_en,c_nu,c_ue} */
    float  result_qv[6];          /* velocity variance/covariance (m^2/s^2) */
    double result_dtr[6];         /* receiver clock bias to time systems (s) */
    unsigned char results_type;   /* type (0:xyz-ecef,1:enu-baseline) */
    unsigned char result_stat;   /* solution status (SOLQ_???) */
    unsigned char result_ns;     /* number of valid satellites */
    float result_age;             /* age of differential (s) */
    float result_ratio;           /* AR ratio factor for valiation */
    float result_thres;           /* AR ratio threshold for valiation */
    int gpsWeek;
    double gpsSec;
    double dop[4];
    int satnum[6];
};

using namespace std;

//add by liuyi 20180820
//sol_t  sol_results;


int rtdrun(string ip_t[8],string port_t[8],string resultfilename);
int passingResults();
void * resultPos(void * arg);

#endif //RTKLIBANDROID_RTDLIB_H
