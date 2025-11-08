// PluginProcessor.cpp

#include "PluginProcessor.h"
#include "PluginEditor.h"

CounterTuneAudioProcessor::CounterTuneAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
//    pitchDetector(44100, 1024),
    parameters(*this, nullptr, "Parameters",
        {
            std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"tempo", 1}, "Tempo", 1, 999, 120),
            std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"beats", 1}, "Beats", 1.0f, 16.0f, 8.0f),
            std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"notes", 1}, "Notes", 1, 16, 8),
            std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"octave", 1}, "Octave", -4, 4, 0),
            std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"detune", 1}, "Detune", -1.0f, 1.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"mix", 1}, "Mix", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"loop", 1}, "Loop", 0)
        })
#endif
{
    inputAudioBuffer.setSize(2, 1); // dummy size for now

    dywapitch_inittracking(&pitchTracker);

    generatedMelody = lastGeneratedMelody;
}

CounterTuneAudioProcessor::~CounterTuneAudioProcessor()
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

const juce::String CounterTuneAudioProcessor::getName() const
{
    return JucePlugin_Name;
}
bool CounterTuneAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}
bool CounterTuneAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}
bool CounterTuneAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
double CounterTuneAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}
int CounterTuneAudioProcessor::getNumPrograms()
{
    return 1;
}
int CounterTuneAudioProcessor::getCurrentProgram()
{
    return 0;
}
void CounterTuneAudioProcessor::setCurrentProgram (int index)
{
}
const juce::String CounterTuneAudioProcessor::getProgramName (int index)
{
    return {};
}
void CounterTuneAudioProcessor::changeProgramName (int index, const juce::String& newName)
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

void CounterTuneAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{    
//    pitchDetector.setSampleRate(sampleRate);
//    pitchDetector.setBufferSize(1024);  // Fixed power-of-2; or use juce::nextPowerOfTwo(samplesPerBlock) if you want dynamic

    analysisBuffer.setSize(1, 1024, true);  // Mono, matches bufferSize, keep data
    pitchDetectorFillPos = 0;  // Reset accumulator

    stretcher.presetCheaper(getTotalNumInputChannels(), static_cast<float>(sampleRate));

    inputAudioBuffer.setSize(getTotalNumInputChannels(), static_cast<int>(sampleRate * 60.0 + 0.5), true, true, true);


    dryWetMixer.prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<std::uint32_t> (samplesPerBlock), static_cast<std::uint32_t> (getTotalNumOutputChannels()) });
    dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    dryWetMixer.setWetMixProportion(0.5f);



    adsr.setSampleRate(sampleRate);

    resetTiming();

}

//              (\_/)
//      .-""-.-.-' a\
//     /  \      _.--'
//    (\  /_---\\_\_
//     `'-.
//jgs   ,__)

