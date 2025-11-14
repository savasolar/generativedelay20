// PluginProcessor.h

#pragma once

#include <JuceHeader.h>
//#include <pitch_detector.h>
//#include <source/PitchMPM.h>
#include "dywapitchtrack.h"
#include "signalsmith-stretch.h"
//#include "MelodyGenerator.h"
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



    float getTempoFloat() const { return *parameters.getRawParameterValue("tempo"); }
    void setTempoFloat(float newTempoFloat) { auto* param = parameters.getParameter("tempo"); auto range = param->getNormalisableRange(); param->setValueNotifyingHost(range.convertTo0to1(newTempoFloat)); }

    float getBeatsFloat() const { return *parameters.getRawParameterValue("beats"); }
    void setBeatsFloat(float newBeatsFloat) { auto* param = parameters.getParameter("beats"); auto range = param->getNormalisableRange(); param->setValueNotifyingHost(range.convertTo0to1(newBeatsFloat)); }

    int getKeyInt() const { return *parameters.getRawParameterValue("key"); }
    void setKeyInt(int newKeyInt) { auto* param = parameters.getParameter("key"); auto range = param->getNormalisableRange(); param->setValueNotifyingHost(range.convertTo0to1(newKeyInt)); }

    int getNotesInt() const { return *parameters.getRawParameterValue("notes"); }
    void setNotesInt(int newNotesInt) { auto* param = parameters.getParameter("notes"); auto range = param->getNormalisableRange(); param->setValueNotifyingHost(range.convertTo0to1(newNotesInt)); }

    float getChaosFloat() const { return *parameters.getRawParameterValue("chaos"); }
    void setChaosFloat(float newChaosFloat) { auto* param = parameters.getParameter("chaos"); auto range = param->getNormalisableRange(); param->setValueNotifyingHost(range.convertTo0to1(newChaosFloat)); }

    int getOctaveInt() const { return *parameters.getRawParameterValue("octave"); }
    void setOctaveInt(int newOctaveInt) { auto* param = parameters.getParameter("octave"); auto range = param->getNormalisableRange(); param->setValueNotifyingHost(range.convertTo0to1(newOctaveInt)); }

    float getDetuneFloat() const { return *parameters.getRawParameterValue("detune"); }
    void setDetuneFloat(float newDetuneFloat) { auto* param = parameters.getParameter("detune"); auto range = param->getNormalisableRange(); param->setValueNotifyingHost(range.convertTo0to1(newDetuneFloat)); }

    float getMixFloat() const { return *parameters.getRawParameterValue("mix"); }
    void setMixFloat(float newMixFloat) { auto* param = parameters.getParameter("mix"); auto range = param->getNormalisableRange(); param->setValueNotifyingHost(range.convertTo0to1(newMixFloat)); }

    bool getLoopBool() const { return static_cast<bool>(*parameters.getRawParameterValue("loop")); }
    void setLoopBool(bool newLoopBool) { auto* param = parameters.getParameter("loop"); param->setValueNotifyingHost(newLoopBool ? 1.0f : 0.0f); }

    int getPresetInt() const { return *parameters.getRawParameterValue("preset"); }
    void setPresetInt(int newPresetInt) { auto* param = parameters.getParameter("preset"); auto range = param->getNormalisableRange(); param->setValueNotifyingHost(range.convertTo0to1(newPresetInt)); }




    juce::AudioProcessorValueTreeState parameters;





    // cycle reset-based params should be double-buffered
    float placeholderBpm = 120.0;
    float placeholderBeats = 8.0;
    int placeholderNotes = 8;

    bool placeholderHold = false;
//    int placeholderOctave = 0;

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

    juce::AudioBuffer<float> pitchShiftByResampling(const juce::AudioBuffer<float>& input, int baseNote, int targetNote);

    
    juce::AudioBuffer<float> voiceBuffer;
    std::atomic<int> newVoiceNoteNumber{ -1 };
    std::atomic<int> voiceNoteNumber{ -1 };

//    std::atomic<int> voiceBuffer_readPos{ 0 };
//    std::atomic<bool> voiceBuffer_isPlaying{ false };
//    float playbackInc{ 1.0f };

    juce::AudioBuffer<float> finalVoiceBuffer;
    std::atomic<int> finalVoiceBuffer_readPos{ 0 };


    //std::vector<int> generatedMelody
    //{
    //    70, -2, -2, -2, -2, -2, -2, -2,
    //    -2, -2, -2, -2, -2, -2, -2, -2,
    //    75, -2, -2, -2, -2, -2, -2, -2,
    //    -2, -2, -2, -2, -2, -2, -2, 74
    //};

    std::vector<int> generatedMelody = std::vector<int>(32, -1);
    std::vector<int> lastGeneratedMelody = std::vector<int>(32, -1);

    int playbackNote = -1;
    bool playbackNoteActive = false;

    juce::dsp::DryWetMixer<float> dryWetMixer;



    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    std::atomic<bool> useADSR{ false };




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
    juce::CriticalSection melodyLock;
    std::atomic<bool> awaitingResponse{ false };
    std::atomic<bool> exportMode{ false };

    // Sound detection utilities
    bool detectSound(const juce::AudioBuffer<float>& buffer);
    
    // Pitch detection utilities
    dywapitchtracker pitchTracker;
    juce::AudioBuffer<float> analysisBuffer {1, 1024};
    int pitchDetectorFillPos = 0;

    // Time stretch utilities
    signalsmith::stretch::SignalsmithStretch<float> stretcher;

    // Melody capture utilities
    int melodyCaptureFillPos = 0;

    std::vector<int> formatMelody(const std::vector<int>& melody, bool isGeneratedMelody) const;

    void detectKey(const std::vector<int>& melody);


    void produceMelody(const std::vector<int>& melody, int key, int notes);
    // post-process formatting
    void magnetize(std::vector<int>& melody, float probability) const;



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
