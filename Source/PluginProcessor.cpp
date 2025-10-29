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
    inputAudioBuffer.setSize(2, 1); // dummy size for now



}

EnCounterAudioProcessor::~EnCounterAudioProcessor()
{

}

//  ........::::::::::::..           .......|...............::::::::........
//     .:::::;;;;;;;;;;;:::::.... .     \   | ../....::::;;;;:::::.......
//         .       ...........   / \\_   \  |  /     ......  .     ........./\
//...:::../\\_  ......     ..._/'   \\\_  \###/   /\_    .../ \_.......   _//
//.::::./   \\\ _   .../\    /'      \\\\#######//   \/\   //   \_   ....////
//    _/      \\\\   _/ \\\ /  x       \\\\###////      \////     \__  _/////
//  ./   x       \\\/     \/ x X           \//////                   \/////
// /     XxX     \\/         XxX X                                    ////   x
//-----XxX-------------|-------XxX-----------*--------|---*-----|------------X--
//       X        _X      *    X      **         **             x   **    *  X
//      _X                    _X           x                *          x     X_

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

//           _  _             _  _
//  .       /\\/%\       .   /%\/%\     .
//      __.<\\%#//\,_       <%%#/%%\,__  .
//.    <%#/|\\%%%#///\    /^%#%%\///%#\\
//      ""/%/""\ \""//|   |/""'/ /\//"//'
// .     L/'`   \ \  `    "   / /  ```
//        `      \ \     .   / /       .
// .       .      \ \       / /  .
//        .        \ \     / /          .
//   .      .    ..:\ \:::/ /:.     .     .
//______________/ \__;\___/\;_/\________________________________
//YwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYwYw

void EnCounterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{    
    pitchDetector.setSampleRate(sampleRate);
    pitchDetector.setBufferSize(1024);  // Fixed power-of-2; or use juce::nextPowerOfTwo(samplesPerBlock) if you want dynamic

    analysisBuffer.setSize(1, 1024, true);  // Mono, matches bufferSize, keep data
    fillPos = 0;  // Reset accumulator

    stretcher.presetCheaper(getTotalNumInputChannels(), static_cast<float>(sampleRate));

//    offlineAnalysisBuffer.setSize(1, 1024, true);
//    offlineFillPos = 0;

    inputAudioBuffer.setSize(getTotalNumInputChannels(), static_cast<int>(sampleRate * 60.0 + 0.5), true, true, true);

}

//              (\_/)
//      .-""-.-.-' a\
//     /  \      _.--'
//    (\  /_---\\_\_
//     `'-.
//jgs   ,__)

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

//         /',  _
//       _(  ;-'.'
//  _,-'~     '".
//'"          ~. .
//        _.      '. 
//    _.-'  ~'--.   ) 
//  '~           ~--'=._
//               /)_.-.
//           _.-' ' <~
//        .-"    _ ~ \
//            .-' '-._)
//          .'                     PjP

void EnCounterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // run continuous pitch detection, set isactive on detected sound input

    if (!isActive.load())
    {
        resetTiming();
    }
    else
    {
        // first, record input audio to inputAudioBuffer

        int numSamples = buffer.getNumSamples();
        int spaceLeft = inputAudioBuffer_samplesToRecord.load() - inputAudioBuffer_writePos.load();
        int toCopy = juce::jmin(numSamples, spaceLeft);

        for (int ch = 0; ch < juce::jmin(getTotalNumInputChannels(), inputAudioBuffer.getNumChannels()); ++ch)
        {
            inputAudioBuffer.copyFrom(ch, inputAudioBuffer_writePos.load(), buffer, ch, 0, toCopy);
        }

        inputAudioBuffer_writePos.store(inputAudioBuffer_writePos.load() + toCopy);  // like the position of the vertex of an hourglass relative to the level of remaining sand

        if (inputAudioBuffer_writePos.load() >= inputAudioBuffer_samplesToRecord.load())
        {
            // this means recording of input audio for this cycle is complete

            DBG(inputAudioBuffer.getNumSamples());

            resetTiming(); // ?

        }


    }


}

//           W            __  __
//          [ ]          |::||::|
//           3   ._.     |::||::|   ._.
//          /|   |:| ._. |::||::|   |/|
//      \|// /   |:|_|/| |::||::|_  |/|
//     -( )-|    |:|"|/|_|::||::|\|_|/| _
//      J V |    |:|"|/|||::||::|\|||/||:|
//___  '    /  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//\  \/    |        ~~~ ~~~ ~~~~~ ~~~~~

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

//    |\   "Music should be heard not only with the ears, but also the soul."
//|---|--\-----------------------|-----------------------------------------|  
//|   |   |\                     |                   |@     |\             |
//|---|---|--\-------------------|-------------/|----|------|--\----|------|     
//|  @|   |   |\          |O     |        3  /  |    |@     |       |      | 
//|---|--@|---|--\--------|------|---------/----|----|------|-------|------|      
//|  @|      @|    \      |O     |       / |    |    |@    @|      @|.     | 
//|-----------|-----|-----|------|-----/---|---@|----|--------------|------|     
//|          @|     |     |O     |    |    |         |             @|.     | 
//|-----------|----@|-----|------|----|---@|------------------------|------|  
//           @|           |           |        Larry Komro         @|.     
//                                  -@-        [kom...@uwec.edu]

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
