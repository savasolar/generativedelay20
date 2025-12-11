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
    parameters(*this, nullptr, "Parameters",
        {
            std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"tempo", 1}, "Tempo", 1, 999, 120),
            std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"beats", 1}, "Beats", 1.0f, 16.0f, 8.0f),
            std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"key", 1}, "Key", 0, 12, 12),//12=auto, 0=c, ... 11=b
            std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"notes", 1}, "Notes", 1, 16, 8),
            std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"chaos", 1}, "Chaos", 1, 10, 5),
            std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"octave", 1}, "Octave", -4, 4, 0),
            std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"detune", 1}, "Detune", -1.0f, 1.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"mix", 1}, "Mix", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"loop", 1}, "Loop", 0),
            std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"preset", 1}, "Preset", 1, 5, 1)
        })
#endif
{
    inputAudioBuffer.setSize(2, 1); // dummy size for now
    dywapitch_inittracking(&pitchTracker);
    generatedMelody = lastGeneratedMelody;

    DBG("DEV LEMONS IS HOT");
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
    analysisBuffer.setSize(1, 1024, true);  // Mono, matches bufferSize, keep data
    pitchDetectorFillPos = 0;  // Reset accumulator

    stretcher.presetCheaper(getTotalNumInputChannels(), static_cast<float>(sampleRate));

    inputAudioBuffer.setSize(getTotalNumInputChannels(), static_cast<int>(sampleRate * 60.0 + 0.5), true, true, true);


    dryWetMixer.prepare(juce::dsp::ProcessSpec{ sampleRate, static_cast<std::uint32_t> (samplesPerBlock), static_cast<std::uint32_t> (getTotalNumOutputChannels()) });
    dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::balanced);

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

#ifdef DEMO_BUILD
    if (isDemoExpired || exportMode.load())
    {
        buffer.clear();
        return;
    }
