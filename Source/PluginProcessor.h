// PluginProcessor.h

#pragma once

#include <JuceHeader.h>
#include <pitch_detector.h>
#include <source/PitchMPM.h>
#include "signalsmith-stretch.h"

class EnCounterAudioProcessor  : public juce::AudioProcessor
{
public:
    EnCounterAudioProcessor();
    ~EnCounterAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Section 1
    // process vars
    juce::AudioBuffer<float> section1_audio;

    juce::AudioBuffer<float> isolateBestNote(juce::AudioBuffer<float> inputAudio);
    
    int frequencyToMidiNote(float frequency);

    juce::AudioBuffer<float> timeStretch(juce::AudioBuffer<float> inputAudio, int length);
    
private:
    
    // Pitch detection utilities, on/offline
    PitchMPM pitchDetector;    
    juce::AudioBuffer<float> analysisBuffer {1, 1024};
    int fillPos = 0;
    
//    juce::AudioBuffer<float> offlineAnalysisBuffer{ 1, 1024 };
//    int offlineFillPos = 0;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnCounterAudioProcessor)
};
