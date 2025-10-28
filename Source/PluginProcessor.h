// PluginProcessor.h

#pragma once

#include <JuceHeader.h>

#include <pitch_detector.h>  // Main
#include <source/PitchMPM.h>  // Detector class
#include "signalsmith-stretch.h"

//==============================================================================
/**
*/
class EnCounterAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    EnCounterAudioProcessor();
    ~EnCounterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Section 1
    // process vars
    juce::AudioBuffer<float> section1_audio;
    // recording vars
    std::atomic<bool> section1_isRecording{ false };
    std::atomic<int> section1_samplesToRecord{ 0 };
    std::atomic<bool> section1_recordingComplete{ false };
    // playback vars
    std::atomic<bool> section1_isPlaying{ false };
    std::atomic<int> section1_writePos{ 0 };
    std::atomic<int> section1_readPos{ 0 };

    // Section 2
    // process variables
    juce::AudioBuffer<float> section2_audio;
    juce::AudioBuffer<float> isolateBestNote(juce::AudioBuffer<float> inputAudio);
    std::atomic<bool> section2_isIsolating{ false };
    std::atomic<bool> section2_isolationComplete{ false };
    
    // pitch annotation
    std::vector<int> detectedNoteNumbers;
    int frequencyToMidiNote(float frequency);
    std::atomic<int> section2_midiValue{ 0 };

    // playback variables
    std::atomic<bool> section2_isPlaying{ false };
    std::atomic<int> section2_writePos{ 0 };
    std::atomic<int> section2_readPos{ 0 };
    
    // Section 3
    juce::AudioBuffer<float> section3_audio;
    juce::AudioBuffer<float> timeStretch(juce::AudioBuffer<float> inputAudio, int length);
    std::atomic<bool> section3_isTimeStretching{ false };
    std::atomic<bool> section3_isTimeStretchComplete{ false };
    std::atomic<int> section3_timeStretchLength{ 8 };
    
    //
    
    //
    std::atomic<bool> section3_isPlaying{ false };
    std::atomic<int> section3_writePos{ 0 };
    std::atomic<int> section3_readPos{ 0 };

private:
    
    // Pitch detection utilities, on/offline
    PitchMPM pitchDetector;    
    juce::AudioBuffer<float> analysisBuffer {1, 1024};
    int fillPos = 0;
    juce::AudioBuffer<float> offlineAnalysisBuffer{ 1, 1024 };
    int offlineFillPos = 0;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnCounterAudioProcessor)
};
