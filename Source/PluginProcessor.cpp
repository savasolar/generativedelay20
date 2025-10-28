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
    section1_audio.setSize(2, 1); // Dummy initial size
    section2_audio.setSize(2, 1);
    section3_audio.setSize(2, 1);
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
    offlineAnalysisBuffer.setSize(1, 1024, true);
    offlineFillPos = 0;

    section1_audio.setSize(getTotalNumInputChannels(), static_cast<int>(sampleRate * 60.0 + 0.5), true, true, true);
    section2_audio.setSize(getTotalNumInputChannels(), static_cast<int>(sampleRate * 60.0 + 0.5), true, true, true);
    section3_audio.setSize(getTotalNumInputChannels(), static_cast<int>(sampleRate * 60.0 + 0.5), true, true, true);
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

    // SECTION 1: RECORD INPUT

    // Record & store copy of input audio
    //  - triggered from editor with param values

    if (section1_isRecording.load())
    {
        int numSamples = buffer.getNumSamples();
        int spaceLeft = section1_samplesToRecord.load() - section1_writePos.load();
        int toCopy = juce::jmin(numSamples, spaceLeft);

        for (int ch = 0; ch < juce::jmin(getTotalNumInputChannels(), section1_audio.getNumChannels()); ++ch)
        {
            section1_audio.copyFrom(ch, section1_writePos.load(), buffer, ch, 0, toCopy);
        }

        section1_writePos.store(section1_writePos.load() + toCopy);  // like the position of the vertex of an hourglass relative to the level of remaining sand

        if (section1_writePos.load() >= section1_samplesToRecord.load())
        {
            section1_isRecording.store(false);
            section1_recordingComplete.store(true);
        }
    }

    // Play stored section1_buffer
    //  - triggered from ui with param values

    if (section1_isPlaying.load())
    {
        int numSamples = buffer.getNumSamples();
        int avail = section1_writePos.load() - section1_readPos.load();
        int toRead = juce::jmin(numSamples, avail);

        for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
        {
            int srcCh = ch % section1_audio.getNumChannels();
            buffer.addFrom(ch, 0, section1_audio, srcCh, section1_readPos.load(), toRead);
        }

        section1_readPos.store(section1_readPos.load() + toRead);

        if (section1_readPos.load() >= section1_writePos.load())
        {
            section1_isPlaying.store(false);
        }
    }

    // SECTION 2: ISOLATE LONGEST SINGLE NOTE

    // Convert section1_audio to section2_audio
    if (section2_isIsolating.load())
    {
        section2_audio = isolateBestNote(section1_audio);

        section2_writePos.store(section2_audio.getNumSamples());

        section2_isIsolating.store(false);
        section2_isolationComplete.store(true);
    }

    // Play stored section2_audio
    if (section2_isPlaying.load())
    {
        int numSamples = buffer.getNumSamples();
        int avail = section2_writePos.load() - section2_readPos.load();
        int toRead = juce::jmin(numSamples, avail);

        for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
        {
            int srcCh = ch % section2_audio.getNumChannels();
            buffer.addFrom(ch, 0, section2_audio, srcCh, section2_readPos.load(), toRead);
        }

        section2_readPos.store(section2_readPos.load() + toRead);

        if (section2_readPos.load() >= section2_writePos.load())
        {
            section2_isPlaying.store(false);
        }
    }
    
    // SECTION 3: TIME STRETCH
    
    if (section3_isTimeStretching.load())
    {
        section3_audio = timeStretch(section2_audio, section3_timeStretchLength);
        
        section3_writePos.store(section3_audio.getNumSamples());
        
        section3_isTimeStretching.store(false);
        section3_isTimeStretchComplete.store(true);
    }
    
    // Play stored section3_audio
    if (section3_isPlaying.load())
    {
        int numSamples = buffer.getNumSamples();
        int avail = section3_writePos.load() - section3_readPos.load();
        int toRead = juce::jmin(numSamples, avail);
        
        for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
        {
            int srcCh = ch % section3_audio.getNumChannels();
            buffer.addFrom(ch, 0, section3_audio, srcCh, section3_readPos.load(), toRead);
        }
        
        section3_readPos.store(section3_readPos.load() + toRead);
        
        if (section3_readPos.load() >= section3_writePos.load())
        {
            section3_isPlaying.store(false);
        }
    }

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
    // clear previous detections
    detectedNoteNumbers.clear();

    // detect pitches of input audio buffer

    juce::AudioBuffer<float> trimmedAudio;

    int numSamples = inputAudio.getNumSamples();
    int numChannels = inputAudio.getNumChannels();
    const float* const* inputData = inputAudio.getArrayOfReadPointers();

    offlineFillPos = 0;

    int pos = 0;
    while (pos < numSamples)
    {
        int spaceLeft = offlineAnalysisBuffer.getNumSamples() - offlineFillPos;
        int toCopy = juce::jmin(spaceLeft, numSamples - pos);

        // Mix to mono and copy
        for (int i = 0; i < toCopy; ++i)
        {
            float sample = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
            {
                sample += inputData[ch][pos + i];
            }
            sample /= static_cast<float>(numChannels);
            offlineAnalysisBuffer.setSample(0, offlineFillPos + i, sample);
        }

        offlineFillPos += toCopy;
        pos += toCopy;

        // if full, detect pitch and store
        if (offlineFillPos >= offlineAnalysisBuffer.getNumSamples())
        {
            float pitch = pitchDetector.getPitch(offlineAnalysisBuffer.getReadPointer(0));
            int note = frequencyToMidiNote(pitch);
            detectedNoteNumbers.push_back(note);  // Push even if -1

            // at the end, remove trailing -1's.

            while (!detectedNoteNumbers.empty() && detectedNoteNumbers.back() == -1)
            {
                detectedNoteNumbers.pop_back();
            }


            offlineFillPos = 0;
        }
    }

    

    // DBG print the detected note numbers
    juce::String noteStr = "Detected Note Numbers: ";
    for (int note : detectedNoteNumbers)
    {
        noteStr += juce::String(note) + ", ";
    }
    DBG(noteStr);



    // Now, find the longest consecutive sequence of identical positive MIDI notes
    // Positive means > 0 (since -1 is invalid)
    // If multiple with same length, pick the one with highest note value
    // If still tied, pick the first (earliest) occurrence
    // Invalid (-1) chunks break sequences; no merging across invalids

    if (detectedNoteNumbers.empty())
    {
        // No detections; return empty buffer and set MIDI to -1
        section2_midiValue.store(-1);
        return trimmedAudio;
    }








    int maxLength = 0;
    int bestNote = -1;
    int bestStart = -1;

    int currentStart = 0;
    int n = static_cast<int>(detectedNoteNumbers.size());

    for (int i = 1; i <= n; ++i)
    {
        // Check if end of sequence or end of vector
        if (i == n || detectedNoteNumbers[i] != detectedNoteNumbers[currentStart] || detectedNoteNumbers[i] <= 0 || detectedNoteNumbers[currentStart] <= 0)
        {
            // Evaluate the previous run only if it was a valid positive note
            if (detectedNoteNumbers[currentStart] > 0)
            {
                int currentLength = i - currentStart;
                bool isBetter = false;

                if (currentLength > maxLength)
                {
                    isBetter = true;
                }
                else if (currentLength == maxLength)
                {
                    if (detectedNoteNumbers[currentStart] > bestNote)
                    {
                        isBetter = true;
                    }
                    else if (detectedNoteNumbers[currentStart] == bestNote && currentStart < bestStart)
                    {
                        isBetter = true;
                    }
                }

                if (isBetter)
                {
                    maxLength = currentLength;
                    bestNote = detectedNoteNumbers[currentStart];
                    bestStart = currentStart;
                }
            }

            // Start new run
            currentStart = i;
        }
    }

    // If no valid sequence found, return empty and set MIDI to 0
    if (maxLength == 0)
    {
        section2_midiValue.store(0);
        return trimmedAudio;
    }

    // Exclude first and last chunk to reduce boundary bleed, if possible
    if (maxLength > 2)
    {
        bestStart += 1;
        maxLength -= 2;
    }

    // Calculate sample positions (each chunk is 1024 samples)
    const int chunkSize = 1024;
    int sampleStart = bestStart * chunkSize;
    int numSamplesToCopy = maxLength * chunkSize;

    // Ensure we don't exceed input length (though it should not, since detections cover full chunks)
    numSamplesToCopy = juce::jmin(numSamplesToCopy, numSamples - sampleStart);

    // Create trimmed audio buffer with original number of channels
    trimmedAudio.setSize(numChannels, numSamplesToCopy, false, true, true);

    // Copy the selected segment from inputAudio to trimmedAudio
    for (int ch = 0; ch < numChannels; ++ch)
    {
        trimmedAudio.copyFrom(ch, 0, inputAudio, ch, sampleStart, numSamplesToCopy);
    }

    // Set the detected MIDI value for the isolated note
    section2_midiValue.store(bestNote);













    return trimmedAudio;
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

//    float** inputPointers = inputAudio.getArrayOfWritePointers();
//    float** outputPointers = timeStretchedAudio.getArrayOfWritePointers();

    float** inputPointers = const_cast<float**>(inputAudio.getArrayOfWritePointers());
    float** outputPointers = const_cast<float**>(timeStretchedAudio.getArrayOfWritePointers());


    stretcher.process(inputPointers, inputSamples, outputPointers, outputSamples);


    
    return timeStretchedAudio;
}






bool EnCounterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EnCounterAudioProcessor::createEditor()
{
    return new EnCounterAudioProcessorEditor (*this);
}

//==============================================================================
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