#endif

    synchronizeBpm();

    if (!isActive.load())
    {
        resetTiming();
        bool check = detectSound(buffer);
        isActive.store(check);
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
            // DYWAPitchTrack uses double*, but analysisBuffer is float*. Convert temporarily.
            std::vector<double> doubleSamples(1024);
            for (int i = 0; i < 1024; ++i)
                doubleSamples[i] = analysisBuffer.getSample(0, i);

            // Compute pitch (returns Hz, or 0.0 if no pitch detected).
            double pitch = dywapitch_computepitch(&pitchTracker, doubleSamples.data(), 0, 1024);
            pitch *= (getSampleRate() / 44100.0); // Scale for DYWAPitchTrack's 44100 assumption
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

        int captureSpaceLeft = (sPs * 32 + std::max(sampleDrift, 0)) - melodyCaptureFillPos;

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
//                        juce::String noteStrB = "cM: "; for (int note : capturedMelody) { noteStrB += juce::String(note) + ", "; } DBG(noteStrB);
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
//                        finalVoiceBuffer = pitchShiftByResampling(voiceBuffer, voiceNoteNumber.load(), generatedMelody[n]);
//                        finalVoiceBuffer_readPos.store(0);

                        int shiftedNote = generatedMelody[n] + getOctaveInt() * 12;
                        finalVoiceBuffer = pitchShiftByResampling(voiceBuffer, voiceNoteNumber.load(), shiftedNote);
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
            DBG("CYCLE END");

#ifdef DEMO_BUILD
            if (demoCounter >= 50)
                isDemoExpired = true;
#endif

            symbolExecuted.reset();
            playbackSymbolExecuted.reset();
            fractionalSymbolExecuted.reset();

            // if captured melody is empty
            if (std::all_of(capturedMelody.begin(), capturedMelody.end(), [](int n) { return n == -1; }))
            {
                isActive.store(false);
                voiceBuffer.clear();
                finalVoiceBuffer.clear();

                if (!getLoopBool())
                {
                    std::fill(generatedMelody.begin(), generatedMelody.end(), -1);
                }
            }
            else
            {
                //// Debug print capturedmelody
                //juce::String debugCapturedMelody = "capturedMelody: ";
                //for (const int& event : capturedMelody)
                //{
                //    debugCapturedMelody += juce::String(event) + " ";
                //}
                //DBG(debugCapturedMelody);

                DBG("starting key detection");

                detectKey(capturedMelody); // this takes an unformatted capturedMelody with lots of repetitive numbers and -1s but no -2s

                if (!getLoopBool() && !isDemoExpired)
                {
                    produceMelody(capturedMelody, getKeyInt(), getNotesInt(), getChaosInt());
#ifdef DEMO_BUILD
                    demoCounter += 1;
#endif
                }

            }

            lastGeneratedMelody = generatedMelody;

            // populate voice buffer with latest info 
            juce::AudioBuffer<float> tempVoiceBuffer = isolateBestNote();

            // <realtime detection> ---- call this BEFORE time stretch
            double currentTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
            double elapsedWallTime = currentTime - cycleStartTime;  // Actual elapsed time (wall-clock)

            double expectedAudioTime = static_cast<double>(inputAudioBuffer_writePos.load()) / getSampleRate();  // Audio duration based on samples

            if (elapsedWallTime < expectedAudioTime * 0.5)  // Threshold: if wall time < 50% of audio time
            {
                exportMode.store(true);  // Set to non-realtime (export/offline)
                DBG("non-realtime");
            }
            else
            {
                exportMode.store(false);  // Set to realtime
                DBG("realtime");
            }
            // </realtime detection>

            // time stretch
            timeStretch(tempVoiceBuffer, static_cast<float>(16 * sPs) / getSampleRate());


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


    dryWetMixer.setWetMixProportion(getMixFloat());

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

bool CounterTuneAudioProcessor::detectSound(const juce::AudioBuffer<float>& buffer)
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

    bool result = false;

    if (rms > threshold)
    {
        result = true;
    }

    return result;
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

    newVoiceNoteNumber.store(bestNote);

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
void CounterTuneAudioProcessor::timeStretch(juce::AudioBuffer<float> inputAudio, float lengthSeconds)
{
    if (!exportMode.load())
    {
        std::thread t([this, inputAudio = std::move(inputAudio), lengthSeconds]() mutable
            {
                using Stretch = signalsmith::stretch::SignalsmithStretch<float>;

                Stretch stretcher;

                int channels = inputAudio.getNumChannels();
                float sampleRateFloat = static_cast<float>(getSampleRate());

                stretcher.presetDefault(channels, sampleRateFloat);

                int inputSamples = inputAudio.getNumSamples();
                int outputSamples = static_cast<int>(lengthSeconds * getSampleRate() + 0.5f);

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
                if (numSamples > 0)
                {
                    int fadeSamples = static_cast<int>(0.01 * getSampleRate() + 0.5f);
                    fadeSamples = juce::jmin(fadeSamples, numSamples / 2);
                    for (int ch = 0; ch < voiceBuffer.getNumChannels(); ++ch) {
                        voiceBuffer.applyGainRamp(ch, 0, fadeSamples, 0.0f, 1.0f);
                        voiceBuffer.applyGainRamp(ch, numSamples - fadeSamples, fadeSamples, 1.0f, 0.0f);
                    }
                }

                // needs some kind of hard limiter here

                voiceNoteNumber.store(newVoiceNoteNumber);

                //        DBG("dev lemons is really cute");

            });
        t.detach();
    }
    else
    {
        using Stretch = signalsmith::stretch::SignalsmithStretch<float>;

        Stretch stretcher;

        int channels = inputAudio.getNumChannels();
        float sampleRateFloat = static_cast<float>(getSampleRate());

        stretcher.presetDefault(channels, sampleRateFloat);

        int inputSamples = inputAudio.getNumSamples();
        int outputSamples = static_cast<int>(lengthSeconds * getSampleRate() + 0.5f);

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
        if (numSamples > 0)
        {
            int fadeSamples = static_cast<int>(0.01 * getSampleRate() + 0.5f);
            fadeSamples = juce::jmin(fadeSamples, numSamples / 2);
            for (int ch = 0; ch < voiceBuffer.getNumChannels(); ++ch) {
                voiceBuffer.applyGainRamp(ch, 0, fadeSamples, 0.0f, 1.0f);
                voiceBuffer.applyGainRamp(ch, numSamples - fadeSamples, fadeSamples, 1.0f, 0.0f);
            }
        }

        // needs some kind of hard limiter here

        voiceNoteNumber.store(newVoiceNoteNumber);

        //        DBG("dev lemons is really cute");
    }

}
juce::AudioBuffer<float> CounterTuneAudioProcessor::pitchShiftByResampling(const juce::AudioBuffer<float>& input, int baseNote, int targetNote)
{
    if (input.getNumSamples() == 0 || baseNote < 0 || targetNote < 0)
    {
        return juce::AudioBuffer<float>(input.getNumChannels(), 0);
    }

    // Calculate pitch ratio (semitones to frequency ratio)
//    float semitoneShift = static_cast<float>(targetNote - baseNote);

    float semitoneShift = static_cast<float>(targetNote - baseNote) + getDetuneFloat();

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
                outputData[i] = inputData[readIndex] * (1.0f - frac) + inputData[readIndex + 1] * frac;
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
    /*std::array<double, 12> hist{};
    double total = 0.0;
    for (int note : melody)
    {
        if (note >= 0)
        {
            int pc = note % 12;
            hist[pc] += 1.0;
            total += 1.0;
        }
    }*/

    // Ignore repeated instances of the same note number
    std::array<double, 12> hist{};
    double total = 0.0;
    int last_note = -1;  // Track the last valid note to skip consecutives
    for (int note : melody)
    {
        if (note >= 0)
        {
            if (note != last_note)
            {
                int pc = note % 12;
                hist[pc] += 1.0;
                total += 1.0;
                last_note = note;
            }
        }
        else
        {
            last_note = -1;  // Reset on gaps (-1) to allow same note later
        }
    }

    juce::String histStr = "Histogram (consecutive deduped): [";
    for (int i = 0; i < 12; ++i)
        histStr += juce::String(hist[i], 1) + (i < 11 ? ", " : "");
    histStr += "]  total = " + juce::String(total);
    DBG(histStr);



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
    detectedKey = key;
}
void CounterTuneAudioProcessor::magnetize(std::vector<int>& melody, float probability) const
{
    if (probability <= 0.0f) return;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    size_t size = melody.size();
    for (size_t i = 1; i < size; i += 2)
    {
        if (melody[i] >= 0)
        {
            if (dis(gen) >= probability) continue;

            ssize_t lower = static_cast<ssize_t>(i) - 1;
            ssize_t higher = static_cast<ssize_t>(i) + 1;

            bool lower_free = (lower >= 0 && melody[lower] < 0);
            bool higher_free = (higher < static_cast<ssize_t>(size) && melody[higher] < 0);

            if (!lower_free && !higher_free)
            {
                continue;
            }

            ssize_t target = -1;
            if (lower_free && higher_free)
            {
                if (dis(gen) < 0.5f)
                {
                    target = lower;
                }
                else
                {
                    target = higher;
                }
            }
            else if (lower_free)
            {
                target = lower;
            }
            else
            {
                target = higher;
            }

            melody[target] = melody[i];
            melody[i] = -2;
        }
    }
}
void CounterTuneAudioProcessor::copyMelodiesTo(std::vector<int>& outCaptured, std::vector<int>& outGenerated) const
{
    juce::ScopedLock sl(melodyLock);
    outCaptured = capturedMelody;
    outGenerated = generatedMelody;
}
void CounterTuneAudioProcessor::produceMelody(const std::vector<int>& melody, int key, int notes, int chaos)
{
    std::vector<int> result;

    // GIVEN AN INPUT MELODY, PRODUCE AN OUTPUT MELODY

    std::vector<int> scale{};
    int rhythm = 0;

    // determine acceptable scale and rhythmic arrangement based on chaos value

    if (chaos == 1)
    {
        scale = { 7 };
        rhythm = 1; // straight quater notes
    }
    if (chaos == 2)
    {
        scale = { 4, 7 };
        rhythm = 1; // straight quater notes
    }
    if (chaos == 3)
    {
        scale = { 2, 4, 7 };
        rhythm = 1; // straight quater notes
    }
    if (chaos == 4)
    {
        scale = { 2, 4, 7, 9, 11 };
        rhythm = 1; // straight quater notes
    }
    if (chaos == 5)
    {
        scale = { 2, 4, 7, 9, 11 };
        rhythm = 2; // straight eighth notes
    }
    if (chaos == 6)
    {
        scale = { 0, 2, 4, 5, 7, 9, 11, 12 };
        rhythm = 2; // straight eighth notes        
    }
    if (chaos == 7)
    {
        scale = { 0, 2, 4, 5, 7, 9, 11, 12 };
        rhythm = 3; // intelligent rhytmic determinism
    }
    if (chaos == 8)
    {

        scale = { 0, 2, 4, 5, 7, 9, 11, 12, /*disjunct intervals:*/ /*6*/ /*or, alternatively:*/ 17};
        rhythm = 3;
    }
    if (chaos == 9)
    {
        scale = { 0, 2, 4, 5, 7, 9, 11, 12, /*disjunct intervals:*/ /*1, 6, 10*/ /*or, alternatively:*/ 14, 17, 23 };
        rhythm = 3;
    }
    if (chaos == 10)
    {
        scale = { 0, 2, 4, 5, 7, 9, 11, 12, /*disjunct intervals:*/ /*1, 3, 6, 8, 10*/ /*or, alternatively:*/ 14, 16, 17, 21, 23 };
        rhythm = 3;
    }


    if (key == 12)
    {
        for (int& note : scale) { note += detectedKey; } // transpose to detected key
    }
    else
    {
        for (int& note : scale) { note += key; } // transpose to selected key
    }


    for (int& note : scale) { note += 60; } // transpose to a regular range

    // construct a melody sequence
    std::vector<int> processed_input;
    // construct processed_input such that it contains "int notes" # of indices,
    // and each index is a randomly picked value from the scale vector.
    processed_input.reserve(notes);
    juce::Random rand;
    for (int i = 0; i < notes; ++i)
    {
        int idx = rand.nextInt(static_cast<int>(scale.size()));
        processed_input.push_back(scale[idx]);
    }

    // length distribution logic

    std::vector<int> post_processed;
    

    if (rhythm == 1) // straight quarter notes
    {
        post_processed = std::vector<int>(32, -2);
//        std::vector<int> post_processed(32, -2);  // Initialize with -2 in all positions
        const int num_slots = 8;  // 8 note positions (even indices 0,4,...,28)

        if (notes > 0) {  // Avoid division by zero
            int repeats = num_slots / notes;
            int remainder = num_slots % notes;
            int slot_idx = 0;

            // Tile the full sequence 'repeats' times
            for (int r = 0; r < repeats; ++r) {
                for (int i = 0; i < notes; ++i) {
                    post_processed[slot_idx * 4] = processed_input[i];
                    ++slot_idx;
                }
            }

            // Add the remainder from the start of the sequence
            for (int i = 0; i < remainder; ++i) {
                post_processed[slot_idx * 4] = processed_input[i];
                ++slot_idx;
            }
        }
    }
    if (rhythm == 2) // straight eighth notes
    {
        post_processed = std::vector<int>(32, -2);
//        std::vector<int> post_processed(32, -2);  // Initialize with -2 in all positions
        const int num_slots = 16;  // 16 note positions (even indices 0,2,...,30)

        if (notes > 0) {  // Avoid division by zero
            int repeats = num_slots / notes;
            int remainder = num_slots % notes;
            int slot_idx = 0;

            // Tile the full sequence 'repeats' times
            for (int r = 0; r < repeats; ++r) {
                for (int i = 0; i < notes; ++i) {
                    post_processed[slot_idx * 2] = processed_input[i];
                    ++slot_idx;
                }
            }

            // Add the remainder from the start of the sequence
            for (int i = 0; i < remainder; ++i) {
                post_processed[slot_idx * 2] = processed_input[i];
                ++slot_idx;
            }
        }
    }
    if (rhythm == 3) // intelligent rhythmic determinism
    {
        std::random_device rd;
        std::mt19937 g(rd());

//        std::vector<int> post_processed;

        // Generate random lengths
        if (notes < 1 || notes > 32)
        {
            return;
        }
        std::vector<int> lengths(notes, 1);
        int extras = 32 - notes;
        std::uniform_int_distribution<> dis(0, notes - 1);
        for (int i = 0; i < extras; ++i)
        {
            int idx = dis(g);
            lengths[idx]++;
        }

        // Build post_processed
        for (size_t i = 0; i < static_cast<size_t>(notes); ++i)
        {
            post_processed.push_back(processed_input[i]);
            for (int j = 1; j < lengths[i]; ++j)
            {
                post_processed.push_back(-2);
            }
        }

        // Magnetize to on-beats 100% of the time
        magnetize(post_processed, 1.0);
    }

    //// Debug print post-processed output
    //juce::String debugPostProcessed = "post_processed: ";
    //for (const int& event : post_processed)
    //{
    //    debugPostProcessed += juce::String(event) + " ";
    //}
    //DBG(debugPostProcessed);

    generatedMelody = post_processed;
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
    auto state = parameters.copyState();

    {
        juce::ScopedLock sl(melodyLock);
        // Store lastGeneratedMelody as binary data
        juce::MemoryBlock melodyData(lastGeneratedMelody.data(), lastGeneratedMelody.size() * sizeof(int));
        state.setProperty("lastGeneratedMelody", melodyData.toBase64Encoding(), nullptr);
    }

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}
void CounterTuneAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    stateLoaded = true;
    if (xmlState.get() != nullptr)
    {
        juce::ValueTree state = juce::ValueTree::fromXml(*xmlState);
        parameters.replaceState(state);

        // Restore lastGeneratedMelody
        juce::String melodyBase64 = state.getProperty("lastGeneratedMelody");
        if (melodyBase64.isNotEmpty())
        {
            juce::MemoryBlock melodyData;
            melodyData.fromBase64Encoding(melodyBase64);

            {
                juce::ScopedLock sl(melodyLock);
                if (melodyData.getSize() == lastGeneratedMelody.size() * sizeof(int))
                {
                    std::memcpy(lastGeneratedMelody.data(), melodyData.getData(), melodyData.getSize());

                    if (getLoopBool())
                    {
                        generatedMelody = lastGeneratedMelody;
                    }
                }
            }

        }
    }
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CounterTuneAudioProcessor();
}
