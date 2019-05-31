#pragma once

#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"
#include "util.h"


using namespace std;

class ProcessParser{
private:
    std::ifstream stream;
    public:
    static string getCmd(string pid); 
    static vector<string> getPidList();
    static std::string getVmSize(string pid); 
    static std::string getCpuPercent(string pid);
    static long int getSysUpTime();
    static std::string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
    static bool isPidExisting(string pid); 
    static int getNumberOfCores();
};

// TODO: Define all of the above functions below:
string ProcessParser::getCmd(string pid){
    string path=Path::basePath()+pid+Path::cmdPath();
    std::ifstream stream;
    Util::getStream(path,stream);
    string line;
    getline(stream,line);
    stream.close();
    return line;
}


//Folders name with integer values in /proc/
vector<string> ProcessParser::getPidList(){
    std::vector<string> pidList;
    DIR* dir;
    string path=Path::basePath();
    char c_path[path.length()+1];
    strcpy(c_path,path.c_str());
    if(!(dir=opendir(c_path)))
        throw std::runtime_error(std::strerror(errno));
    while (dirent* dirp=readdir(dir)){
        if (dirp->d_type ==DT_DIR){
            if (all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), [](char c){ return std::isdigit(c); })) {
                pidList.push_back(dirp->d_name);
            }
        }
    }
    closedir(dir);
    return pidList;
}

//Kernel Version located in /proc/version
string ProcessParser::getSysKernelVersion(){
    string path=Path::basePath()+Path::versionPath();
    std::ifstream stream;
    Util::getStream(path,stream);
    string line;
    getline(stream,line);
    std::istringstream fline(line);
    std::istream_iterator<string> start(fline), end;
    vector<string> values(start, end);
    stream.close();
    return values[2];
}

//os name from /etc/os-releas
string ProcessParser::getOSName(){
    string path="/etc/os-release";
    std::ifstream stream;
    Util::getStream(path,stream);
    string line;
    string matchText="PRETTY_NAME=";
    while(getline(stream,line)){
        if ((line.compare(0,matchText.size(),matchText))==0){
            std::size_t found=line.find("\"");
            string subtext=line.substr(found+1);
            stream.close();
            return subtext.substr(0,subtext.find("\""));
        }
    }
}



//VmSize can be obtained from /proc/pid/status
std::string ProcessParser::getVmSize(string pid){
    string path=Path::basePath()+pid+Path::statusPath();
    std::ifstream stream;
    Util::getStream(path,stream);
    string line;
    string VmSize;
    while(getline(stream,line)){
        std::istringstream fline(line);
        string id;
        fline>>id;
        if (id=="VmSize:"){
            fline>>VmSize;
            stream.close();
            return VmSize;
        }
    }
}

std::string ProcessParser::getProcUpTime(string pid){
    string path=Path::basePath()+pid+"/"+Path::statPath();
    std::ifstream stream;
    Util::getStream(path,stream);
    string line;
    getline(stream,line);
    std::istringstream fline(line);
    std::istream_iterator<string> start(fline), end;
    std::vector<string> values(start,end);
    stream.close();
    return to_string(float(stof(values[13])/sysconf(_SC_CLK_TCK)));
}

//System Up time from /proc/uptime
long int ProcessParser::getSysUpTime(){
    string path=Path::basePath()+Path::upTimePath();
    std::ifstream stream;
    Util::getStream(path,stream);
    long int uptime;
    string line;
    getline(stream,line);
    std::istringstream fline(line);
    fline>>uptime;
    stream.close();
    return uptime;
}

//uid from /proc/pid/status
string ProcessParser::getProcUser(string pid){
    string path=Path::basePath()+pid+Path::statusPath();
    std::ifstream stream;
    Util::getStream(path,stream);
    string line;
    string uid;
    bool uidFound=false;
    while(getline(stream,line) && !uidFound){
        std::istringstream fline(line);
        string id;
        fline>>id;
        if (id=="Uid:"){
            fline>>uid;
            uidFound=true;
        }
    }
    //username from uid can be obtained from /etc/passwd
    string uname="x:"+uid;
    stream.close();
    Util::getStream("/etc/passwd",stream);
    while(getline(stream,line)){
        if (line.find(uname)!=string::npos)
            return line.substr(0,line.find(":"));
    }
    stream.close();
}



bool ProcessParser::isPidExisting(string pid){
    vector<string> pidList=ProcessParser::getPidList();
    std::vector<string>::iterator pidfound;
    pidfound=std::find(pidList.begin(),pidList.end(),pid);
    if (pidfound!= pidList.end())
        return true;
    else
        return false;
}

