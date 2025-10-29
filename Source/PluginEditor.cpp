// PluginEditor.cpp

#include "PluginProcessor.h"
#include "PluginEditor.h"

EnCounterAudioProcessorEditor::EnCounterAudioProcessorEditor (EnCounterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (560, 464);

//    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::encounterbodywide_png, BinaryData::encounterbodywide_pngSize);


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
    
    startTimer(30);
}

EnCounterAudioProcessorEditor::~EnCounterAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void EnCounterAudioProcessorEditor::timerCallback()
{
    //if (!audioProcessor.isActive.load())
    //{
    //    startButton.setEnabled(true);
    //}
}

void EnCounterAudioProcessorEditor::paint (juce::Graphics& g)
{

//    g.drawImage(backgroundImage, getLocalBounds().toFloat());
    
}

void EnCounterAudioProcessorEditor::resized()
{

}
