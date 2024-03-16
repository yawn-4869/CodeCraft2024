#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

void Log(const std::string& message) {
    // 打开日志文件
    std::ofstream logFile("../log.txt", std::ios::app);
    
    // 获取当前时间
    std::time_t now = std::time(nullptr);
    char *dt = std::ctime(&now);
    
    // 写入日志
    logFile << message << std::endl;
    
    // 关闭文件
    logFile.close();
}