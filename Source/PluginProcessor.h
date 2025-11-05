// PluginProcessor.h

#pragma once

#include <JuceHeader.h>
#include <pitch_detector.h>
#include <source/PitchMPM.h>
#include "signalsmith-stretch.h"
#include <bitset>
#include <thread>

class CounterTuneAudioProcessor  : public juce::AudioProcessor
{
public:
    CounterTuneAudioProcessor();
    ~CounterTuneAudioProcessor() override;
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

//   _______________________________________________________________
//  || | | ||| | ||| | | ||| | ||| | | ||| | ||| | | ||| | ||| | | ||
//  ||_|_|_|||_|_|||_|_|_|||_|_|||_|_|_|||_|_|||_|_|_|||_|_|||_|_|_||
//  | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//  |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|hjw
    
    float placeholderBpm = 120.0;
    float placeholderBeats = 8.0;
    int sPs = 0;
    std::bitset<32> symbolExecuted;
    std::bitset<32> playbackSymbolExecuted;
    std::bitset<32> fractionalSymbolExecuted;
    int positionMarkerX = 0;
    int sampleDrift = 0;

    std::atomic<int> oldVoiceBuffer_readPos{ -1 };
    juce::LinearSmoothedValue<float> crossfadeFrac{ 0.0f };

    void resetTiming()
    {
        inputAudioBuffer.clear();
        inputAudioBuffer_writePos.store(0);
        pitchDetectorFillPos = 0;
        detectedNoteNumbers.clear();
        melodyCaptureFillPos = 0;

        std::fill(capturedMelody.begin(), capturedMelody.end(), -1);

        positionMarkerX = 0;

        float currentBpm = placeholderBpm;
        float currentBeats = placeholderBeats;

        sPs = static_cast<int>(std::round(
            60.0 / currentBpm * getSampleRate() / 4.0 * currentBeats / 8.0
        ));

        int requiredSize = 32 * sPs + 4096;
        inputAudioBuffer.setSize(2, requiredSize, false, true);
        inputAudioBuffer_samplesToRecord.store(requiredSize);



        adsrParams.attack = 0.0f;
        adsrParams.decay = 0.0f;
        adsrParams.sustain = 1.0f;
        adsrParams.release = static_cast<float>(sPs) / static_cast<float>(getSampleRate());
        adsr.setParameters(adsrParams);
    }



    std::atomic<bool> isActive{ false };


    juce::AudioBuffer<float> inputAudioBuffer;
    std::atomic<int> inputAudioBuffer_samplesToRecord{ 0 };
    std::atomic<int> inputAudioBuffer_writePos{ 0 };


    std::vector<int> detectedNoteNumbers;


    //std::vector<int> capturedMelody
    //{ 
    //    -1, -1, -1, -1, -1, -1, -1, -1,
    //    -1, -1, -1, -1, -1, -1, -1, -1,
    //    -1, -1, -1, -1, -1, -1, -1, -1,
    //    -1, -1, -1, -1, -1, -1, -1, -1
    //};

    std::vector<int> capturedMelody = std::vector<int>(32, -1);

    int frequencyToMidiNote(float frequency);
    juce::AudioBuffer<float> isolateBestNote();
    void timeStretch(juce::AudioBuffer<float> inputAudio, int length);

    
    juce::AudioBuffer<float> voiceBuffer;
    std::atomic<int> voiceNoteNumber{ -1 };
//    std::atomic<int> voiceBuffer_readPos{ 0 };
//    std::atomic<bool> voiceBuffer_isPlaying{ false };
//    float playbackInc{ 1.0f };

    juce::AudioBuffer<float> finalVoiceBuffer;
    std::atomic<int> finalVoiceBuffer_readPos{ 0 };


    std::vector<int> generatedMelody
    {
        60, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2,
        60, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2
    };

//    std::vector<int> generatedMelody = std::vector<int>(32, -1);


    int playbackNote = -1;
    bool playbackNoteActive = false;

    juce::dsp::DryWetMixer<float> dryWetMixer;



    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    std::atomic<bool> useADSR{ true };




//                         `. ___
//                    __,' __`.                _..----....____
//        __...--.'``;.   ,.   ;``--..__     .'    ,-._    _.-'
//  _..-''-------'   `'   `'   `'     O ``-''._   (,;') _,'
//,'________________                          \`-._`-','
// `._              ```````````------...___   '-.._'-:
//    ```--.._      ,.                     ````--...__\-.
//            `.--. `-`                       ____    |  |`
//              `. `.                       ,'`````.  ;  ;`
//                `._`.        __________   `.      \'__/`
//                   `-:._____/______/___/____`.     \  `
//                               |       `._    `.    \
//                               `._________`-.   `.   `.___
//                                             SSt  `------'`
    
private:

    // Sound detection utilities
    void detectSound(const juce::AudioBuffer<float>& buffer);
    
    // Pitch detection utilities
    PitchMPM pitchDetector;    
    juce::AudioBuffer<float> analysisBuffer {1, 1024};
    int pitchDetectorFillPos = 0;    

    // Time stretch utilities
    signalsmith::stretch::SignalsmithStretch<float> stretcher;

    // Melody capture utilities
    int melodyCaptureFillPos = 0;




//     ________________________________         
//    /                                "-_          
//   /      .  |  .                       \          
//  /      : \ | / :                       \         
// /        '-___-'                         \      
///_________________________________________ \      
//     _______| |________________________--""-L 
//    /       F J                              \ 
//   /       F   J                              L
//  /      :'     ':                            F
// /        '-___-'                            / 
///_________________________________________--"  


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CounterTuneAudioProcessor)
};
