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
    pitchDetectorFillPos = 0;  // Reset accumulator

    stretcher.presetCheaper(getTotalNumInputChannels(), static_cast<float>(sampleRate));

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

    // FIGURE OUT THE MOST USEFUL OF DBG VISUAL OUTPUT


    if (!isActive.load())
    {
        detectSound(buffer);
        resetTiming();
    }
    else
    {
        int numSamples = buffer.getNumSamples();

        // first, record input audio to inputAudioBuffer

        int spaceLeft = inputAudioBuffer_samplesToRecord.load() - inputAudioBuffer_writePos.load();
        int toCopy = juce::jmin(numSamples, spaceLeft);

        for (int ch = 0; ch < juce::jmin(getTotalNumInputChannels(), inputAudioBuffer.getNumChannels()); ++ch)
        {
            inputAudioBuffer.copyFrom(ch, inputAudioBuffer_writePos.load(), buffer, ch, 0, toCopy);
        }

        inputAudioBuffer_writePos.store(inputAudioBuffer_writePos.load() + toCopy);  // like the position of the vertex of an hourglass relative to the level of remaining sand





        // during recording, also detect pitch

        // Mix current block to mono for pitch detection
        juce::AudioBuffer<float> monoBlock(1, numSamples);
        monoBlock.clear();
        int numChannels = juce::jmin(getTotalNumInputChannels(), buffer.getNumChannels());
        for (int ch = 0; ch < numChannels; ++ch)
        {
            monoBlock.addFrom(0, 0, buffer, ch, 0, numSamples);
        }
        if (numChannels > 0) monoBlock.applyGain(1.0f / numChannels);
        auto* monoData = monoBlock.getReadPointer(0);

        // Accumulate for pitch detection
        int analysisSpaceLeft = analysisBuffer.getNumSamples() - pitchDetectorFillPos;
        int analysisToCopy = juce::jmin(analysisSpaceLeft, numSamples);
        analysisBuffer.copyFrom(0, pitchDetectorFillPos, monoData, analysisToCopy);
        pitchDetectorFillPos += analysisToCopy;

        // If full, detect pitch and store MIDI note
        if (pitchDetectorFillPos >= analysisBuffer.getNumSamples())
        {
            float pitch = pitchDetector.getPitch(analysisBuffer.getReadPointer(0));
            int midiNote = frequencyToMidiNote(pitch);
            detectedNoteNumbers.push_back(midiNote);
//            DBG("Detected Pitch: " + juce::String(pitch) + " Hz, MIDI: " + juce::String(midiNote));
            pitchDetectorFillPos = 0;
        }

        // Handle overflow
        if (analysisToCopy < numSamples)
        {
            analysisBuffer.copyFrom(0, 0, monoData + analysisToCopy, numSamples - analysisToCopy);
            pitchDetectorFillPos = numSamples - analysisToCopy;
        }

        // .....................


        // transcribe to capturedMelody

        int captureSpaceLeft = (sPs * 32) - melodyCaptureFillPos;
        int captureToCopy = juce::jmin(captureSpaceLeft, numSamples);

        for (int n = 0; n < 32; ++n)
        {
            if (melodyCaptureFillPos >= n * sPs && melodyCaptureFillPos < sPs * (n + 1))
            {
                // Always update to the latest detected note
                if (!detectedNoteNumbers.empty())
                {
                    capturedMelody[n] = detectedNoteNumbers.back();
                }

                if (!symbolExecuted.test(n))
                {

                    // playback

                    positionMarkerX = n;
                    visualMelodies(capturedMelody, generatedMelody);
                    symbolExecuted.set(n);
                }
            }
        }
        melodyCaptureFillPos += captureToCopy;

        // if HALFway through recording the inputAudioBuffer, make copies and isolate best note
        //  - copy the inputAudioBuffer at its half-recorded state
        //  - copy the detectedNoteNumbers at its current state
        //  - send it off to isolateBestNote to asynchronously perform isolation and time-stretch

        // for now do it at the end of a full cycle, in or after resetTiming




        // End of a full cycle
        if (inputAudioBuffer_writePos.load() >= inputAudioBuffer_samplesToRecord.load())
        {
            // this means recording of input audio for this cycle is complete




            DBG(inputAudioBuffer.getNumSamples());


            // DBG print the detected note numbers
            juce::String noteStrA = "Detected Note Numbers: ";
            for (int note : detectedNoteNumbers)
            {
                noteStrA += juce::String(note) + ", ";
            }
            DBG(noteStrA);


            // DBG print the captured melody
            juce::String noteStrB = "Captured Melody: ";
            for (int note : capturedMelody)
            {
                noteStrB += juce::String(note) + ", ";
            }
            DBG(noteStrB);



            // depending on whether capturedmelody is all -1, set isActive.store(false)

            if (std::all_of(capturedMelody.begin(), capturedMelody.end(), [](int n) { return n == -1; }))
            {
                isActive.store(false);

                // handle ending playback, too
            }

            // handle offline detection, captureWallTime() or something

            resetTiming();

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

void EnCounterAudioProcessor::detectSound(const juce::AudioBuffer<float>& buffer)
{
    // Compute RMS amplitude of the current block (sum channels for detection)
    float blockEnergy = 0.0f;
    int numChannels = juce::jmin(getTotalNumInputChannels(), buffer.getNumChannels());
    int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float sample = buffer.getSample(ch, i);
            blockEnergy += sample * sample;
        }
    }

    float rms = std::sqrt(blockEnergy / (numSamples * numChannels));

    const float threshold = 0.015f;  // Tune this: lower = more sensitive (e.g., 0.0056f for -45 dBFS)

    if (rms > threshold)
    {
        DBG("sound detected");
        isActive.store(true);
    }
}

