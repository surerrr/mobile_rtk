package com.kubolab.gnss.gnssloggerTestSPP;

import android.app.Fragment;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import java.io.File;
import java.text.DecimalFormat;
import java.util.Calendar;

public class Logger3Fragment extends Fragment {

    private FileLogger mFileLogger;
    private UiLogger mUiLogger;
    private TextView mSensorLogView;
    private TextView mSensorRawAccView;
    private TextView mSensorRawPressView;
    private TextView mSensorRawMagView;
    //private TextView mSensorRawMagUncalibratedView;
    private TextView mSensorRawGyroView;
    //private TextView mSensorRawGyroUncalibratedView;
    //----------------------------------------------------add by yanglijun
    private EditText obsIP;
    private EditText obsType;
    private EditText brdcIP;
    private EditText brdcType;
    private EditText vrsIP;
    private EditText vrsType;
    private Button OK;
    private Button runButton;

    private String Tcpip = "4";
    private String Ntrip = "7";

    //mainactivity中的定义
    //变量
    private TextView xView;                     //坐标 X Y Z
    private TextView yView;
    private TextView zView;
    private TextView bView;                     //经纬度 B L H
    private TextView lView;
    private TextView hView;
    private TextView pdopView;
    private TextView satnumView;
    private TextView statusView;

    private int type = 0;

    private double[] result = new double[6];
    private double[] ep = new double[6];
    private double[] gpst = new double[2];
    private double[] velocity = new double[6];
    private double[] clk = new double[4];
    private int oldgpst = 0;

    private static final int msgkey1 = 1;       //

    private double[] dop = new double[4];
    private int[] satnum = new int[6];

    DecimalFormat df1 = new DecimalFormat("###.000");
    DecimalFormat df2 = new DecimalFormat("###.000000");
    DecimalFormat df3 = new DecimalFormat("00");
    DecimalFormat df4 = new DecimalFormat("0000");
    DecimalFormat df5 = new DecimalFormat("###");

    static {
        System.loadLibrary("rtd");
    }

    //-----------------------------------------------------------------------
    private final Logger3Fragment.UIFragment3Component mUiComponent = new Logger3Fragment.UIFragment3Component();

    public void setUILogger(UiLogger value) {
        mUiLogger = value;
    }

    public void setFileLogger(FileLogger value) {
        mFileLogger = value;
    }
    @Override
    public void onAttach(Context context){
        super.onAttach(context);
        UiLogger currentUiLogger = mUiLogger;
        if (currentUiLogger != null) {
            currentUiLogger.setUiFragment3Component(mUiComponent);
        }
    }

    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_log3, container, false /* attachToRoot */);
    }

    public void onViewCreated(View view, Bundle savedInstanceState){

//----------------------------add by yanglijun
        //setContentView(R.layout.fragment_log3);
        obsIP   = (EditText) view.findViewById(R.id.obsip);
//        obsType = (EditText)findViewById(R.id.obstype);
        vrsIP   = (EditText)view.findViewById(R.id.vrsip);
        vrsType = (EditText)view.findViewById(R.id.vrstype);
        brdcIP   = (EditText)view.findViewById(R.id.brdcip);
//        brdcType = (EditText)findViewById(R.id.brdctype);


        OK    = (Button)view.findViewById(R.id.OK);

         //传递编辑框中的IP和端口
        OK.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
//                StaticVariable.obsType = obsType.getText().toString();
                StaticVariable.vrsType = vrsType.getText().toString();
//                StaticVariable.brdcType = brdcType.getText().toString();

//                if(StaticVariable.obsType.equals( Tcpip )){
//                    StaticVariable.obsIP = ":@" + obsIP.getText().toString() + "/:";
//                }else if(StaticVariable.obsType.equals( Ntrip )) {
//                    StaticVariable.obsIP = obsIP.getText().toString();
//                }

                if(StaticVariable.vrsType.equals( Tcpip )){
                    StaticVariable.vrsIP  = ":@" + vrsIP.getText().toString()+"/:";
                }else if(StaticVariable.obsType.equals( Ntrip )) {
                    StaticVariable.obsIP = vrsIP.getText().toString();
                }

//                if(StaticVariable.brdcType.equals( Tcpip )) {
//                    StaticVariable.brdcIP = brdcIP.getText().toString();
//                }else if(StaticVariable.obsType.equals( Ntrip )) {
//                    StaticVariable.obsIP = brdcIP.getText().toString();
//                }
            }

        });


