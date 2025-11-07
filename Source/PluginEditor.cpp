// PluginEditor.cpp

#include "PluginProcessor.h"
#include "PluginEditor.h"

CounterTuneAudioProcessorEditor::CounterTuneAudioProcessorEditor (CounterTuneAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), parameters(p.parameters)
{
    setSize (640, 480);
    setWantsKeyboardFocus(true);

    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::uibody_png, BinaryData::uibody_pngSize);
    presetMenuDefault = juce::ImageCache::getFromMemory(BinaryData::presetmenu_png, BinaryData::presetmenu_pngSize);
    presetMenuHover = juce::ImageCache::getFromMemory(BinaryData::presetmenuhover_png, BinaryData::presetmenuhover_pngSize);

    // custom font: B612Mono-Regular.ttf


//    addAndMakeVisible(voiceBuffer_waveform);
//    voiceBuffer_waveform.setBounds(1, 121, 638, 358);

    
    // param setup: i) TITLE, ii) KNOB, iii) VALUE

    // TEMPO 
    addAndMakeVisible(tempoTitleLabel);
    tempoTitleLabel.setBounds(25, 75, 50, 16);
    tempoTitleLabel.setJustificationType(juce::Justification::centred);
    tempoTitleLabel.setFont(getCustomFont(16.0f));
    tempoTitleLabel.setColour(juce::Label::textColourId, foregroundColor);
    tempoTitleLabel.setText("TEMPO", dontSendNotification);

    tempoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "tempo", tempoKnob);
    tempoKnob.setSliderStyle(juce::Slider::LinearBar);
    tempoKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    tempoKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    tempoKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    tempoKnob.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    tempoKnob.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    tempoKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    tempoKnob.setBounds(25, 74, 50, 16);
    tempoKnob.setRange(1, 999, 1);
