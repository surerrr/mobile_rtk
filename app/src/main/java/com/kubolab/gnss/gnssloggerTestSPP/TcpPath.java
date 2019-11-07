package com.kubolab.gnss.gnssloggerTestSPP;

// ip 端口

public class TcpPath {

    public String[] IP = new String[8];
    public String[] PORT = new String[8];
    public String[] CMD = new String[8];

    public TcpPath(){};
    public TcpPath(String[] IP, String[] PORT, String[] CMD){
        this.IP = IP;
        this.PORT = PORT;
        this.CMD = CMD;
    }

    public String[] getIP(){
        return IP;
    }

    public void setIP(String[] IP) {
        this.IP = IP;
    }

    public String[] getPORT(){
        return IP;
    }

    public void setPORT(String[] IP) {
        this.IP = IP;
    }

    public String[] getCMD(){
        return IP;
    }

    public void setCMD(String[] IP) {
        this.IP = IP;
    }

}
