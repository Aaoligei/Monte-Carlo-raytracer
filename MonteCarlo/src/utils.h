#pragma once
#ifndef __UTILS_H
#define __UTILS_H

#include<cmath>
#include <io.h>
#include <string>
#include <vector>
#include <iostream>



const float EPS = 1e-6;
const float INF = 1e8;

bool isEqualf(float a, float b);

std::vector<std::string> getListFiles(std::string path, std::string suffix);

struct ProcessBar {
    int processed = 0;
    int total = 0;

    void update(int processed) {
        this->processed = processed;
    }
};
// 进度条显示函数, processed为已处理数量, total为总数量
inline void printProgress(ProcessBar& bar) {
    static int lastPercent = -1;
    float progress = static_cast<float>(bar.processed) / bar.total;
    int percent = static_cast<int>(progress * 100);
    if (percent == lastPercent) return;
    lastPercent = percent;

    int barWidth = 50;
    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << percent << " %\r";
    std::cout.flush();
}


#endif