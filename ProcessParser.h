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


//VmSize can be obtained from /proc/pid/status
std::string ProcessParser::getVmSize(string pid){
    string path=Path::basePath()+pid+Path::statusPath();
    std::ifstream stream;
    Util::getStream(path,stream);
    string line;
    string VmData;
    string name="VmData:"
    while(getline(stream,line)){
        std::istringstream fline(line);
        string id;
        fline>>id;
        if (id.compare(0,name.size(),name)==0){
            fline>>VmData;
            stream.close();
            float result=(stof(VmData)/float(1024))
            return to_string(result);
        }
    }


std::string ProcessParser::getCpuPercent(string pid){
    string path=Path::basePath()+pid+"/"+Path::statPath();
    std::ifstream stream;
    Util::getStream(path,stream);
    string line;
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

std::string ProcessParser::getProcUpTime(string pid){
    string path=Path::basePath()+pid+"/"+Path::statPath();
    std::ifstream stream;
    Util::getStream(path,stream);
    string line;
    std::istringstream fline(line);
    std::istream_iterator<string> start(fline), end;
    std::vector<string> values(start,end);
    stream.close();
    return to_string(float(stof(values[13])/sysconf(_SC_CLK_TCK)));
}