//-----------------------------------------------------------------------------------------------------------
        //传值完后开始计算
   //     startGetData();

        //创建结果存放位置
        String mPath = "/storage/self/primary/ALaas";
        File file = new File(mPath);
        if (!file.exists()) {
            file.mkdir();
        }

        //开启显示线程(显示计算后获得的值)
        ViewThread displayInfo = new ViewThread();
        displayInfo.start();


        xView = (TextView) view.findViewById(R.id.xview);
        yView = (TextView) view.findViewById(R.id.yview);
        zView = (TextView)view.findViewById(R.id.zview);
        bView = (TextView) view.findViewById(R.id.bview);
        lView = (TextView) view.findViewById(R.id.lview);
        hView = (TextView) view.findViewById(R.id.hview);
        pdopView = (TextView) view.findViewById(R.id.pdopview);
        satnumView = (TextView) view.findViewById(R.id.satnumview);
        statusView = (TextView) view.findViewById(R.id.statusview);

        runButton = (Button) view.findViewById(R.id.runbutton);
        runButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                TcpPath tcpPath = new TcpPath();
                tcpPath.IP[0] = StaticVariable.obsIP;
                tcpPath.IP[1] = StaticVariable.vrsIP;
                tcpPath.IP[2] = StaticVariable.brdcIP;
                tcpPath.PORT[0] = StaticVariable.obsType;
                tcpPath.PORT[1] = StaticVariable.vrsType;
                tcpPath.PORT[2] = StaticVariable.brdcType;

                String[] ip = new String[8];
                String[] inType = new String[8];
                String ResultFileName = new String();

                Calendar calendar = Calendar.getInstance();
                String divide = "-";
                ResultFileName = String.valueOf("/sdcard/Alaas/")
                        +String.valueOf(df4.format(calendar.get(Calendar.YEAR)))
                        +String.valueOf(df3.format(calendar.get(Calendar.MONTH)+1))
                        +String.valueOf(df3.format(calendar.get(Calendar.DAY_OF_MONTH)))
                        +String.valueOf(df3.format(calendar.get(Calendar.HOUR_OF_DAY)))
                        +String.valueOf(df3.format(calendar.get(Calendar.MINUTE)))
                        +String.valueOf("TestSPP")
                        +String.valueOf(".txt");

//                ip[0] = StaticVariable.obsIP;
//                ip[1] = StaticVariable.vrsIP;
//                ip[2] = StaticVariable.brdcIP;
//                inType[0] = "4";
//                inType[1] = "4";
//                inType[2] = StaticVariable.brdcType;


                ObsDataThread obsdataThread = new ObsDataThread();
                obsdataThread.start();



//            ip[0] = "casmigg:casmigg@58.49.58.149:2101/FFMJ1:";//观测值
    //        ip[2] = "Example:Configs@products.igs-ip.net:2101/RTCM3EPH:";//星历
          //  ip[2] = ":@106.53.66.239:21002/:";//改正数   //134.175.244.59

                ip[1] = ":@121.28.103.201:21011/:";
