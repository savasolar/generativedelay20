// PluginEditor.cpp

#include "PluginProcessor.h"
#include "PluginEditor.h"

CounterTuneAudioProcessorEditor::CounterTuneAudioProcessorEditor (CounterTuneAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (640, 480);

    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::uibody_png, BinaryData::uibody_pngSize);

    // custom font: B612Mono-Regular.ttf


//    addAndMakeVisible(voiceBuffer_waveform);
//    voiceBuffer_waveform.setBounds(1, 121, 638, 358);

    
    addAndMakeVisible(tempoTitleLabel);
    tempoTitleLabel.setBounds(25, 75, 50, 16);
    tempoTitleLabel.setJustificationType(juce::Justification::centred);
    tempoTitleLabel.setFont(getCustomFont(16.0f));
    tempoTitleLabel.setColour(juce::Label::textColourId, foregroundColor);
    tempoTitleLabel.setText("TEMPO", dontSendNotification);
    
    addAndMakeVisible(tempoValueLabel);
    tempoValueLabel.setBounds(25, 90, 50, 16);
    tempoValueLabel.setJustification(juce::Justification::centredTop);
    tempoValueLabel.setMultiLine(false);
    tempoValueLabel.setReturnKeyStartsNewLine(false);
    tempoValueLabel.setInputRestrictions(10, "0123456789.-+");
    tempoValueLabel.setSelectAllWhenFocused(true);
    tempoValueLabel.setFont(getCustomFont(16.0f));
    tempoValueLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    tempoValueLabel.setColour(juce::TextEditor::backgroundColourId, backgroundColor);
    tempoValueLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    tempoValueLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    updateTempoValueLabel();





    // a good dropdown menu can be created simply with a rectangle and buttons




    startTimer(30);
}

CounterTuneAudioProcessorEditor::~CounterTuneAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void CounterTuneAudioProcessorEditor::timerCallback()
{

//    voiceBuffer_waveform.setAudioBuffer(&audioProcessor.voiceBuffer, audioProcessor.voiceBuffer.getNumSamples());

}

void CounterTuneAudioProcessorEditor::paint (juce::Graphics& g)
{

    g.drawImage(backgroundImage, getLocalBounds().toFloat());
    
}

void CounterTuneAudioProcessorEditor::resized()
{

}


