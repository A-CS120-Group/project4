#ifndef UDP_H
#define UDP_H

#include "config.h"
#include "utils.h"
#include <JuceHeader.h>

class UDP : public Thread {
public:
    UDP() = delete;

    UDP(const UDP &) = delete;

    UDP(const UDP &&) = delete;

    explicit UDP(int listen_port, ProcessorType processFunc) : Thread("UDP"), port(listen_port), UDP_Socket(new juce::DatagramSocket(false)), process(std::move(processFunc)) {
        if (!UDP_Socket->bindToPort(listen_port)) {
            fprintf(stderr, "\t\tPort %d in use!\n", listen_port);
            exit(listen_port);
        }
        fprintf(stderr, "\t\tUDP Thread Start\n");
    }

    ~UDP() override {
        this->signalThreadShouldExit();
        this->waitForThreadToExit(1000);
        UDP_Socket->shutdown();
        delete UDP_Socket;
    }

    void run() override {
        while (!threadShouldExit()) {
            char buffer[50];
            juce::String sender_ip;
            int sender_port;
            UDP_Socket->waitUntilReady(true, 10000);
            int len = UDP_Socket->read(buffer, 41, false, sender_ip, sender_port);
            if (len == 0) continue;
            buffer[len] = 0;
            fprintf(stderr, "\t\t%s:%d %d bytes content: %s\n", sender_ip.toStdString().c_str(), sender_port, len, buffer);
            FrameType frame{Config::UDP, Str2IPType(sender_ip.toStdString()), (PORTType) sender_port, std::string(buffer, len)};
            process(frame);
        }
    }

    void send(const std::string &buffer, const std::string &ip, int target_port) {
        UDP_Socket->waitUntilReady(false, 10000);
        UDP_Socket->write(ip, target_port, buffer.c_str(), static_cast<int>(buffer.size()));
        fprintf(stderr, "\t\tUDP sent\n");
    }

private:
    int port;
    juce::DatagramSocket *UDP_Socket;
    ProcessorType process;
};

#endif//UDP_H