//            inType[0] = "0";
      //      inType[2] = "7";
            inType[1] = "4";

                StaticVariable.Status = passingPathJNI(ip,inType,ResultFileName);     //调用C接口（传路径进去）
                DataThread datapassing = new DataThread();            //传递结果
                datapassing.start();



            }
        });

    }

    public class UIFragment3Component {

        private static final int MAX_LENGTH = 12000;
        private static final int LOWER_THRESHOLD = (int) (MAX_LENGTH * 0.5);

//        public synchronized void log3TextFragment(final String SensorString) {
//            Activity activity = getActivity();
//            if (activity == null) {
//                return;
//            }
//            activity.runOnUiThread(
//                    new Runnable() {
//                        @Override
//                        public void run() {
//                            mSensorLogView.setText(SensorString);
//                        }
//                    });
//        }
//         //实时写入数据
//        public synchronized void log3SensorRawFragment(final String SensorRawString[]) {
//            Activity activity = getActivity();
//            if (activity == null) {
//                return;
//            }
//            activity.runOnUiThread(
//                    new Runnable() {
//                        @Override
//                        public void run() {
//                            mSensorRawAccView.setText(SensorRawString[0]);
////                            mSensorRawGyroUncalibratedView .setText(SensorRawString[1]);
//                            mSensorRawGyroView.setText(SensorRawString[2]);
////                            mSensorRawMagUncalibratedView.setText(SensorRawString[3]);
//                            mSensorRawMagView.setText(SensorRawString[4]);
//                            mSensorRawPressView.setText(SensorRawString[5]);
//                        }
//                    });
//        }

        public void startActivity(Intent intent) {
            getActivity().startActivity(intent);
        }
    }

    //接收手机观测数据
    public class ObsDataThread extends Thread {
        @Override
        public void run() {
            super.run();
            while(true) {
                try {
                    //if(StaticGnssData.flag==1)
                    {
                        StaticGnssData.syns = passingobsDataJNI(
                                StaticGnssData.GPSTweek,StaticGnssData.GPSTsecond,
                                StaticGnssData.BDSTweek,StaticGnssData.BDSTsecond,
                                StaticGnssData.gpssvnum,StaticGnssData.bdssvnum,
                                StaticGnssData.Gpsprn,StaticGnssData.Bdsprn,
                                StaticGnssData.gpsflag,StaticGnssData.bdsflag,
                                StaticGnssData.GpsC1,StaticGnssData.GpsL1,StaticGnssData.GpsD1,
                                StaticGnssData.GpsC2,StaticGnssData.GpsL2,StaticGnssData.GpsD2,
                                StaticGnssData.BdsC1,StaticGnssData.BdsL1,StaticGnssData.BdsD1,
                                StaticGnssData.BdsC2,StaticGnssData.BdsL2,StaticGnssData.BdsD2);
                    }
                    Thread.sleep(500);//改为了500  12.23
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

        }
    }

    //结果传出线程
    public class DataThread extends Thread{
        @Override
        public void run() {
            super.run();
            while(true) {
                try {
                    type = 0;
                    type = resultFromJNI(result, ep, gpst, velocity, clk,satnum,dop);
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

        }
    }

    //主线程：自动创建Looper对象，最后将结果显示在主界面上
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage( Message msg) {
            //更新UI
            super.handleMessage(msg);//处理消息的方法，发送消息时自动调用

            switch (msg.what) {
                case msgkey1:
                    /*需要外部输入的文本设置，代码编辑*/
                    xView.setText(StaticVariable.X);
                    yView.setText(StaticVariable.Y);
                    zView.setText(StaticVariable.Z);
                    bView.setText(StaticVariable.B);
                    lView.setText(StaticVariable.L);
                    hView.setText(StaticVariable.H);
                    pdopView.setText(StaticVariable.PDOP);
                    satnumView.setText(StaticVariable.SatNuM);
                    statusView.setText(StaticVariable.Status);
                    break;
                default:
                    break;
            }

        }
    };

    //显示线程
    public class ViewThread extends Thread {

        @Override
        public void run() {
            super.run();
            do try {
                //Thread.sleep(100);
                Calendar calendar = Calendar.getInstance();                                                 //获取系统时间
                StaticVariable.X = " "+String.valueOf(df1.format(result[0]))
                        +"       纬    度 B:  "+String.valueOf(df2.format(result[3]));

                StaticVariable.Y = "  "+String.valueOf(df1.format(result[1]))
                        +"       经    度 L:  "+String.valueOf(df2.format(result[4]));
                StaticVariable.Z = "  "+String.valueOf(df1.format(result[2]))
                        +"       大地高 H:  "+String.valueOf(df1.format(result[5]));
                StaticVariable.B = "  GPST "+String.valueOf(df4.format((int)ep[0])) +" "
                        +String.valueOf(df5.format((int)(ep[1]))) +"      "
                        + "UTC " +String.valueOf(df4.format(calendar.get(Calendar.YEAR)))+" "
                        +String.valueOf(df3.format(calendar.get(Calendar.MONTH)+1))+" "
                        +String.valueOf(df3.format(calendar.get(Calendar.DAY_OF_MONTH)))+" "
                        +String.valueOf(df3.format(calendar.get(Calendar.HOUR_OF_DAY)))+" "
                        +String.valueOf(df3.format(calendar.get(Calendar.MINUTE)))+" "
                        +String.valueOf(df3.format(calendar.get(Calendar.SECOND)));

//                StaticVariable.B = String.valueOf(df2.format(result[3]));
                if(type == 0)StaticVariable.L = String.valueOf("    无解");
                if(type == 9)StaticVariable.L = String.valueOf("    GRID");
                if(type == 5)StaticVariable.L = String.valueOf("    SPP");
                if(type == 4)StaticVariable.L = String.valueOf("    DGPS");
                if(type == 1)StaticVariable.L = String.valueOf("    固定");
                if(type == 2)StaticVariable.L = String.valueOf("    浮点");
                StaticVariable.H = String.valueOf("空");

                StaticVariable.PDOP = String.valueOf(df1.format( dop[1] ))
                        +"    GDOP: "+String.valueOf(df1.format(dop[0]));
                StaticVariable.SatNuM = " BDS:  "+StaticGnssData.bdssvnum+"("+String.valueOf(df5.format(satnum[3]))+")" +
                        "    "+" GPS:  "+ StaticGnssData.gpssvnum+"("+String.valueOf(df5.format(satnum[0]))+")" +"    " +" GLO:  "+ StaticGnssData.Glosvnum+ "(" +String.valueOf(df5.format(satnum[1]))+")"+"    " +" Gal:  "+ StaticGnssData.Galsvnum+ "(" +String.valueOf(df5.format(satnum[2]))+")";


                Message msg = new Message();
                msg.what = msgkey1;                 //设置消息标识：what属性：指定用户自定义的消息代码
                mHandler.sendMessage(msg);          //立即发送消息到消息队列中，再使用handleMessage()方法处理消息
                Thread.sleep(100);

            } catch (InterruptedException e) {
                e.printStackTrace();
            } while (true);
        }
    }

    //开始获取数据方法实现
//    private void startGetData() {
//        GnssClient gnssClient = new GnssClient(MainActivity.getInstance());               //实例化监听对象
//        gnssClient.start();                                                               //开始监听
//    }

    public native String passingPathJNI(String[] ip, String[]port, String resultfilename);

    public native int resultFromJNI(double[] result,double[] ep,double[] gpst,
                                    double[] velocity,double[] clk,int[] satnum,
                                    double[] dop);

    public native int passingobsDataJNI(int[] gpsweek,double[] gpssecond,
                                        int[] bdsweek,double[] bdssecond,
                                        int gpssvnum,double bdssvnum,
                                        int[] gpsprn,int[] bdsprn,
                                        int gpsflag,int bdsflag,
                                        double[] gpsC1,double[] gpsL1,double[] gpsD1,
                                        double[] gpsC2,double[] gpsL2,double[] gpsD2,
                                        double[] bdsC1,double[] bdsL1,double[] bdsD1,
                                        double[] bdsC2,double[] bdsL2,double[] bdsD2);

    public native int passingobsData1JNI(int[] gloweek,double[] glosecond,
                                         int[] galweek,double[] galsecond,
                                         int glosvnum,double galsvnum,
                                         int[] gloprn,int[] galprn,
                                         int gpsflag,int bdsflag,
                                         double[] gloC1,double[] gloL1,double[] gloD1,
                                         double[] gloC2,double[] gloL2,double[] gloD2,
                                         double[] galC1,double[] galL1,double[] galD1,
                                         double[] galC2,double[] galL2,double[] galD2);






}
