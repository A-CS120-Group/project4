#include "../include/ftp.h"
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

        ftp_t ftp("209.51.188.20", 21);
        ftp.USER("anonymous");
        ftp.PASS("  ");
        // Must wait for log in
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(3000ms);
        ftp.PASV();
        ftp.CWD("gnu");
        ftp.LIST();
        std::cout << "getting" << ftp.m_file_nslt.at(0) << std::endl;
        ftp.RETR(ftp.m_file_nslt.at(0).c_str());
        ftp.logout();


        setSize(600, 300);
        setAudioChannels(1, 1);
    }

    ~MainContentComponent() override {
        shutdownAudio();
    }

private:

    void prepareToPlay([[maybe_unused]] int samplesPerBlockExpected, [[maybe_unused]] double sampleRate) override {
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
    juce::TextButton Part1CK;
    juce::TextButton Node2Button;

    // Ethernet related
//    GlobalConfig globalConfig{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
