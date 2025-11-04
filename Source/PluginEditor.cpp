// PluginEditor.cpp

#include "PluginProcessor.h"
#include "PluginEditor.h"

CounterTuneAudioProcessorEditor::CounterTuneAudioProcessorEditor (CounterTuneAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (640, 480);

    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::uibody_png, BinaryData::uibody_pngSize);


    //addAndMakeVisible(startButton);
    //startButton.setBounds(10, 10, 80, 40);
    //startButton.setButtonText("Start");
    //startButton.onClick = [this]
    //{
    //    audioProcessor.isActive.store(true);
    //    startButton.setEnabled(false);
    //    stopButton.setEnabled(true);
    //};

    //addAndMakeVisible(stopButton);
    //stopButton.setBounds(10, 60, 80, 40);
    //stopButton.setButtonText("Stop");
    //stopButton.setEnabled(false);
    //stopButton.onClick = [this]
    //{
    //    audioProcessor.isActive.store(false);
    //    stopButton.setEnabled(false);
    //};


    addAndMakeVisible(voiceBuffer_waveform);
    voiceBuffer_waveform.setBounds(1, 121, 638, 358);

    
//    addAndMakeVisible(playButton);
//    playButton.setBounds(1, 121, 80, 40);
//    playButton.setButtonText("Play");
//    playButton.onClick = [this]
//    {
//        if (audioProcessor.voiceBuffer.getNumSamples() > 0)
//        {
//            audioProcessor.voiceBuffer_readPos.store(0);
//            audioProcessor.voiceBuffer_isPlaying.store(true);
//        }
//    };


    startTimer(30);
}

CounterTuneAudioProcessorEditor::~CounterTuneAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void CounterTuneAudioProcessorEditor::timerCallback()
{
    //if (!audioProcessor.isActive.load())
    //{
    //    startButton.setEnabled(true);
    //}

    if (!audioProcessor.isUpdatingVisualMelodies.load())
    {
        displayVisualMelodies(audioProcessor.visualMelodies);
    }

    voiceBuffer_waveform.setAudioBuffer(&audioProcessor.voiceBuffer, audioProcessor.voiceBuffer.getNumSamples());

}

void CounterTuneAudioProcessorEditor::paint (juce::Graphics& g)
{

    g.drawImage(backgroundImage, getLocalBounds().toFloat());
    
}

void CounterTuneAudioProcessorEditor::resized()
{

}


