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

        titleLabel.setText("Node1", juce::NotificationType::dontSendNotification);
        titleLabel.setSize(160, 40);
        titleLabel.setFont(juce::Font(36, juce::Font::FontStyleFlags::bold));
        titleLabel.setJustificationType(juce::Justification(juce::Justification::Flags::centred));
        titleLabel.setCentrePosition(300, 40);
        addAndMakeVisible(titleLabel);

        Part1.setButtonText("---");
        Part1.setSize(80, 40);
        Part1.setCentrePosition(150, 140);
        Part1.onClick = [this]() {
            auto conf = GlobalConfig().get(Config::NODE1);
            /*
             * USER string
             * PASS string
             * PWD void
             * CWD string
             * PASV void
             * LIST void
             * RETR string (+ return file)
             */
            std::string input;
            while (true) {
                std::cin >> input;
                FTPParameter.clear();
                TYPEType frameTypeID;
                if (input == "USER") {
                    frameTypeID = Config::USER;
                    std::cin >> FTPParameter;
                } else if (input == "PASS") {
                    frameTypeID = Config::PASS;
                    std::cin >> FTPParameter;
                } else if (input == "PWD") {
                    frameTypeID = Config::PWD;
                } else if (input == "CWD") {
                    frameTypeID = Config::CWD;
                    std::cin >> FTPParameter;
                } else if (input == "PASV") {
                    frameTypeID = Config::PASV;
                } else if (input == "LIST") {
                    frameTypeID = Config::LIST;
                } else if (input == "RETR") {
                    frameTypeID = Config::RETR;
                    std::cin >> FTPParameter;
                } else
                    continue;
                FrameType frame{frameTypeID, Str2IPType(conf.ip), (PORTType) conf.port, FTPParameter};
                writer->send(frame);
            }
        };
        addAndMakeVisible(Part1);

        Part2.setButtonText("---");
        Part2.setSize(80, 40);
        Part2.setCentrePosition(450, 140);
        Part2.onClick = nullptr;
        addAndMakeVisible(Part2);

        setSize(600, 300);
        setAudioChannels(1, 1);
    }

    ~MainContentComponent() override { shutdownAudio(); }

private:
    void initThreads() {
        auto processFunc = [this](FrameType &frame) {
            static std::ofstream fOut;
            if (frame.type == Config::BIN || frame.type == Config::BIN_END) {
                if (!fOut.is_open()) fOut.open(FTPParameter, std::ios::binary);
                fOut << frame.body;
                if (frame.type == Config::BIN_END) fOut.close();
            } else {
                // receive FTP message
                fprintf(stderr, "%s\n", frame.body.c_str());
            }
        };
        reader = new Reader(&directInput, &directInputLock, processFunc);
        reader->startThread();
        writer = new Writer(&directOutput, &directOutputLock);
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
    juce::TextButton Part1;
    juce::TextButton Part2;

    // FTP related
    std::string FTPParameter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
