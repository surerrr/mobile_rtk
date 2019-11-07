#include <jni.h>
//#include <string>
//#include <iostream>
#include "rtdlib.h"
#include "rtklib.h"
//#include "phoneObsData.h"

//#include "phoneObsData.h"
//导入日志头文件
#include <android/log.h>
//修改日志tag中的值
#define LOG_TAG "logfromc"
//日志显示的等级
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

char* jstringToChar(JNIEnv* env, jstring jstr);
jstring charTojstring(JNIEnv* env, const char* pat);

jstring charTojstring(JNIEnv* env, const char* pat) {
    //定义java String类 strClass
    jclass strClass = (env)->FindClass("Ljava/lang/String;");
    //获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    //建立byte数组
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    //将char* 转换为byte数组
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*) pat);
    // 设置String, 保存语言类型,用于byte数组转换至String时的参数
    jstring encoding = (env)->NewStringUTF("GB2312");
    //将byte数组转换为java String,并输出
    return (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);
}

char* jstringToChar(JNIEnv* env, jstring jstr) {
    char* rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char*) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

extern result_t sol_result;
double StationPos[3] = {0};
extern "C"
JNIEXPORT jint JNICALL
Java_com_kubolab_gnss_gnssloggerTestSPP_Logger3Fragment_resultFromJNI(JNIEnv *env, jobject instance,
                                                    jdoubleArray result_, jdoubleArray ep_,
                                                    jdoubleArray gpst_,
                                                    jdoubleArray velocity_,
                                                    jdoubleArray clk_,
                                                    jintArray satprn_,
                                                    jdoubleArray dop_   ) {
    jdouble *result = env->GetDoubleArrayElements(result_, NULL);
    jdouble *ep = env->GetDoubleArrayElements(ep_, NULL);
    jdouble *gpst = env->GetDoubleArrayElements(gpst_, NULL);
    jdouble *velocity = env->GetDoubleArrayElements(velocity_, NULL);
    jdouble *clk = env->GetDoubleArrayElements(clk_, NULL);
    jint    *satprn = env->GetIntArrayElements(satprn_, NULL);
    jdouble *dop = env->GetDoubleArrayElements(dop_, NULL);
    // TODO

    for (int i = 0; i < 3; i++) {
        result[i] = sol_result.result_rr[i];
    }
    for (int i = 0; i < 4; i++) {
        dop[i] = sol_result.dop[i];
    }
    for (int i = 0; i < 6; i++) {
        satprn[i] = sol_result.satnum[i];
    }
    ep[0] = sol_result.gpsWeek;
    ep[1] = sol_result.gpsSec;
//    double StationPos[3] = {0};
    StationPos[3] = {0};
    memset(StationPos,0x00, sizeof(StationPos));
    ecef2pos(result,StationPos);
    result[3] = StationPos[0]*R2D;
    result[4] = StationPos[1]*R2D;
    result[5] = StationPos[2];


    env->ReleaseDoubleArrayElements(result_, result, 0);
    env->ReleaseDoubleArrayElements(ep_, ep, 0);
    env->ReleaseDoubleArrayElements(gpst_, gpst, 0);
    env->ReleaseDoubleArrayElements(velocity_, velocity, 0);
    env->ReleaseDoubleArrayElements(clk_, clk, 0);
    env->ReleaseIntArrayElements(satprn_, satprn, 0);
    env->ReleaseDoubleArrayElements(dop_, dop, 0);
    return sol_result.result_stat;
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_kubolab_gnss_gnssloggerTestSPP_Logger3Fragment_passingPathJNI(JNIEnv *env, jobject instance,
                                                              jobjectArray ip, jobjectArray port,
                                                              jstring resultfilename_) {
    const char *resultfilename = env->GetStringUTFChars(resultfilename_, 0);

    // TODO
    std::string IP[8],PORT[8],RESULTFILENAME;
    int PathNum = 0;
    memset(&IP,0x00,sizeof(IP));memset(&PORT,0x00,sizeof(PORT));
    //数组的长度
    jsize iplen = (*env).GetArrayLength(ip);
    jsize portlen = (*env).GetArrayLength(port);
    //先获取对象，再根据长度将对象分割
    for (int i = 0,j = 0; (i < iplen)&&(j <portlen); i++,j++) {
        jstring jstrsip = (jstring)(*env).GetObjectArrayElement(ip,i);
        jstring jstrsport = (jstring)(*env).GetObjectArrayElement(port,j);
        if(jstrsip!=NULL&&jstrsport!=NULL){
            char* chardataIP = jstringToChar(env, jstrsip);
            char* chardataPORT = jstringToChar(env, jstrsport);
            IP[i] = chardataIP;
            PORT[j] = chardataPORT;
            PathNum++;
        }
    }
    RESULTFILENAME = resultfilename;
    int success = 0;
    std::string type = "Null";
    if(PathNum > 0) {
        success = rtdrun(IP,PORT,RESULTFILENAME);                          //rtd接口
    }
    if(success) {type = "Connecting";}
    else {type = "Faild";}


    env->ReleaseStringUTFChars(resultfilename_, resultfilename);

    return env->NewStringUTF(type.c_str());
    //return env->NewStringUTF(returnValue);
}



extern obsdata_t ObsData;
extern "C"
JNIEXPORT jint JNICALL
Java_com_kubolab_gnss_gnssloggerTestSPP_Logger3Fragment_passingobsDataJNI(JNIEnv *env, jobject instance,
                                                                    jintArray gpsweek_,
                                                                    jdoubleArray gpssecond_,
                                                                    jintArray bdsweek_,
                                                                    jdoubleArray bdssecond_,
                                                                    jint gpssvnum, jdouble bdssvnum,
                                                                    jintArray gpsprn_,
                                                                    jintArray bdsprn_, jint gpsflag,
                                                                    jint bdsflag,
                                                                    jdoubleArray gpsC1_,
                                                                    jdoubleArray gpsL1_,
                                                                    jdoubleArray gpsD1_,
                                                                    jdoubleArray gpsC2_,
                                                                    jdoubleArray gpsL2_,
                                                                    jdoubleArray gpsD2_,
                                                                    jdoubleArray bdsC1_,
                                                                    jdoubleArray bdsL1_,
                                                                    jdoubleArray bdsD1_,
                                                                    jdoubleArray bdsC2_,
                                                                    jdoubleArray bdsL2_,
                                                                    jdoubleArray bdsD2_) {
    jint *gpsweek = env->GetIntArrayElements(gpsweek_, NULL);
    jdouble *gpssecond = env->GetDoubleArrayElements(gpssecond_, NULL);
    jint *bdsweek = env->GetIntArrayElements(bdsweek_, NULL);
    jdouble *bdssecond = env->GetDoubleArrayElements(bdssecond_, NULL);
    jint *gpsprn = env->GetIntArrayElements(gpsprn_, NULL);
    jint *bdsprn = env->GetIntArrayElements(bdsprn_, NULL);
    jdouble *gpsC1 = env->GetDoubleArrayElements(gpsC1_, NULL);
    jdouble *gpsL1 = env->GetDoubleArrayElements(gpsL1_, NULL);
    jdouble *gpsD1 = env->GetDoubleArrayElements(gpsD1_, NULL);
    jdouble *gpsC2 = env->GetDoubleArrayElements(gpsC2_, NULL);
    jdouble *gpsL2 = env->GetDoubleArrayElements(gpsL2_, NULL);
    jdouble *gpsD2 = env->GetDoubleArrayElements(gpsD2_, NULL);
    jdouble *bdsC1 = env->GetDoubleArrayElements(bdsC1_, NULL);
    jdouble *bdsL1 = env->GetDoubleArrayElements(bdsL1_, NULL);
    jdouble *bdsD1 = env->GetDoubleArrayElements(bdsD1_, NULL);
    jdouble *bdsC2 = env->GetDoubleArrayElements(bdsC2_, NULL);
    jdouble *bdsL2 = env->GetDoubleArrayElements(bdsL2_, NULL);
    jdouble *bdsD2 = env->GetDoubleArrayElements(bdsD2_, NULL);

    // TODO
    ObsData.gpsSvNum = gpssvnum;
    ObsData.bdsSvNum = bdssvnum;
//    ObsData.gloSvNum = glosvnum;
//    ObsData.galSvNum = galsvnum;
    for(int i = 0;i<ObsData.gpsSvNum;i++)
    {
        ObsData.GPSTweek[i] = gpsweek[i];
        ObsData.GPSTsecond[i] = gpssecond[i];
        ObsData.Gpsprn_t[i] = gpsprn[i];
        ObsData.GpsC1_t[i] = gpsC1[i];
        ObsData.GpsC2_t[i] = gpsC2[i];
//        ObsData.GpsC3_t[i] = gpsC3[i];
        ObsData.GpsL1_t[i] = gpsL1[i];
        ObsData.GpsL2_t[i] = gpsL2[i];
//        ObsData.GpsL3_t[i] = gpsL3[i];
        ObsData.GpsD1_t[i] = gpsD1[i];
//        ObsData.GpsD3_t[i] = gpsD3[i];
    }
    for(int i = 0;i<ObsData.bdsSvNum;i++)
    {
        ObsData.BDSTweek[i] = bdsweek[i];
        ObsData.BDSTsecond[i] = bdssecond[i];
        ObsData.Bdsprn_t[i] = bdsprn[i];
        ObsData.BdsC1_t[i] = bdsC1[i];
        ObsData.BdsC2_t[i] = bdsC2[i];
//        ObsData.BdsC3_t[i] = bdsC3[i];
        ObsData.BdsL1_t[i] = bdsL1[i];
        ObsData.BdsL2_t[i] = bdsL2[i];
//        ObsData.BdsL3_t[i] = bdsL3[i];
        ObsData.BdsD1_t[i] = bdsD1[i];
        ObsData.BdsD2_t[i] = bdsD2[i];
//        ObsData.BdsD3_t[i] = bdsD3[i];

    }
/*    for(int i = 0;i<ObsData.gloSvNum;i++) {

        ObsData.Gloprn_t[i] = gloprn[i];
        ObsData.GLOTweek[i] = gloweek[i];
        ObsData.GLOTsecond[i] = glosecond[i];
        ObsData.GloC1_t[i] = gloC1[i];
        ObsData.GloC2_t[i] = gloC2[i];
        ObsData.GloC3_t[i] = gloC3[i];
        ObsData.GloL1_t[i] = gloL1[i];
        ObsData.GloL2_t[i] = gloL2[i];
        ObsData.GloL3_t[i] = gloL3[i];
        ObsData.GloD1_t[i] = gloD1[i];
        ObsData.GloD2_t[i] = gloD2[i];
        ObsData.GloD3_t[i] = gloD3[i];
    }
    for(int i = 0;i<ObsData.galSvNum;i++) {
        ObsData.Galprn_t[i] = galprn[i];
        ObsData.GALTweek[i] = galweek[i];
        ObsData.GALTsecond[i] = galsecond[i];
        ObsData.GalC1_t[i] = galC1[i];
        ObsData.GalC2_t[i] = galC2[i];
        ObsData.GalC3_t[i] = galC3[i];
        ObsData.GalL1_t[i] = galL1[i];
        ObsData.GalL2_t[i] = galL2[i];
        ObsData.GalL3_t[i] = galL3[i];
        ObsData.GalD1_t[i] = galD1[i];
        ObsData.GalD2_t[i] = galD2[i];
        ObsData.GalD3_t[i] = galD3[i];

    }*/
    env->ReleaseIntArrayElements(gpsweek_, gpsweek, 0);
    env->ReleaseDoubleArrayElements(gpssecond_, gpssecond, 0);
    env->ReleaseIntArrayElements(bdsweek_, bdsweek, 0);
    env->ReleaseDoubleArrayElements(bdssecond_, bdssecond, 0);
    env->ReleaseIntArrayElements(gpsprn_, gpsprn, 0);
    env->ReleaseIntArrayElements(bdsprn_, bdsprn, 0);
    env->ReleaseDoubleArrayElements(gpsC1_, gpsC1, 0);
    env->ReleaseDoubleArrayElements(gpsL1_, gpsL1, 0);
    env->ReleaseDoubleArrayElements(gpsD1_, gpsD1, 0);
    env->ReleaseDoubleArrayElements(gpsC2_, gpsC2, 0);
    env->ReleaseDoubleArrayElements(gpsL2_, gpsL2, 0);
    env->ReleaseDoubleArrayElements(gpsD2_, gpsD2, 0);
    env->ReleaseDoubleArrayElements(bdsC1_, bdsC1, 0);
    env->ReleaseDoubleArrayElements(bdsL1_, bdsL1, 0);
    env->ReleaseDoubleArrayElements(bdsD1_, bdsD1, 0);
    env->ReleaseDoubleArrayElements(bdsC2_, bdsC2, 0);
    env->ReleaseDoubleArrayElements(bdsL2_, bdsL2, 0);
    env->ReleaseDoubleArrayElements(bdsD2_, bdsD2, 0);

    return 1;
}extern "C"
JNIEXPORT jint JNICALL
Java_com_kubolab_gnss_gnssloggerTestSPP_Logger3Fragment_passingobsDataJNI1(JNIEnv *env,
                                                                           jobject instance,
                                                                           jintArray gpsweek_,
                                                                           jdoubleArray gpssecond_,
                                                                           jintArray bdsweek_,
                                                                           jdoubleArray bdssecond_,
                                                                           jint gpssvnum,
                                                                           jdouble bdssvnum,
                                                                           jintArray gpsprn_,
                                                                           jintArray bdsprn_,
                                                                           jint gpsflag,
                                                                           jint bdsflag,
                                                                           jdoubleArray gpsC1_,
                                                                           jdoubleArray gpsL1_,
                                                                           jdoubleArray gpsD1_,
                                                                           jdoubleArray gpsS1_,
                                                                           jdoubleArray gpsL2_,
                                                                           jdoubleArray gpsD2_,
                                                                           jdoubleArray gpsS2_,
                                                                           jdoubleArray bdsL1_,
                                                                           jdoubleArray bdsD1_,
                                                                           jdoubleArray bdsS1_,
                                                                           jdoubleArray bdsL2_,
                                                                           jdoubleArray bdsD2_,
                                                                           jdoubleArray bdsS2_) {
    jint *gpsweek = env->GetIntArrayElements(gpsweek_, NULL);
    jdouble *gpssecond = env->GetDoubleArrayElements(gpssecond_, NULL);
    jint *bdsweek = env->GetIntArrayElements(bdsweek_, NULL);
    jdouble *bdssecond = env->GetDoubleArrayElements(bdssecond_, NULL);
    jint *gpsprn = env->GetIntArrayElements(gpsprn_, NULL);
    jint *bdsprn = env->GetIntArrayElements(bdsprn_, NULL);
    jdouble *gpsC1 = env->GetDoubleArrayElements(gpsC1_, NULL);
    jdouble *gpsL1 = env->GetDoubleArrayElements(gpsL1_, NULL);
    jdouble *gpsD1 = env->GetDoubleArrayElements(gpsD1_, NULL);
    jdouble *gpsS1 = env->GetDoubleArrayElements(gpsS1_, NULL);
    jdouble *gpsL2 = env->GetDoubleArrayElements(gpsL2_, NULL);
    jdouble *gpsD2 = env->GetDoubleArrayElements(gpsD2_, NULL);
    jdouble *gpsS2 = env->GetDoubleArrayElements(gpsS2_, NULL);
    jdouble *bdsL1 = env->GetDoubleArrayElements(bdsL1_, NULL);
    jdouble *bdsD1 = env->GetDoubleArrayElements(bdsD1_, NULL);
    jdouble *bdsS1 = env->GetDoubleArrayElements(bdsS1_, NULL);
    jdouble *bdsL2 = env->GetDoubleArrayElements(bdsL2_, NULL);
    jdouble *bdsD2 = env->GetDoubleArrayElements(bdsD2_, NULL);
    jdouble *bdsS2 = env->GetDoubleArrayElements(bdsS2_, NULL);

    // TODO

    env->ReleaseIntArrayElements(gpsweek_, gpsweek, 0);
    env->ReleaseDoubleArrayElements(gpssecond_, gpssecond, 0);
    env->ReleaseIntArrayElements(bdsweek_, bdsweek, 0);
    env->ReleaseDoubleArrayElements(bdssecond_, bdssecond, 0);
    env->ReleaseIntArrayElements(gpsprn_, gpsprn, 0);
    env->ReleaseIntArrayElements(bdsprn_, bdsprn, 0);
    env->ReleaseDoubleArrayElements(gpsC1_, gpsC1, 0);
    env->ReleaseDoubleArrayElements(gpsL1_, gpsL1, 0);
    env->ReleaseDoubleArrayElements(gpsD1_, gpsD1, 0);
    env->ReleaseDoubleArrayElements(gpsS1_, gpsS1, 0);
    env->ReleaseDoubleArrayElements(gpsL2_, gpsL2, 0);
    env->ReleaseDoubleArrayElements(gpsD2_, gpsD2, 0);
    env->ReleaseDoubleArrayElements(gpsS2_, gpsS2, 0);
    env->ReleaseDoubleArrayElements(bdsL1_, bdsL1, 0);
    env->ReleaseDoubleArrayElements(bdsD1_, bdsD1, 0);
    env->ReleaseDoubleArrayElements(bdsS1_, bdsS1, 0);
    env->ReleaseDoubleArrayElements(bdsL2_, bdsL2, 0);
    env->ReleaseDoubleArrayElements(bdsD2_, bdsD2, 0);
    env->ReleaseDoubleArrayElements(bdsS2_, bdsS2, 0);
}