//    tempoKnob.setSkewFactorFromMidPoint(150.0);
    tempoKnob.setSkewFactor(0.3);
    tempoKnob.onValueChange = [this]() { updateTempoValueLabel(); };
    addAndMakeVisible(tempoKnob);
    
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

    // BEATS
    addAndMakeVisible(beatsTitleLabel);
    beatsTitleLabel.setBounds(115, 75, 50, 16);
    beatsTitleLabel.setJustificationType(juce::Justification::centred);
    beatsTitleLabel.setFont(getCustomFont(16.0f));
    beatsTitleLabel.setColour(juce::Label::textColourId, foregroundColor);
    beatsTitleLabel.setText("BEATS", dontSendNotification);

    beatsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "beats", beatsKnob);
    beatsKnob.setSliderStyle(juce::Slider::LinearBar);
    beatsKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    beatsKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    beatsKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    beatsKnob.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    beatsKnob.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    beatsKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    beatsKnob.setBounds(115, 74, 50, 16);
    beatsKnob.setRange(1.0, 16.0, 0.25);
    beatsKnob.onValueChange = [this]() { updateBeatsValueLabel(); };
    addAndMakeVisible(beatsKnob);

    addAndMakeVisible(beatsValueLabel);
    beatsValueLabel.setBounds(115, 90, 50, 16);
    beatsValueLabel.setJustification(juce::Justification::centredTop);
    beatsValueLabel.setMultiLine(false);
    beatsValueLabel.setReturnKeyStartsNewLine(false);
    beatsValueLabel.setInputRestrictions(10, "0123456789.-+");
    beatsValueLabel.setSelectAllWhenFocused(true);
    beatsValueLabel.setFont(getCustomFont(16.0f));
    beatsValueLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    beatsValueLabel.setColour(juce::TextEditor::backgroundColourId, backgroundColor);
    beatsValueLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    beatsValueLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    updateBeatsValueLabel();
    auto commitBeats = [this]()
        {
            beatsValueLabel.moveCaretToEnd(false);

            juce::String text = beatsValueLabel.getText().trim();
            float value = text.getFloatValue();
            if (text.isEmpty() || !std::isfinite(value))
            {
                updateBeatsValueLabel();
                return;
            }
            value = juce::jlimit(1.0f, 16.0f, value);
            beatsKnob.setValue(value);
            updateBeatsValueLabel();

            grabKeyboardFocus();
        };
    beatsValueLabel.onReturnKey = commitBeats;
    beatsValueLabel.onFocusLost = commitBeats;

    // NOTES
    addAndMakeVisible(notesTitleLabel);
    notesTitleLabel.setBounds(205, 75, 50, 16);
    notesTitleLabel.setJustificationType(juce::Justification::centred);
    notesTitleLabel.setFont(getCustomFont(16.0f));
    notesTitleLabel.setColour(juce::Label::textColourId, foregroundColor);
    notesTitleLabel.setText("NOTES", dontSendNotification);

    notesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "notes", notesKnob);
    notesKnob.setSliderStyle(juce::Slider::LinearBar);
    notesKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    notesKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    notesKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    notesKnob.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    notesKnob.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    notesKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    notesKnob.setBounds(205, 74, 50, 16);
    notesKnob.setRange(1, 16, 1);
    notesKnob.onValueChange = [this]() { updateNotesValueLabel(); };
    addAndMakeVisible(notesKnob);

    addAndMakeVisible(notesValueLabel);
    notesValueLabel.setBounds(205, 90, 50, 16);
    notesValueLabel.setJustification(juce::Justification::centredTop);
    notesValueLabel.setMultiLine(false);
    notesValueLabel.setReturnKeyStartsNewLine(false);
    notesValueLabel.setInputRestrictions(10, "0123456789.-+");
    notesValueLabel.setSelectAllWhenFocused(true);
    notesValueLabel.setFont(getCustomFont(16.0f));
    notesValueLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    notesValueLabel.setColour(juce::TextEditor::backgroundColourId, backgroundColor);
    notesValueLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    notesValueLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    updateNotesValueLabel();
    auto commitNotes = [this]()
        {
            notesValueLabel.moveCaretToEnd(false);

            juce::String text = notesValueLabel.getText().trim();
            int value = text.getIntValue();
            if (text.isEmpty())
            {
                updateNotesValueLabel();
                return;
            }
            value = juce::jlimit(1, 16, value);
            notesKnob.setValue(value);
            updateNotesValueLabel();

            grabKeyboardFocus();
        };
    notesValueLabel.onReturnKey = commitBeats;
    notesValueLabel.onFocusLost = commitBeats;

    // OCTAVE

    addAndMakeVisible(octaveTitleLabel);
    octaveTitleLabel.setBounds(295, 75, 50, 16);
    octaveTitleLabel.setJustificationType(juce::Justification::centred);
    octaveTitleLabel.setFont(getCustomFont(16.0f));
    octaveTitleLabel.setColour(juce::Label::textColourId, foregroundColor);
    octaveTitleLabel.setText("OCTAVE", dontSendNotification);

    octaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "octave", octaveKnob);
    octaveKnob.setSliderStyle(juce::Slider::LinearBar);
    octaveKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    octaveKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    octaveKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    octaveKnob.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    octaveKnob.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    octaveKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    octaveKnob.setBounds(295, 74, 50, 16);
    octaveKnob.setRange(-4, 4, 1);
    octaveKnob.onValueChange = [this]() { updateOctaveValueLabel(); };
    addAndMakeVisible(octaveKnob);

    addAndMakeVisible(octaveValueLabel);
    octaveValueLabel.setBounds(295, 90, 50, 16);
    octaveValueLabel.setJustification(juce::Justification::centredTop);
    octaveValueLabel.setMultiLine(false);
    octaveValueLabel.setReturnKeyStartsNewLine(false);
    octaveValueLabel.setInputRestrictions(10, "0123456789.-+");
    octaveValueLabel.setSelectAllWhenFocused(true);
    octaveValueLabel.setFont(getCustomFont(16.0f));
    octaveValueLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    octaveValueLabel.setColour(juce::TextEditor::backgroundColourId, backgroundColor);
    octaveValueLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    octaveValueLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    updateOctaveValueLabel();
    auto commitOctave = [this]()
        {
            octaveValueLabel.moveCaretToEnd(false);

            juce::String text = octaveValueLabel.getText().trim();

            if (text.isEmpty())
            {
                updateOctaveValueLabel();
                return;
            }

            // Remove spaces from the formatted display text (e.g., "- 2" becomes "-2")
            text = text.removeCharacters(" ");

            int value = text.getIntValue();
            value = juce::jlimit(-4, 4, value);
            octaveKnob.setValue(value);
            updateOctaveValueLabel();

            grabKeyboardFocus();
        };
    octaveValueLabel.onReturnKey = commitOctave;
    octaveValueLabel.onFocusLost = commitOctave;

    // DETUNE

    addAndMakeVisible(detuneTitleLabel);
    detuneTitleLabel.setBounds(385, 75, 50, 16);
    detuneTitleLabel.setJustificationType(juce::Justification::centred);
    detuneTitleLabel.setFont(getCustomFont(16.0f));
    detuneTitleLabel.setColour(juce::Label::textColourId, foregroundColor);
    detuneTitleLabel.setText("DETUNE", dontSendNotification);

    detuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "detune", detuneKnob);
    detuneKnob.setSliderStyle(juce::Slider::LinearBar);
    detuneKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    detuneKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    detuneKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    detuneKnob.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    detuneKnob.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    detuneKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    detuneKnob.setBounds(385, 74, 50, 16);
    detuneKnob.setRange(-1.0, 1.0, 0.01);
    detuneKnob.onValueChange = [this]() { updateDetuneValueLabel(); };
    addAndMakeVisible(detuneKnob);

    addAndMakeVisible(detuneValueLabel);
    detuneValueLabel.setBounds(385, 90, 50, 16);
    detuneValueLabel.setJustification(juce::Justification::centredTop);
    detuneValueLabel.setMultiLine(false);
    detuneValueLabel.setReturnKeyStartsNewLine(false);
    detuneValueLabel.setInputRestrictions(10, "0123456789.-+");
    detuneValueLabel.setSelectAllWhenFocused(true);
    detuneValueLabel.setFont(getCustomFont(16.0f));
    detuneValueLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    detuneValueLabel.setColour(juce::TextEditor::backgroundColourId, backgroundColor);
    detuneValueLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    detuneValueLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    updateDetuneValueLabel();
    auto commitDetune = [this]()
        {
            detuneValueLabel.moveCaretToEnd(false);

            juce::String text = detuneValueLabel.getText().trim();
            float value = text.getFloatValue();
            if (text.isEmpty() || !std::isfinite(value))
            {
                updateDetuneValueLabel();
                return;
            }
            value = juce::jlimit(-1.0f, 1.0f, value);
            detuneKnob.setValue(value);
            updateDetuneValueLabel();

            grabKeyboardFocus();
        };
    detuneValueLabel.onReturnKey = commitDetune;
    detuneValueLabel.onFocusLost = commitDetune;

    // MIX

    addAndMakeVisible(mixTitleLabel);
    mixTitleLabel.setBounds(475, 75, 50, 16);
    mixTitleLabel.setJustificationType(juce::Justification::centred);
    mixTitleLabel.setFont(getCustomFont(16.0f));
    mixTitleLabel.setColour(juce::Label::textColourId, foregroundColor);
    mixTitleLabel.setText("MIX", dontSendNotification);

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "mix", mixKnob);
    mixKnob.setSliderStyle(juce::Slider::LinearBar);
    mixKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    mixKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    mixKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    mixKnob.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    mixKnob.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    mixKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    mixKnob.setBounds(475, 74, 50, 16);
    mixKnob.setRange(0.0, 1.0, 0.01);
    mixKnob.onValueChange = [this]() { updateMixValueLabel(); };
    addAndMakeVisible(mixKnob);

    addAndMakeVisible(mixValueLabel);
    mixValueLabel.setBounds(475, 90, 50, 16);
    mixValueLabel.setJustification(juce::Justification::centredTop);
    mixValueLabel.setMultiLine(false);
    mixValueLabel.setReturnKeyStartsNewLine(false);
    mixValueLabel.setInputRestrictions(10, "0123456789.-+");
    mixValueLabel.setSelectAllWhenFocused(true);
    mixValueLabel.setFont(getCustomFont(16.0f));
    mixValueLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    mixValueLabel.setColour(juce::TextEditor::backgroundColourId, backgroundColor);
    mixValueLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    mixValueLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    updateMixValueLabel();
    auto commitMix = [this]()
        {
            mixValueLabel.moveCaretToEnd(false);

            juce::String text = mixValueLabel.getText().trim();
            float value = text.getFloatValue();
            if (text.isEmpty() || !std::isfinite(value))
            {
                updateMixValueLabel();
                return;
            }
            value = juce::jlimit(0.0f, 1.0f, value);
            mixKnob.setValue(value);
            updateMixValueLabel();

            grabKeyboardFocus();
        };
    mixValueLabel.onReturnKey = commitMix;
    mixValueLabel.onFocusLost = commitMix;

    // LOOP

    addAndMakeVisible(loopTitleLabel);
    loopTitleLabel.setBounds(565, 75, 50, 16);
    loopTitleLabel.setJustificationType(juce::Justification::centred);
    loopTitleLabel.setFont(getCustomFont(16.0f));
    loopTitleLabel.setColour(juce::Label::textColourId, foregroundColor);
    loopTitleLabel.setText("LOOP", juce::dontSendNotification);

    addAndMakeVisible(loopValueLabel);
    loopValueLabel.setBounds(565, 91, 50, 16);
    loopValueLabel.setJustificationType(juce::Justification::centred);
    loopValueLabel.setFont(getCustomFont(16.0f));
    loopValueLabel.setColour(juce::Label::textColourId, foregroundColor);
    updateLoopValueLabel();

    loopAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(parameters, "loop", loopButton);
    loopButton.setClickingTogglesState(true);
    loopButton.setBounds(565, 74, 50, 33);
    loopButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    loopButton.onClick = [this]() { updateLoopValueLabel(); };
    addAndMakeVisible(loopButton);




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
        updateBeatsValueLabel();
        updateNotesValueLabel();
        updateOctaveValueLabel();
        updateDetuneValueLabel();
        updateMixValueLabel();
        updateLoopValueLabel();
        firstLoad = false;
    }

    // paint melodies




}

void CounterTuneAudioProcessorEditor::paint (juce::Graphics& g)
{

    g.drawImage(backgroundImage, getLocalBounds().toFloat());
    g.drawImage(presetMenuDefault, juce::Rectangle<float>(481, 21, 140, 18));

}

void CounterTuneAudioProcessorEditor::resized()
{

}