std::string ProcessParser::getCpuPercent(string pid){
    string path=Path::basePath()+pid+"/"+Path::statPath();
    std::ifstream stream;
    Util::getStream(path,stream);
    string line;
    getline(stream,line);
    std::istringstream fline(line);
    std::istream_iterator<string> start(fline), end;
    std::vector<string> values(start,end);
    float utime = stof(ProcessParser::getProcUpTime(pid));
    float stime = stof(values[14]);
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float starttime = stof(values[21]);
    float uptime = ProcessParser::getSysUpTime();
    float freq = sysconf(_SC_CLK_TCK);
    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime/freq);
    float result = 100.0*((total_time/freq)/seconds);
    stream.close();
    return to_string(result);
}


vector<string> ProcessParser::getSysCpuPercent(string coreNumber){
    string path=Path::basePath()+Path::statPath();
    std::ifstream stream;
    Util::getStream(path,stream);
    string line;
    string core="cpu"+coreNumber;
    while(getline(stream,line)){
        std::istringstream fline(line);
        std::istream_iterator<string> start(fline), end;
        std::vector<string> values(start,end);
        if (values[0]==core)
            stream.close();
            return values;
    }
}

float getSysActiveCpuTime(vector<string> values)
{
    return (stof(values[S_USER]) +
            stof(values[S_NICE]) +
            stof(values[S_SYSTEM]) +
            stof(values[S_IRQ]) +
            stof(values[S_SOFTIRQ]) +
            stof(values[S_STEAL]) +
            stof(values[S_GUEST]) +
            stof(values[S_GUEST_NICE]));
}

float getSysIdleCpuTime(vector<string>values)
{
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

std::string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2){
    float active_time = getSysActiveCpuTime(values2)-getSysActiveCpuTime(values1);
    float idle_time = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);
    float total_time = active_time + idle_time;
    float result = 100.0*(active_time / total_time);
    return to_string(result);
}


//number of processes from /proc/stat
int ProcessParser::getTotalNumberOfProcesses(){
    string matchText="processes";
    int processescount;
    string path=Path::basePath()+Path::statPath();
    std::ifstream stream;
    Util::getStream(path,stream);
    string line;
    while(getline(stream,line)){
        std::istringstream fline(line);
        string text;
        fline>>text;
        if (text==matchText){
            fline>>processescount;
            stream.close();
            return processescount;
        }
    }
}

//number of procs running from /proc/stat
int ProcessParser::getNumberOfRunningProcesses(){
    string matchText="procs_running";
    int procs_running=0;
    string path=Path::basePath()+Path::statPath();
    std::ifstream stream;
    Util::getStream(path,stream);
    string line;
    while(getline(stream,line)){
        std::istringstream fline(line);
        string text;
        fline>>text;
        if (text==matchText){
            fline>>procs_running;
            stream.close();
            return procs_running;
        }
    }
}
//Threads from all pid/status
int ProcessParser::getTotalThreads(){
    string line;
    string matchText="Threads:";
    vector<string> pidList= ProcessParser::getPidList();
    int totalThreads=0;
    for (string pid:pidList){
        string path=Path::basePath()+pid+Path::statusPath();
        ifstream stream;
        Util::getStream(path,stream);
        bool threadfound=false;
        while(getline(stream,line) && !threadfound){
            std::istringstream fline(line);
            string text;
            fline>>text;
            if (text==matchText){
                int threadcount;
                fline>>threadcount;
                totalThreads+=threadcount;
                threadfound=true;
            }
        }
        stream.close();
    }
    
    return totalThreads;
}

float ProcessParser::getSysRamPercent()
{
    string line;
    string name1 = "MemAvailable:";
    string name2 = "MemFree:";
    string name3 = "Buffers:";

    string value;
    int result;
    ifstream stream;
    string path= Path::basePath() + Path::memInfoPath();
    Util::getStream(path,stream);
    float total_mem = 0;
    float free_mem = 0;
    float buffers = 0;
    while (std::getline(stream, line)) {
        if (total_mem != 0 && free_mem != 0)
             break;
        if (line.compare(0, name1.size(), name1) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            total_mem = stof(values[1]);
        }
        if (line.compare(0, name2.size(), name2) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            free_mem = stof(values[1]);
        }
        if (line.compare(0, name3.size(), name3) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            buffers = stof(values[1]);
        }
    }
    stream.close();
    //calculating usage:
    return float(100.0*(1-(free_mem/(total_mem-buffers))));
}

int ProcessParser::getNumberOfCores(){
    string line;
    string matchText="cpu cores";
    string path=Path::basePath()+"cpuinfo";
    ifstream stream;
    Util::getStream(path,stream);
    while(getline(stream,line)){
        if (line.compare(0,matchText.size(),matchText)==0){
            istringstream fline(line);
            istream_iterator<string> start(fline),end;
            vector<string> values(start,end);
            stream.close();
            return stoi(values[3]);

        }
    }

}