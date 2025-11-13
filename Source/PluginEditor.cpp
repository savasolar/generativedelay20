// PluginEditor.cpp

#include "PluginProcessor.h"
#include "PluginEditor.h"

CounterTuneAudioProcessorEditor::CounterTuneAudioProcessorEditor(CounterTuneAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), parameters(p.parameters)
{
    setSize(640, 480);
    setWantsKeyboardFocus(true);

    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::uibody_png, BinaryData::uibody_pngSize);
    presetMenuDefault = juce::ImageCache::getFromMemory(BinaryData::presetmenu_png, BinaryData::presetmenu_pngSize);
    presetMenuHover = juce::ImageCache::getFromMemory(BinaryData::presetmenuhover_png, BinaryData::presetmenuhover_pngSize);


    //    addAndMakeVisible(voiceBuffer_waveform);
    //    voiceBuffer_waveform.setBounds(1, 121, 638, 358);



    addAndMakeVisible(presetTitleButton);
    presetTitleButton.setBounds(481, 21, 140, 18);
//    presetTitleButton.onMouse = [this]() {};


        // param setup: i) TITLE, ii) KNOB, iii) VALUE

    // TEMPO 
    addAndMakeVisible(tempoTitleLabel);
    tempoTitleLabel.setBounds(1, 60, 60, 20);
    tempoTitleLabel.setJustification(juce::Justification::centred);
    tempoTitleLabel.setFont(getCustomFont(18.0f));
    tempoTitleLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    tempoTitleLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    tempoTitleLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    tempoTitleLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    tempoTitleLabel.setReadOnly(true);
    tempoTitleLabel.setCaretVisible(false);
    tempoTitleLabel.setMouseCursor(juce::MouseCursor::NormalCursor);
    tempoTitleLabel.setText("TEMPO", dontSendNotification);
    
    tempoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "tempo", tempoKnob);
    tempoKnob.setSliderStyle(juce::Slider::LinearBar);
    tempoKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    tempoKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    tempoKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    tempoKnob.setColour(juce::Slider::trackColourId, foregroundColor);
    tempoKnob.setColour(juce::Slider::thumbColourId, foregroundColor);
    tempoKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    tempoKnob.setBounds(1, 80, 60, 20);
    tempoKnob.setRange(1, 999, 1);
    tempoKnob.setSkewFactor(0.3);
    tempoKnob.onValueChange = [this]() { updateTempoValueLabel(); };
    addAndMakeVisible(tempoKnob);

    addAndMakeVisible(tempoValueLabel);
    tempoValueLabel.setBounds(1, 100, 60, 16);
    tempoValueLabel.setJustification(juce::Justification::centredTop);
    tempoValueLabel.setMultiLine(false);
    tempoValueLabel.setReturnKeyStartsNewLine(false);
    tempoValueLabel.setInputRestrictions(10, "0123456789.-+");
    tempoValueLabel.setSelectAllWhenFocused(true);
    tempoValueLabel.setFont(getCustomFont(18.0f));
    tempoValueLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    tempoValueLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
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
    beatsTitleLabel.setBounds(73, 60, 60, 20);
    beatsTitleLabel.setJustification(juce::Justification::centred);
    beatsTitleLabel.setFont(getCustomFont(18.0f));
    beatsTitleLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    beatsTitleLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    beatsTitleLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    beatsTitleLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    beatsTitleLabel.setReadOnly(true);
    beatsTitleLabel.setCaretVisible(false);
    beatsTitleLabel.setMouseCursor(juce::MouseCursor::NormalCursor);
    beatsTitleLabel.setText("BEATS", dontSendNotification);

    beatsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "beats", beatsKnob);
    beatsKnob.setSliderStyle(juce::Slider::LinearBar);
    beatsKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    beatsKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    beatsKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    beatsKnob.setColour(juce::Slider::trackColourId, foregroundColor);
    beatsKnob.setColour(juce::Slider::thumbColourId, foregroundColor);
    beatsKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    beatsKnob.setBounds(73, 80, 60, 20);
    beatsKnob.setRange(1.0, 16.0, 0.25);
    beatsKnob.onValueChange = [this]() { updateBeatsValueLabel(); };
    addAndMakeVisible(beatsKnob);

    addAndMakeVisible(beatsValueLabel);
    beatsValueLabel.setBounds(73, 100, 60, 16);
    beatsValueLabel.setJustification(juce::Justification::centredTop);
    beatsValueLabel.setMultiLine(false);
    beatsValueLabel.setReturnKeyStartsNewLine(false);
    beatsValueLabel.setInputRestrictions(10, "0123456789.-+");
    beatsValueLabel.setSelectAllWhenFocused(true);
    beatsValueLabel.setFont(getCustomFont(18.0f));
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

    // KEY
    addAndMakeVisible(keyTitleLabel);
    keyTitleLabel.setBounds(145, 60, 60, 20);
    keyTitleLabel.setJustification(juce::Justification::centred);
    keyTitleLabel.setFont(getCustomFont(18.0f));
    keyTitleLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    keyTitleLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    keyTitleLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    keyTitleLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    keyTitleLabel.setReadOnly(true);
    keyTitleLabel.setCaretVisible(false);
    keyTitleLabel.setMouseCursor(juce::MouseCursor::NormalCursor);
    keyTitleLabel.setText("KEY", dontSendNotification);

    keyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "key", keyKnob);
    keyKnob.setSliderStyle(juce::Slider::LinearBar);
    keyKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    keyKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    keyKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    keyKnob.setColour(juce::Slider::trackColourId, foregroundColor);
    keyKnob.setColour(juce::Slider::thumbColourId, foregroundColor);
    keyKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    keyKnob.setBounds(145, 80, 60, 20);
    keyKnob.setRange(0, 12, 1);
    keyKnob.onValueChange = [this]() { updateKeyValueLabel(); };
    addAndMakeVisible(keyKnob);

    addAndMakeVisible(keyValueLabel);
    keyValueLabel.setBounds(145, 100, 60, 16);
    keyValueLabel.setJustification(juce::Justification::centredTop);
    keyValueLabel.setMultiLine(false);
    keyValueLabel.setReturnKeyStartsNewLine(false);
    keyValueLabel.setInputRestrictions(10, "0123456789.-+");
    keyValueLabel.setSelectAllWhenFocused(true);
    keyValueLabel.setFont(getCustomFont(18.0f));
    keyValueLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    keyValueLabel.setColour(juce::TextEditor::backgroundColourId, backgroundColor);
    keyValueLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    keyValueLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    updateKeyValueLabel();
    auto commitKey = [this]()
    {
        keyValueLabel.moveCaretToEnd(false);

        juce::String text = keyValueLabel.getText().trim();
        int value = text.getIntValue();
        if (text.isEmpty())
        {
            updateKeyValueLabel();
            return;
        }
        value = juce::jlimit(0, 12, value);
        keyKnob.setValue(value);
        updateKeyValueLabel();

        grabKeyboardFocus();
    };
    keyValueLabel.onReturnKey = commitKey;
    keyValueLabel.onFocusLost = commitKey;


    // NOTES
    addAndMakeVisible(notesTitleLabel);
    notesTitleLabel.setBounds(217, 60, 60, 20);
    notesTitleLabel.setJustification(juce::Justification::centred);
    notesTitleLabel.setFont(getCustomFont(18.0f));
    notesTitleLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    notesTitleLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    notesTitleLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    notesTitleLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    notesTitleLabel.setReadOnly(true);
    notesTitleLabel.setCaretVisible(false);
    notesTitleLabel.setMouseCursor(juce::MouseCursor::NormalCursor);
    notesTitleLabel.setText("NOTES", dontSendNotification);

    notesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "notes", notesKnob);
    notesKnob.setSliderStyle(juce::Slider::LinearBar);
    notesKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    notesKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    notesKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    notesKnob.setColour(juce::Slider::trackColourId, foregroundColor);
    notesKnob.setColour(juce::Slider::thumbColourId, foregroundColor);
    notesKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    notesKnob.setBounds(217, 80, 60, 20);
    notesKnob.setRange(1, 16, 1);
    notesKnob.onValueChange = [this]() { updateNotesValueLabel(); };
    addAndMakeVisible(notesKnob);

    addAndMakeVisible(notesValueLabel);
    notesValueLabel.setBounds(217, 100, 60, 16);
    notesValueLabel.setJustification(juce::Justification::centredTop);
    notesValueLabel.setMultiLine(false);
    notesValueLabel.setReturnKeyStartsNewLine(false);
    notesValueLabel.setInputRestrictions(10, "0123456789.-+");
    notesValueLabel.setSelectAllWhenFocused(true);
    notesValueLabel.setFont(getCustomFont(18.0f));
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


    // CHAOS
    addAndMakeVisible(chaosTitleLabel);
    chaosTitleLabel.setBounds(289, 60, 60, 20);
    chaosTitleLabel.setJustification(juce::Justification::centred);
    chaosTitleLabel.setFont(getCustomFont(18.0f));
    chaosTitleLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    chaosTitleLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    chaosTitleLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    chaosTitleLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    chaosTitleLabel.setReadOnly(true);
    chaosTitleLabel.setCaretVisible(false);
    chaosTitleLabel.setMouseCursor(juce::MouseCursor::NormalCursor);
    chaosTitleLabel.setText("CHAOS", dontSendNotification);

    chaosAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "chaos", chaosKnob);
    chaosKnob.setSliderStyle(juce::Slider::LinearBar);
    chaosKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    chaosKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    chaosKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    chaosKnob.setColour(juce::Slider::trackColourId, foregroundColor);
    chaosKnob.setColour(juce::Slider::thumbColourId, foregroundColor);
    chaosKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    chaosKnob.setBounds(289, 80, 60, 20);
    chaosKnob.setRange(0.0, 1.0, 0.01);
    chaosKnob.onValueChange = [this]() { updateChaosValueLabel(); };
    addAndMakeVisible(chaosKnob);

    addAndMakeVisible(chaosValueLabel);
    chaosValueLabel.setBounds(289, 100, 60, 16);
    chaosValueLabel.setJustification(juce::Justification::centredTop);
    chaosValueLabel.setMultiLine(false);
    chaosValueLabel.setReturnKeyStartsNewLine(false);
    chaosValueLabel.setInputRestrictions(10, "0123456789.-+");
    chaosValueLabel.setSelectAllWhenFocused(true);
    chaosValueLabel.setFont(getCustomFont(18.0f));
    chaosValueLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    chaosValueLabel.setColour(juce::TextEditor::backgroundColourId, backgroundColor);
    chaosValueLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    chaosValueLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    updateChaosValueLabel();
    auto commitChaos = [this]()
    {
        chaosValueLabel.moveCaretToEnd(false);

        juce::String text = chaosValueLabel.getText().trim();
        float value = text.getFloatValue();
        if (text.isEmpty() || !std::isfinite(value))
        {
            updateChaosValueLabel();
            return;
        }
        value = juce::jlimit(0.0f, 1.0f, value);
        chaosKnob.setValue(value);
        updateChaosValueLabel();

        grabKeyboardFocus();
    };
    chaosValueLabel.onReturnKey = commitChaos;
    chaosValueLabel.onFocusLost = commitChaos;

    // OCTAVE

    addAndMakeVisible(octaveTitleLabel);
    octaveTitleLabel.setBounds(361, 60, 60, 20);
    octaveTitleLabel.setJustification(juce::Justification::centred);
    octaveTitleLabel.setFont(getCustomFont(18.0f));
    octaveTitleLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    octaveTitleLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    octaveTitleLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    octaveTitleLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    octaveTitleLabel.setReadOnly(true);
    octaveTitleLabel.setCaretVisible(false);
    octaveTitleLabel.setMouseCursor(juce::MouseCursor::NormalCursor);
    octaveTitleLabel.setText("OCTAVE", dontSendNotification);

    octaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "octave", octaveKnob);
    octaveKnob.setSliderStyle(juce::Slider::LinearBar);
    octaveKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    octaveKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    octaveKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    octaveKnob.setColour(juce::Slider::trackColourId, foregroundColor);
    octaveKnob.setColour(juce::Slider::thumbColourId, foregroundColor);
    octaveKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    octaveKnob.setBounds(361, 80, 60, 20);
    octaveKnob.setRange(-4, 4, 1);
    octaveKnob.onValueChange = [this]() { updateOctaveValueLabel(); };
    addAndMakeVisible(octaveKnob);

    addAndMakeVisible(octaveValueLabel);
    octaveValueLabel.setBounds(361, 100, 60, 16);
    octaveValueLabel.setJustification(juce::Justification::centredTop);
    octaveValueLabel.setMultiLine(false);
    octaveValueLabel.setReturnKeyStartsNewLine(false);
    octaveValueLabel.setInputRestrictions(10, "0123456789.-+");
    octaveValueLabel.setSelectAllWhenFocused(true);
    octaveValueLabel.setFont(getCustomFont(18.0f));
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
    detuneTitleLabel.setBounds(433, 60, 60, 20);
    detuneTitleLabel.setJustification(juce::Justification::centred);
    detuneTitleLabel.setFont(getCustomFont(18.0f));
    detuneTitleLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    detuneTitleLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    detuneTitleLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    detuneTitleLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    detuneTitleLabel.setReadOnly(true);
    detuneTitleLabel.setCaretVisible(false);
    detuneTitleLabel.setMouseCursor(juce::MouseCursor::NormalCursor);
    detuneTitleLabel.setText("DETUNE", dontSendNotification);

    detuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "detune", detuneKnob);
    detuneKnob.setSliderStyle(juce::Slider::LinearBar);
    detuneKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    detuneKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    detuneKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    detuneKnob.setColour(juce::Slider::trackColourId, foregroundColor);
    detuneKnob.setColour(juce::Slider::thumbColourId, foregroundColor);
    detuneKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    detuneKnob.setBounds(433, 80, 60, 20);
    detuneKnob.setRange(-1.0, 1.0, 0.01);
    detuneKnob.onValueChange = [this]() { updateDetuneValueLabel(); };
    addAndMakeVisible(detuneKnob);

    addAndMakeVisible(detuneValueLabel);
    detuneValueLabel.setBounds(433, 100, 60, 16);
    detuneValueLabel.setJustification(juce::Justification::centredTop);
    detuneValueLabel.setMultiLine(false);
    detuneValueLabel.setReturnKeyStartsNewLine(false);
    detuneValueLabel.setInputRestrictions(10, "0123456789.-+");
    detuneValueLabel.setSelectAllWhenFocused(true);
    detuneValueLabel.setFont(getCustomFont(18.0f));
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

    // LOOP

    addAndMakeVisible(loopTitleLabel);
    loopTitleLabel.setBounds(505, 60, 60, 20);
    loopTitleLabel.setJustification(juce::Justification::centred);
    loopTitleLabel.setFont(getCustomFont(18.0f));
    loopTitleLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    loopTitleLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    loopTitleLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    loopTitleLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    loopTitleLabel.setReadOnly(true);
    loopTitleLabel.setCaretVisible(false);
    loopTitleLabel.setMouseCursor(juce::MouseCursor::NormalCursor);
    loopTitleLabel.setText("LOOP", dontSendNotification);

    //addAndMakeVisible(loopValueLabel);
    //loopValueLabel.setBounds(505, 80, 60, 20);
    //loopValueLabel.setJustificationType(juce::Justification::centred);
    //loopValueLabel.setFont(getCustomFont(18.0f));
    //loopValueLabel.setColour(juce::Label::textColourId, foregroundColor);


    addAndMakeVisible(loopOnLabel);
    loopOnLabel.setBounds(505, 80, 60, 20);
    loopOnLabel.setJustification(juce::Justification::centred);
    loopOnLabel.setFont(getCustomFont(18.0f));
    loopOnLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    loopOnLabel.setColour(juce::TextEditor::backgroundColourId, backgroundColor);
    loopOnLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    loopOnLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    loopOnLabel.setReadOnly(true);
    loopOnLabel.setCaretVisible(false);
    loopOnLabel.setMouseCursor(juce::MouseCursor::NormalCursor);
    loopOnLabel.setText("ON", dontSendNotification);

    addAndMakeVisible(loopOffLabel);
    loopOffLabel.setBounds(505, 100, 60, 20);
    loopOffLabel.setJustification(juce::Justification::centred);
    loopOffLabel.setFont(getCustomFont(18.0f));
    loopOffLabel.setColour(juce::TextEditor::textColourId, backgroundColor);
    loopOffLabel.setColour(juce::TextEditor::backgroundColourId, foregroundColor);
    loopOffLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    loopOffLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    loopOffLabel.setReadOnly(true);
    loopOffLabel.setCaretVisible(false);
    loopOffLabel.setMouseCursor(juce::MouseCursor::NormalCursor);
    loopOffLabel.setText("OFF", dontSendNotification);


    updateLoopValueLabel();

    loopAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(parameters, "loop", loopButton);
    loopButton.setClickingTogglesState(true);
    loopButton.setBounds(505, 60, 60, 60);
    loopButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    loopButton.onClick = [this]() { updateLoopValueLabel(); };
    addAndMakeVisible(loopButton);

    // MIX

    addAndMakeVisible(mixTitleLabel);
    mixTitleLabel.setBounds(579, 60, 60, 20);
    mixTitleLabel.setJustification(juce::Justification::centred);
    mixTitleLabel.setFont(getCustomFont(18.0f));
    mixTitleLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    mixTitleLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    mixTitleLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    mixTitleLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    mixTitleLabel.setReadOnly(true);
    mixTitleLabel.setCaretVisible(false);
    mixTitleLabel.setMouseCursor(juce::MouseCursor::NormalCursor);
    mixTitleLabel.setText("MIX", dontSendNotification);

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "mix", mixKnob);
    mixKnob.setSliderStyle(juce::Slider::LinearBar);
    mixKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    mixKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    mixKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    mixKnob.setColour(juce::Slider::trackColourId, foregroundColor);
    mixKnob.setColour(juce::Slider::thumbColourId, foregroundColor);
    mixKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    mixKnob.setBounds(579, 80, 60, 20);
    mixKnob.setRange(0.0, 1.0, 0.01);
    mixKnob.onValueChange = [this]() { updateMixValueLabel(); };
    addAndMakeVisible(mixKnob);

    addAndMakeVisible(mixValueLabel);
    mixValueLabel.setBounds(579, 100, 60, 16);
    mixValueLabel.setJustification(juce::Justification::centredTop);
    mixValueLabel.setMultiLine(false);
    mixValueLabel.setReturnKeyStartsNewLine(false);
    mixValueLabel.setInputRestrictions(10, "0123456789.-+");
    mixValueLabel.setSelectAllWhenFocused(true);
    mixValueLabel.setFont(getCustomFont(18.0f));
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




    repaint();

}

void CounterTuneAudioProcessorEditor::paint (juce::Graphics& g)
{

    g.drawImage(backgroundImage, getLocalBounds().toFloat());
    
    if (presetTitleButton.isMouseButtonDown())
    {
        g.drawImage(presetMenuHover, juce::Rectangle<float>(481, 21, 140, 18));
    }
    else
    {
        g.drawImage(presetMenuDefault, juce::Rectangle<float>(481, 21, 140, 18));
    }


}

void CounterTuneAudioProcessorEditor::resized()
{

}


