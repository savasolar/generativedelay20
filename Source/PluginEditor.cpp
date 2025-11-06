// PluginEditor.cpp

#include "PluginProcessor.h"
#include "PluginEditor.h"

CounterTuneAudioProcessorEditor::CounterTuneAudioProcessorEditor (CounterTuneAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), parameters(p.parameters)
{
    setSize (640, 480);
    setWantsKeyboardFocus(true);

    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::uibody_png, BinaryData::uibody_pngSize);

    // custom font: B612Mono-Regular.ttf


//    addAndMakeVisible(voiceBuffer_waveform);
//    voiceBuffer_waveform.setBounds(1, 121, 638, 358);

    
    tempoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "tempo", tempoKnob);
    tempoKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    tempoKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    //bpmKnob.setLookAndFeel(knobLookAndFeel.get());
    tempoKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
    tempoKnob.onDragStart = [this]() {};
    tempoKnob.onDragEnd = [this]() {};
    tempoKnob.setBounds(32, 404, 40, 40);
    tempoKnob.setRange(1, 999, 1);
    tempoKnob.onValueChange = [this]() { updateTempoValueLabel(); };
    addAndMakeVisible(tempoKnob);

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

    auto commitTempo = [this]()
        {
            tempoValueLabel.moveCaretToEnd(false);

            juce::String text = tempoValueLabel.getText().trim();
            float value = text.getFloatValue();
            if (text.isEmpty() || !std::isfinite(value))
            {
                updateTempoValueLabel();
                return;
            }
            value = juce::jlimit(1.0f, 999.0f, value);
            tempoKnob.setValue(value);
            updateTempoValueLabel();
            grabKeyboardFocus();
        };
    tempoValueLabel.onReturnKey = commitTempo;
    tempoValueLabel.onFocusLost = commitTempo;



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


    if (firstLoad)
    {
        updateTempoValueLabel();
        //...
        firstLoad = false;
    }

    // paint melodies




}

void CounterTuneAudioProcessorEditor::paint (juce::Graphics& g)
{

    g.drawImage(backgroundImage, getLocalBounds().toFloat());
    
}

void CounterTuneAudioProcessorEditor::resized()
{

}