void CounterTuneAudioProcessor::releaseResources()
{

}
#ifndef JucePlugin_PreferredChannelConfigurations
bool CounterTuneAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void CounterTuneAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // FIGURE OUT THE MOST USEFUL OF DBG VISUAL OUTPUT


    if (!isActive.load())
    {
        detectSound(buffer);
        resetTiming();
//        isActive.store(true);

    }
    else
    {
        // 1/6: RECORD INPUT AUDIO BUFFER====================================================================================

        int numSamples = buffer.getNumSamples();

        int spaceLeft = inputAudioBuffer_samplesToRecord.load() - inputAudioBuffer_writePos.load();
        int toCopy = juce::jmin(numSamples, spaceLeft);

        for (int ch = 0; ch < juce::jmin(getTotalNumInputChannels(), inputAudioBuffer.getNumChannels()); ++ch)
        {
            inputAudioBuffer.copyFrom(ch, inputAudioBuffer_writePos.load(), buffer, ch, 0, toCopy);
        }

        inputAudioBuffer_writePos.store(inputAudioBuffer_writePos.load() + toCopy);


        // 2/6: PITCH DETECTION====================================================================================

        // if possible, without affecting the recording of input audio, maximize the volume of input audio for pitch detection on as granular a level as possible; maybe on a chunk-by-chunk basis


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
//            float pitch = pitchDetector.getPitch(analysisBuffer.getReadPointer(0));
//            int midiNote = frequencyToMidiNote(pitch);



            // it needs a noise gate



            // DYWAPitchTrack uses double*, but analysisBuffer is float*. Convert temporarily.
            std::vector<double> doubleSamples(1024);
            for (int i = 0; i < 1024; ++i)
                doubleSamples[i] = analysisBuffer.getSample(0, i);

            // Compute pitch (returns Hz, or 0.0 if no pitch detected).
            double pitch = dywapitch_computepitch(&pitchTracker, doubleSamples.data(), 0, 1024);

            // Scale for your sample rate (DYWAPitchTrack assumes 44100 Hz).
            pitch *= (getSampleRate() / 44100.0);

            int midiNote = frequencyToMidiNote(static_cast<float>(pitch));






            detectedNoteNumbers.push_back(midiNote);


            pitchDetectorFillPos = 0;
        }

        // Handle overflow
        if (analysisToCopy < numSamples)
        {
            analysisBuffer.copyFrom(0, 0, monoData + analysisToCopy, numSamples - analysisToCopy);
            pitchDetectorFillPos = numSamples - analysisToCopy;
        }

        // 3/6: CAPTURED MELODY TRANSCRIPTION====================================================================================

        int captureSpaceLeft = (sPs * 32) - melodyCaptureFillPos;
        int captureToCopy = juce::jmin(captureSpaceLeft, numSamples);

        for (int n = 0; n < 32; ++n)
        {
            // fill captured melody at middle of a symbol
            if (melodyCaptureFillPos >= (n + 0.5) * sPs)
            {
                if (!symbolExecuted.test(n))
                {
                    if (!detectedNoteNumbers.empty())
                    {
                        capturedMelody[n] = detectedNoteNumbers.back();
//						juce::String noteStrA = "Dnn: "; for (int note : detectedNoteNumbers) { noteStrA += juce::String(note) + ", "; } DBG(noteStrA);
                        juce::String noteStrB = "cM: "; for (int note : capturedMelody) { noteStrB += juce::String(note) + ", "; } DBG(noteStrB);
                    }
                    sampleDrift = static_cast<int>(std::round(32.0 * (60.0 / placeholderBpm * getSampleRate() / 4.0 * placeholderBeats / 8.0 - sPs)));
                    symbolExecuted.set(n);
                }
            }
        }

        // 4/6: GENERATED MELODY SYMBOL READING====================================================================================

        for (int n = 0; n < 32; ++n)
        {
            if (melodyCaptureFillPos >= n * sPs)
            {
                if (!playbackSymbolExecuted.test(n))
                {
                    useADSR.store(false);

                    // prepare a note for playback if there's a note number
                    if (generatedMelody[n] >= 0)
                    {
                        finalVoiceBuffer = pitchShiftByResampling(voiceBuffer, voiceNoteNumber.load(), generatedMelody[n]);
                        finalVoiceBuffer_readPos.store(0);
                    }

                    // if next generatedMelody symbol indicates a fadeout in the current symbol is needed, activate useADSR here

                    if ((generatedMelody[(n + 1) % 32]) >= -1)
                    {
                        useADSR.store(true);
                    }

                    if (useADSR.load())
                    {
                        adsr.reset();
                        adsr.noteOn();
                        adsr.noteOff();
                    }

                    playbackSymbolExecuted.set(n);
                }
            }
        }

        melodyCaptureFillPos += captureToCopy;

        // 5/6: END OF A CYCLE====================================================================================

        if (melodyCaptureFillPos >= sPs * 32 + sampleDrift)
        {
            symbolExecuted.reset();
            playbackSymbolExecuted.reset();
            fractionalSymbolExecuted.reset();

            // if captured melody is empty
            if (std::all_of(capturedMelody.begin(), capturedMelody.end(), [](int n) { return n == -1; }))
            {
                isActive.store(false);
                voiceBuffer.clear();
                finalVoiceBuffer.clear();
            }
            else
            {

                detectKey(capturedMelody);

            }

            // populate voice buffer with latest info 
//            juce::AudioBuffer<float> tempVoiceBuffer = isolateBestNote();
//            timeStretch(tempVoiceBuffer, static_cast<float>(16 * sPs) / getSampleRate()); // this is async btw

            resetTiming();
        }
    }


    // 6/6: SYNTHESIZED PLAYBACK AND MIXING====================================================================================

    juce::dsp::AudioBlock<float> block(buffer);
    dryWetMixer.pushDrySamples(block);
    block.clear();

    if (finalVoiceBuffer.getNumSamples() > 0)
    {
        int numSamples = buffer.getNumSamples();
        int voiceBufferSize = finalVoiceBuffer.getNumSamples();
        int readPos = finalVoiceBuffer_readPos.load();

        for (int i = 0; i < numSamples; ++i)
        {
            int currentPos = readPos + i;

            if (currentPos >= voiceBufferSize) break;

            float gain = useADSR.load() ? adsr.getNextSample() : 1.0f;

            for (int ch = 0; ch < juce::jmin(buffer.getNumChannels(), finalVoiceBuffer.getNumChannels()); ++ch)
            {
                buffer.addSample(ch, i, finalVoiceBuffer.getSample(ch, currentPos) * gain);
            }
        }

        finalVoiceBuffer_readPos.store(readPos + numSamples);
    }

    dryWetMixer.mixWetSamples(block);

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

