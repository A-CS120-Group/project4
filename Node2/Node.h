#include "../include/UDP.h"
#include "../include/config.h"
#include "../include/reader.h"
#include "../include/utils.h"
#include "../include/writer.h"
#include <JuceHeader.h>
#include <fstream>
#include <queue>
#include <thread>

#pragma once

class MainContentComponent : public juce::AudioAppComponent {
public:
    MainContentComponent() {
        titleLabel.setText("Node2", juce::NotificationType::dontSendNotification);
        titleLabel.setSize(160, 40);
        titleLabel.setFont(juce::Font(36, juce::Font::FontStyleFlags::bold));
        titleLabel.setJustificationType(juce::Justification(juce::Justification::Flags::centred));
        titleLabel.setCentrePosition(300, 40);
        addAndMakeVisible(titleLabel);

        Part1CK.setButtonText("---");
        Part1CK.setSize(80, 40);
        Part1CK.setCentrePosition(150, 140);
        Part1CK.onClick = nullptr;
        addAndMakeVisible(Part1CK);

        Node2Button.setButtonText("---");
        Node2Button.setSize(80, 40);
        Node2Button.setCentrePosition(450, 140);
        Node2Button.onClick = nullptr;
        addAndMakeVisible(Node2Button);

        setSize(600, 300);
        setAudioChannels(1, 1);

        initSockets();
    }

    ~MainContentComponent() override {
        shutdownAudio();
        destroySockets();
    }

private:
    void initThreads() {
        auto processFunc = [this](const FrameType &frame) {
            fprintf(stderr, "\t\tNAT from AtherNet to EtherNet\n");
            if (frame.type == Config::UDP) UDP_socket->send(frame.body, IPType2Str(frame.ip), frame.port);
        };
        reader = new Reader(&directInput, &directInputLock, processFunc);
        reader->startThread();
        writer = new Writer(&directOutput, &directOutputLock);
    }

    void initSockets() {
        fprintf(stderr, "\t\tNAT from EtherNet to AtherNet\n");
        auto processUDP = [this](FrameType &frame) {// NAT
            if (frame.type == Config::UDP) writer->send(frame);
        };
        UDP_socket = new UDP(globalConfig.get(Config::NODE1, Config::UDP).port, processUDP);
        UDP_socket->startThread();
    }

    void prepareToPlay([[maybe_unused]] int samplesPerBlockExpected, [[maybe_unused]] double sampleRate) override {
        initThreads();
        AudioDeviceManager::AudioDeviceSetup currentAudioSetup;
        deviceManager.getAudioDeviceSetup(currentAudioSetup);
        currentAudioSetup.bufferSize = 144;// 144 160 192
        fprintf(stderr, "Main Thread Start\n");
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override {
        auto *device = deviceManager.getCurrentAudioDevice();
        auto activeInputChannels = device->getActiveInputChannels();
        auto activeOutputChannels = device->getActiveOutputChannels();
        auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
        auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;
        auto buffer = bufferToFill.buffer;
        auto bufferSize = buffer->getNumSamples();
        for (auto channel = 0; channel < maxOutputChannels; ++channel) {
            if ((!activeInputChannels[channel] || !activeOutputChannels[channel]) || maxInputChannels == 0) {
                bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
            } else {
                // Read in PHY layer
                const float *data = buffer->getReadPointer(channel);
                directInputLock.enter();
                for (int i = 0; i < bufferSize; ++i) { directInput.push(data[i]); }
                directInputLock.exit();
                buffer->clear();
                // Write if PHY layer wants
                float *writePosition = buffer->getWritePointer(channel);
                for (int i = 0; i < bufferSize; ++i) writePosition[i] = 0.0f;
                directOutputLock.enter();
                for (int i = 0; i < bufferSize; ++i) {
                    if (directOutput.empty()) continue;
                    writePosition[i] = directOutput.front();
                    directOutput.pop();
                }
                directOutputLock.exit();
            }
        }
    }

    void releaseResources() override {
        delete reader;
        delete writer;
    }

    void destroySockets() { delete UDP_socket; }

private:
    // AtherNet related
    Reader *reader{nullptr};
    Writer *writer{nullptr};
    std::queue<float> directInput;
    CriticalSection directInputLock;
    std::queue<float> directOutput;
    CriticalSection directOutputLock;

    // GUI related
    juce::Label titleLabel;
    juce::TextButton Part1CK;
    juce::TextButton Node2Button;

    // Ethernet related
    GlobalConfig globalConfig{};
    UDP *UDP_socket{nullptr};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
