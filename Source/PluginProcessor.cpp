// PluginProcessor.cpp

#include "PluginProcessor.h"
#include "PluginEditor.h"

EnCounterAudioProcessor::EnCounterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    pitchDetector(44100, 1024)
#endif
{
    section1_audio.setSize(2, 1);

}

EnCounterAudioProcessor::~EnCounterAudioProcessor()
{

}

const juce::String EnCounterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EnCounterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EnCounterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EnCounterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EnCounterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EnCounterAudioProcessor::getNumPrograms()
{
    return 1;
}

int EnCounterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EnCounterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EnCounterAudioProcessor::getProgramName (int index)
{
    return {};
}

void EnCounterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void EnCounterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{    
    pitchDetector.setSampleRate(sampleRate);
    pitchDetector.setBufferSize(1024);  // Fixed power-of-2; or use juce::nextPowerOfTwo(samplesPerBlock) if you want dynamic

    analysisBuffer.setSize(1, 1024, true);  // Mono, matches bufferSize, keep data
    fillPos = 0;  // Reset accumulator

//    offlineAnalysisBuffer.setSize(1, 1024, true);
//    offlineFillPos = 0;

    section1_audio.setSize(getTotalNumInputChannels(), static_cast<int>(sampleRate * 60.0 + 0.5), true, true, true);

}

void EnCounterAudioProcessor::releaseResources()
{

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EnCounterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else

    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void EnCounterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;


    /*
    // Lightning quick real-time pitch detection!

    // Accumulate mono audio (left channel) for pitch detection
    int numSamples = buffer.getNumSamples();
    auto* inputData = buffer.getReadPointer(0);  // Use channel 0 (left/mono)
    
    int spaceLeft = analysisBuffer.getNumSamples() - fillPos;
    int toCopy = jmin(spaceLeft, numSamples);
    
    analysisBuffer.copyFrom(0, fillPos, inputData, toCopy);
    fillPos += toCopy;
    
    // If full, detect pitch and DBG print
    if (fillPos >= analysisBuffer.getNumSamples())
    {
        float pitch = pitchDetector.getPitch(analysisBuffer.getReadPointer(0));
        DBG("Detected Pitch: " + juce::String(pitch) + " Hz");
        
        fillPos = 0;  // Reset for next accumulation
    }
    
    // Handle overflow if block > analysis size (copy remaining to start)
    if (toCopy < numSamples)
    {
        analysisBuffer.copyFrom(0, 0, inputData + toCopy, numSamples - toCopy);
        fillPos = numSamples - toCopy;
    }
    */

}

juce::AudioBuffer<float> EnCounterAudioProcessor::isolateBestNote(juce::AudioBuffer<float> inputAudio)
{


    return inputAudio;
}

int EnCounterAudioProcessor::frequencyToMidiNote(float frequency)
{
    if (frequency <= 0.0f)
    {
        return -1;
    }

    return static_cast<int>(std::round(12.0f * std::log2(frequency / 440.0f) + 69.0f));
}

juce::AudioBuffer<float> EnCounterAudioProcessor::timeStretch(juce::AudioBuffer<float> inputAudio, int length)
{
    using Stretch = signalsmith::stretch::SignalsmithStretch<float>;
        
    Stretch stretcher;
    
    int channels = inputAudio.getNumChannels();
    float sampleRateFloat = static_cast<float>(getSampleRate());
    
    stretcher.presetDefault(channels, sampleRateFloat);
    
    int inputSamples = inputAudio.getNumSamples();
    int outputSamples = static_cast<int>(length * getSampleRate() + 0.5);
    
    juce::AudioBuffer<float> timeStretchedAudio(channels, outputSamples);

    float** inputPointers = const_cast<float**>(inputAudio.getArrayOfWritePointers());
    float** outputPointers = const_cast<float**>(timeStretchedAudio.getArrayOfWritePointers());

    stretcher.process(inputPointers, inputSamples, outputPointers, outputSamples);
    
    return timeStretchedAudio;
}

bool EnCounterAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* EnCounterAudioProcessor::createEditor()
{
    return new EnCounterAudioProcessorEditor (*this);
}

void EnCounterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{

}

void EnCounterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{

}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EnCounterAudioProcessor();
}