void CounterTuneAudioProcessor::detectSound(const juce::AudioBuffer<float>& buffer)
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

    const float threshold = 0.0056f;  // Tune this: lower = more sensitive (e.g., 0.0056f for -45 dBFS)

    if (rms > threshold)
    {
        DBG("sound detected");
        isActive.store(true);
    }
}

int CounterTuneAudioProcessor::frequencyToMidiNote(float frequency)
{
    if (frequency <= 0.0f)
    {
        return -1;
    }

    return static_cast<int>(std::round(12.0f * std::log2(frequency / 440.0f) + 69.0f));
}

juce::AudioBuffer<float> CounterTuneAudioProcessor::isolateBestNote()
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


    // Store the best note number and DBG print it
    voiceNoteNumber.store(bestNote);
    DBG("Isolated best note number: " + juce::String(voiceNoteNumber.load()));



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

void CounterTuneAudioProcessor::timeStretch(juce::AudioBuffer<float> inputAudio, int length)
{
    std::thread t([this, inputAudio = std::move(inputAudio), length]() mutable
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

        // Isolate middle 50% here.
        int trimStart = static_cast<int>(outputSamples * 0.25f);  // Left 25%
        int trimLength = static_cast<int>(outputSamples * 0.5f);  // Middle 50%
        // Ensure we don't exceed bounds (though unlikely).
        trimStart = juce::jmax(0, trimStart);
        trimLength = juce::jmin(trimLength, outputSamples - trimStart);

        if (trimLength > 0)
        {
            juce::AudioBuffer<float> trimmedAudio(channels, trimLength);
            for (int ch = 0; ch < channels; ++ch)
            {
                trimmedAudio.copyFrom(ch, 0, timeStretchedAudio, ch, trimStart, trimLength);
            }
            this->voiceBuffer = std::move(trimmedAudio);

        }
        else
        {
            // Fallback: Use full buffer if trimLength is invalid (rare).
            this->voiceBuffer = std::move(timeStretchedAudio);


        }

        // Apply 50ms linear fade-in and fade-out
        int numSamples = voiceBuffer.getNumSamples();
        if (numSamples > 0) {
            int fadeSamples = static_cast<int>(0.01 * getSampleRate() + 0.5f);
            fadeSamples = juce::jmin(fadeSamples, numSamples / 2);
            for (int ch = 0; ch < voiceBuffer.getNumChannels(); ++ch) {
                voiceBuffer.applyGainRamp(ch, 0, fadeSamples, 0.0f, 1.0f);
                voiceBuffer.applyGainRamp(ch, numSamples - fadeSamples, fadeSamples, 1.0f, 0.0f);
            }
        }

        DBG("new voice buffer ready");

    });
    t.detach();
}

juce::AudioBuffer<float> CounterTuneAudioProcessor::pitchShiftByResampling(const juce::AudioBuffer<float>& input, int baseNote, int targetNote)
{
    if (input.getNumSamples() == 0 || baseNote < 0 || targetNote < 0)
    {
        return juce::AudioBuffer<float>(input.getNumChannels(), 0);
    }

    // Calculate pitch ratio (semitones to frequency ratio)
    float semitoneShift = static_cast<float>(targetNote - baseNote);
    float pitchRatio = std::pow(2.0f, semitoneShift / 12.0f);

    int numChannels = input.getNumChannels();
    int inputSamples = input.getNumSamples();
    int outputSamples = static_cast<int>(inputSamples / pitchRatio + 0.5f);

    if (outputSamples <= 0)
    {
        return juce::AudioBuffer<float>(numChannels, 0);
    }

    juce::AudioBuffer<float> output(numChannels, outputSamples);

    // Linear interpolation resampling
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* inputData = input.getReadPointer(ch);
        float* outputData = output.getWritePointer(ch);

        for (int i = 0; i < outputSamples; ++i)
        {
            float readPos = i * pitchRatio;
            int readIndex = static_cast<int>(readPos);
            float frac = readPos - readIndex;

            if (readIndex < inputSamples - 1)
            {
                // Linear interpolation between samples
                outputData[i] = inputData[readIndex] * (1.0f - frac) +
                    inputData[readIndex + 1] * frac;
            }
            else if (readIndex < inputSamples)
            {
                outputData[i] = inputData[readIndex];
            }
            else
            {
                outputData[i] = 0.0f;
            }
        }
    }

    //// Apply short fade-in/out to eliminate clicks (5ms each)
    //int fadeSamples = static_cast<int>(0.005 * getSampleRate() + 0.5f);
    //fadeSamples = juce::jmin(fadeSamples, outputSamples / 4); // Max 25% of buffer

    //if (fadeSamples > 0)
    //{
    //    for (int ch = 0; ch < numChannels; ++ch)
    //    {
    //        output.applyGainRamp(ch, 0, fadeSamples, 0.0f, 1.0f);
    //        output.applyGainRamp(ch, outputSamples - fadeSamples, fadeSamples, 1.0f, 0.0f);
    //    }
    //}

    return output;
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

