// PluginEditor.cpp

#include "PluginProcessor.h"
#include "PluginEditor.h"

EnCounterAudioProcessorEditor::EnCounterAudioProcessorEditor (EnCounterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 400);

//    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::encounterbodywide_png, BinaryData::encounterbodywide_pngSize);

    customLookAndFeel = std::make_unique<CustomLookAndFeel>();
    setLookAndFeel (customLookAndFeel.get());
    
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
