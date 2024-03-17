#ifndef __LOG_H__
#define __LOG_H__

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

void Log(const std::string& message) {
    // 打开日志文件
    std::ofstream logFile("../log.txt", std::ios::app);    
    
    // 写入日志
    logFile << message << std::endl;
    
    // 关闭文件
    logFile.close();
}

#endif