std::vector<int> CounterTuneAudioProcessor::formatMelody(const std::vector<int>& melody, bool isGeneratedMelody) const
{
    std::vector<int> formattedMelody = melody;

    // Rule 1: Rotate so it doesnt start with -1 or -2
    auto it = std::find_if(formattedMelody.begin(), formattedMelody.end(), [](int n) { return n >= 0; });

    if (it != formattedMelody.end())
    {
        std::rotate(formattedMelody.begin(), it, formattedMelody.end());
    }

    // Rule 2: Replace consecutive -1s with -2, keeping the first -1
    for (size_t i = 0; i < formattedMelody.size(); ++i)
    {
        if (formattedMelody[i] == -1)
        {
            // Replace all subsequent consecutive -1s with -2
            for (size_t j = i + 1; j < formattedMelody.size() && formattedMelody[j] == -1; ++j)
            {
                formattedMelody[j] = -2;
            }
        }
    }

    // Rule 3: Replace consecutive identical notes with -2, keeping the first
    for (size_t i = 0; i < formattedMelody.size(); ++i)
    {
        if (formattedMelody[i] >= 0)
        {
            int currentNote = formattedMelody[i];
            // Replace all subsequent consecutive identical notes with -2
            for (size_t j = i + 1; j < formattedMelody.size() && formattedMelody[j] == currentNote; ++j)
            {
                formattedMelody[j] = -2;
            }
        }
    }

    // Rule 4: Eliminate redundant note-off events
    // After a note-off (-1), replace any subsequent -1s with -2 until the next note (>= 0)
    for (size_t i = 0; i < formattedMelody.size(); ++i)
    {
        if (formattedMelody[i] == -1)
        {
            // Replace all subsequent -1s with -2 until we hit a note (>= 0)
            for (size_t j = i + 1; j < formattedMelody.size() && formattedMelody[j] < 0; ++j)
            {
                if (formattedMelody[j] == -1)
                {
                    formattedMelody[j] = -2;
                }
            }
        }
    }

    return formattedMelody; // Return the new vector
}