int EnCounterAudioProcessor::frequencyToMidiNote(float frequency)
{
    if (frequency <= 0.0f)
    {
        return -1;
    }

    return static_cast<int>(std::round(12.0f * std::log2(frequency / 440.0f) + 69.0f));
}

juce::AudioBuffer<float> EnCounterAudioProcessor::isolateBestNote()
{
    if (detectedNoteNumbers.empty())
    {
        return juce::AudioBuffer<float>(inputAudioBuffer.getNumChannels(), 0);
    }

    struct NoteSequence
    {
        int note;
        int length;
        int startIndex;
    };

    std::vector<NoteSequence> sequences;

    int currentNote = -1;
    int currentStart = 0;

    for (size_t i = 0; i < detectedNoteNumbers.size(); ++i)
    {
        int note = detectedNoteNumbers[i];

        if (note >= 0 && note == currentNote)
        {
            continue;
        }
        else
        {
            if (currentNote >= 0)
            {
                int length = static_cast<int>(i) - currentStart;
                if (length >= 3)
                {
                    sequences.push_back({ currentNote, length, currentStart });
                }
            }

            if (note >= 0)
            {
                currentNote = note;
                currentStart = static_cast<int>(i);
            }
            else
            {
                currentNote = -1;
            }
        }
    }

    // Check the last sequence
    if (currentNote >= 0)
    {
        int length = static_cast<int>(detectedNoteNumbers.size()) - currentStart;
        if (length >= 3)
        {
            sequences.push_back({ currentNote, length, currentStart });
        }
    }

    if (sequences.empty())
    {
        return juce::AudioBuffer<float>(inputAudioBuffer.getNumChannels(), 0);
    }

    // Find the best sequence
    int bestLength = -1;
    int bestNote = -1;
    int bestStart = -1;

    for (const auto& seq : sequences)
    {
        if (seq.length > bestLength ||
            (seq.length == bestLength && seq.note > bestNote) ||
            (seq.length == bestLength && seq.note == bestNote && seq.startIndex < bestStart))
        {
            bestLength = seq.length;
            bestNote = seq.note;
            bestStart = seq.startIndex;
        }
    }

    const int chunkSize = 1024;
    int startSample = (bestStart + 1) * chunkSize;
    int numSamples = (bestLength - 2) * chunkSize;

    // Ensure we don't exceed buffer bounds
    numSamples = juce::jmin(numSamples, inputAudioBuffer.getNumSamples() - startSample);

    if (numSamples <= 0)
    {
        return juce::AudioBuffer<float>(inputAudioBuffer.getNumChannels(), 0);
    }

    juce::AudioBuffer<float> result(inputAudioBuffer.getNumChannels(), numSamples);

    for (int ch = 0; ch < result.getNumChannels(); ++ch)
    {
        result.copyFrom(ch, 0, inputAudioBuffer, ch, startSample, numSamples);
    }

    return result;

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
