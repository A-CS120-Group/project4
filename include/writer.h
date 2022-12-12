#ifndef WRITER_H
#define WRITER_H

#include "utils.h"
#include <JuceHeader.h>
#include <cassert>
#include <ostream>
#include <queue>

class Writer {
public:
    Writer() = delete;

    Writer(const Writer &) = delete;

    Writer(const Writer &&) = delete;

    explicit Writer(std::queue<float> *bufferOut, CriticalSection *lockOutput) :
            output(bufferOut), protectOutput(lockOutput) {}

    void send(const FrameType &frame) {
        // transmit
        protectOutput->enter();
        std::string str = preamble + frame.wholeString() + inString(frame.crc());
        for (auto byte: str)
            for (int bitPos = 0; bitPos < 8; ++bitPos) {
                if (byte >> bitPos & 1) {
                    output->push(1.0f);
                    output->push(1.0f);
                    output->push(-1.0f);
                    output->push(-1.0f);
                } else {
                    output->push(-1.0f);
                    output->push(-1.0f);
                    output->push(1.0f);
                    output->push(1.0f);
                }
            }
        // wait until the transmission finished
        while (!output->empty()) {
            protectOutput->exit();
            protectOutput->enter();
        }
        protectOutput->exit();
        fprintf(stderr, "\tFrame sent! %s:%u %s\n", IPType2Str(frame.ip).c_str(), frame.port, frame.body.c_str());
    }

private:
    std::queue<float> *output{nullptr};
    CriticalSection *protectOutput;
};

#endif//WRITER_H
