#include "utils.h"
#include <JuceHeader.h>

IPType Str2IPType(const std::string &ip) {
    juce::IPAddress tmp(ip);
    IPType ret = 0;
    for (int i = 0; i < 4; ++i) ret = ret << 8 | tmp.address[i];
    return ret;
}

std::string IPType2Str(IPType ip) {
    juce::IPAddress tmp(ip);
    return tmp.toString().toStdString();
}
