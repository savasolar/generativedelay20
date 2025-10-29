// PluginEditor.cpp

#include "PluginProcessor.h"
#include "PluginEditor.h"

EnCounterAudioProcessorEditor::EnCounterAudioProcessorEditor (EnCounterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (560, 464);

//    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::encounterbodywide_png, BinaryData::encounterbodywide_pngSize);

    
    startTimer(30);
}

EnCounterAudioProcessorEditor::~EnCounterAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void EnCounterAudioProcessorEditor::timerCallback()
{

}

void EnCounterAudioProcessorEditor::paint (juce::Graphics& g)
{

//    g.drawImage(backgroundImage, getLocalBounds().toFloat());
    
}

void EnCounterAudioProcessorEditor::resized()
{

}