void CounterTuneAudioProcessor::detectKey(const std::vector<int>& melody)
{
    std::array<double, 12> hist{};
    double total = 0.0;
    for (int note : melody)
    {
        if (note >= 0)
        {
            int pc = note % 12;
            hist[pc] += 1.0;
            total += 1.0;
        }
    }

    if (total == 0.0)
    {
        DBG("Detected key: Unknown");
        return;
    }

    // Normalize histogram to probabilities
    for (auto& v : hist) v /= total;

    // KS reference profile for C major
    std::array<double, 12> c_profile{ 6.35, 2.18, 3.48, 2.14, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88 };

    // Pearson correlation function
    auto pearson = [](const std::array<double, 12>& a, const std::array<double, 12>& b) -> double
        {
            double meanA = 0.0, meanB = 0.0;
            for (double x : a) meanA += x;
            for (double x : b) meanB += x;
            meanA /= 12.0;
            meanB /= 12.0;

            double cov = 0.0, varA = 0.0, varB = 0.0;
            for (size_t i = 0; i < 12; ++i)
            {
                double da = a[i] - meanA;
                double db = b[i] - meanB;
                cov += da * db;
                varA += da * da;
                varB += db * db;
            }

            if (varA == 0.0 || varB == 0.0) return 0.0;
            return cov / std::sqrt(varA * varB);
        };

    double max_r = -1.0;
    int best_tonic = -1;
    for (int tonic = 0; tonic < 12; ++tonic)
    {
        std::array<double, 12> shifted;
        for (size_t i = 0; i < 12; ++i)
        {
            shifted[i] = hist[(i + tonic) % 12];
        }
        double r = pearson(shifted, c_profile);
        if (r > max_r)
        {
            max_r = r;
            best_tonic = tonic;
        }
    }

    //juce::StringArray keys{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    //juce::String key = keys[best_tonic] + " major";
    //DBG("Detected key: " + key);

    std::array<int, 12> keys{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    int key = keys[best_tonic];
    DBG(key);

    if (key == 0)
        generatedMelody = { 60, -2, -2, -2, 62, -2, -2, -2, 64, -2, -2, -2, 65, -2, -2, -2, 67, -2, -2, -2, 69, -2, -2, -2, 71, -2, -2, -2, 72, -2, -2, -2 };
    if (key == 1)
        generatedMelody = { 61, -2, -2, -2, 63, -2, -2, -2, 65, -2, -2, -2, 66, -2, -2, -2, 68, -2, -2, -2, 70, -2, -2, -2, 72, -2, -2, -2, 73, -2, -2, -2 };
    if (key == 2)
        generatedMelody = { 62, -2, -2, -2, 64, -2, -2, -2, 66, -2, -2, -2, 67, -2, -2, -2, 69, -2, -2, -2, 71, -2, -2, -2, 73, -2, -2, -2, 74, -2, -2, -2 };
    if (key == 3)
        generatedMelody = { 63, -2, -2, -2, 65, -2, -2, -2, 67, -2, -2, -2, 68, -2, -2, -2, 70, -2, -2, -2, 72, -2, -2, -2, 74, -2, -2, -2, 75, -2, -2, -2 };
    if (key == 4)
        generatedMelody = { 64, -2, -2, -2, 66, -2, -2, -2, 68, -2, -2, -2, 69, -2, -2, -2, 71, -2, -2, -2, 73, -2, -2, -2, 75, -2, -2, -2, 76, -2, -2, -2 };
    if (key == 5)
        generatedMelody = { 65, -2, -2, -2, 67, -2, -2, -2, 69, -2, -2, -2, 70, -2, -2, -2, 72, -2, -2, -2, 74, -2, -2, -2, 76, -2, -2, -2, 77, -2, -2, -2 };
    if (key == 6)
        generatedMelody = { 66, -2, -2, -2, 68, -2, -2, -2, 70, -2, -2, -2, 71, -2, -2, -2, 73, -2, -2, -2, 75, -2, -2, -2, 77, -2, -2, -2, 78, -2, -2, -2 };
    if (key == 7)
        generatedMelody = { 67, -2, -2, -2, 69, -2, -2, -2, 71, -2, -2, -2, 72, -2, -2, -2, 74, -2, -2, -2, 76, -2, -2, -2, 78, -2, -2, -2, 79, -2, -2, -2 };
    if (key == 8)
        generatedMelody = { 68, -2, -2, -2, 70, -2, -2, -2, 72, -2, -2, -2, 73, -2, -2, -2, 75, -2, -2, -2, 77, -2, -2, -2, 79, -2, -2, -2, 80, -2, -2, -2 };
    if (key == 9)
        generatedMelody = { 69, -2, -2, -2, 71, -2, -2, -2, 73, -2, -2, -2, 74, -2, -2, -2, 76, -2, -2, -2, 78, -2, -2, -2, 80, -2, -2, -2, 81, -2, -2, -2 };
    if (key == 10)
        generatedMelody = { 70, -2, -2, -2, 72, -2, -2, -2, 74, -2, -2, -2, 75, -2, -2, -2, 77, -2, -2, -2, 79, -2, -2, -2, 81, -2, -2, -2, 82, -2, -2, -2 };
    if (key == 11)
        generatedMelody = { 71, -2, -2, -2, 73, -2, -2, -2, 75, -2, -2, -2, 76, -2, -2, -2, 78, -2, -2, -2, 80, -2, -2, -2, 82, -2, -2, -2, 83, -2, -2, -2 };
}

bool CounterTuneAudioProcessor::hasEditor() const
{
    return true;
}
juce::AudioProcessorEditor* CounterTuneAudioProcessor::createEditor()
{
    return new CounterTuneAudioProcessorEditor (*this);
}
void CounterTuneAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{

}
void CounterTuneAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{

}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CounterTuneAudioProcessor();
}
