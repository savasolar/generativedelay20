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

    // In PluginEditor.cpp constructor, after loading other images like presetMenuHover

    capturedTextureImage = juce::ImageCache::getFromMemory(BinaryData::capmeltexture_png, BinaryData::capmeltexture_pngSize);
    overlapTextureImage = juce::ImageCache::getFromMemory(BinaryData::overlapmeltexture_png, BinaryData::overlapmeltexture_pngSize);
    genTextureImage = juce::ImageCache::getFromMemory(BinaryData::genmeltexture_png, BinaryData::genmeltexture_pngSize);


    setupParams();

    setupPresetMenu();


    startTimer(30);
}

CounterTuneAudioProcessorEditor::~CounterTuneAudioProcessorEditor()
{
    loopButton.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

void CounterTuneAudioProcessorEditor::timerCallback()
{
    if (firstLoad)
    {
        updateTempoValueLabel();
        updateBeatsValueLabel();
//        key!?
        updateKeyValueLabel();
        updateNotesValueLabel();
//        chaos!?
        updateChaosValueLabel();
        updateOctaveValueLabel();
        updateDetuneValueLabel();
        updateMixValueLabel();
        updateLoopValueLabel();
        updateLoopButtonTheme();
        updatePresetLabel();
        firstLoad = false;
    }

    // paint melodies


    audioProcessor.copyMelodiesTo(capturedMelody, generatedMelody);


    repaint();

}

void CounterTuneAudioProcessorEditor::paint (juce::Graphics& g)
{

    g.drawImage(backgroundImage, getLocalBounds().toFloat());
    
    paintPresetMenu(g);




    // draw a 1px dotted line

    drawDottedLine(g, 25, 121, 1, 358);

    drawDottedLine(g, 25, 149, 614, 1);
    drawDottedLine(g, 25, 179, 614, 1);
    drawDottedLine(g, 25, 209, 614, 1);
    drawDottedLine(g, 25, 239, 614, 1);
    drawDottedLine(g, 25, 269, 614, 1);
    drawDottedLine(g, 25, 299, 614, 1);
    drawDottedLine(g, 25, 329, 614, 1);
    drawDottedLine(g, 25, 359, 614, 1);
    drawDottedLine(g, 25, 389, 614, 1);
    drawDottedLine(g, 25, 419, 614, 1);
    drawDottedLine(g, 25, 449, 614, 1);



    // Dynamic vertical dotted lines based on beats
    int numBeats = static_cast<int>(std::round(audioProcessor.getBeatsFloat() * 4));
    if (numBeats > 1)
    {
        int numLines = numBeats - 1;
        double sectionWidth = 614.0 / numBeats;
        for (int i = 1; i <= numLines; ++i)
        {
            int x = 25 + static_cast<int>(std::round(i * sectionWidth));
            drawDottedLine(g, x, 121, 1, 358);
        }
    }


    // paint melodies

    std::vector<int> processedMelody;
    if (!generatedMelody.empty() && generatedMelody.size() == 32)
    {
        processedMelody = generatedMelody;
        int lastNote = -1;
        for (auto& n : processedMelody)
        {
            if (n == -2)
            {
                if (lastNote != -1)
                {
                    n = lastNote;
                }
                else
                {
                    n = -1; // Skip if no previous note
                }
            }
            else if (n >= 0) // Assuming valid MIDI notes are non-negative
            {
                lastNote = n;
            }
            else
            {
                n = -1; // Invalid, skip
            }
        }

        int gridX = 26;
        int gridY = 120;
        int gridW = 614;
        int gridH = 360;

        int cellH = gridH / 12; // Should be 30

        for (int i = 0; i < 32; ++i)
        {
            int note = processedMelody[i];
            if (note == -1) continue;

            int normNote = note % 12;
            if (normNote < 0 || normNote > 11) continue; // Safety check

            int x = gridX + (i * gridW) / 32;
            int next_x = gridX + ((i + 1) * gridW) / 32;
            int cellW = next_x - x;

            int y = gridY + cellH * (11 - normNote); // Higher notes at top

            juce::Rectangle<int> rect(x, y, cellW, cellH);

            g.setTiledImageFill(genTextureImage, 0, 0, 1.0f);
            g.fillRect(rect);
        }
    }

    if (!capturedMelody.empty() && capturedMelody.size() == 32)
    {
        int gridX = 26;
        int gridY = 120;
        int gridW = 614;
        int gridH = 360;

        int cellH = gridH / 12; // Should be 30

        for (int i = 0; i < 32; ++i)
        {
            int note = capturedMelody[i];
            if (note == -1) continue;

            int normNote = note % 12;
            if (normNote < 0 || normNote > 11) continue; // Safety check

            int x = gridX + (i * gridW) / 32;
            int next_x = gridX + ((i + 1) * gridW) / 32;
            int cellW = next_x - x;

            int y = gridY + cellH * (11 - normNote); // Higher notes at top

            juce::Rectangle<int> rect(x, y, cellW, cellH);

            // Detect overlap: same i and same normNote in processed generated melody
            bool isOverlap = false;
            if (i < processedMelody.size())
            {
                int genNote = processedMelody[i];
                if (genNote != -1 && (genNote % 12) == normNote)
                {
                    isOverlap = true;
                }
            }

            g.setTiledImageFill(isOverlap ? overlapTextureImage : capturedTextureImage, 0, 0, 1.0f);
            g.fillRect(rect);
        }
    }



    // draw 1px solid white line at x: 25, y: 479, w: 614, h: 1
    // draw 1px solid white line at x: 639, y: 120, w: 1, h: 360

    g.setColour(foregroundColor);
    g.fillRect(25, 479, 614, 1);
    g.fillRect(639, 120, 1, 360);


}

void CounterTuneAudioProcessorEditor::resized()
{

}


void CounterTuneAudioProcessorEditor::drawDottedLine(juce::Graphics& g, int x, int y, int width, int height)
{
    g.setColour(foregroundColor);

    float dashes[] = { 1.0f, 1.0f };

    if (width > 1 && height == 1) // horizontal
    {
        juce::Line<float> line(static_cast<float>(x), static_cast<float>(y) + 0.5f, static_cast<float>(x + width), static_cast<float>(y) + 0.5f);
        g.drawDashedLine(line, dashes, 2, 1.0f);
    }
    else if (height > 1 && width == 1) // vertical
    {
        juce::Line<float> line(static_cast<float>(x) + 0.5f, static_cast<float>(y), static_cast<float>(x) + 0.5f, static_cast<float>(y + height));
        g.drawDashedLine(line, dashes, 2, 1.0f);
    }
}

void CounterTuneAudioProcessorEditor::setupPresetMenu()
{
    // a good dropdown menu can be created simply with a rectangle and buttons
    presetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(parameters, "preset", hiddenPresetKnob);
    hiddenPresetKnob.setSliderStyle(juce::Slider::LinearBar);
    hiddenPresetKnob.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    hiddenPresetKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    hiddenPresetKnob.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    hiddenPresetKnob.setColour(juce::Slider::trackColourId, foregroundColor);
    hiddenPresetKnob.setColour(juce::Slider::thumbColourId, foregroundColor);
    hiddenPresetKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    hiddenPresetKnob.setBounds(-10, -10, 10, 10);
    hiddenPresetKnob.setRange(1, 5, 1);
    hiddenPresetKnob.onValueChange = [this]() { updatePresetLabel(); };
    hiddenPresetKnob.setVisible(false);
    addAndMakeVisible(hiddenPresetKnob);

    presetTitleButton.onClick = [this]()
    {
        presetBackgroundBox.setVisible(true);
        presetBackgroundBox.grabKeyboardFocus();
        // make all the menu option text boxes and buttons visible here too
        presetOption1.setVisible(true);
        presetOption1Button.setVisible(true);
        presetOption2.setVisible(true);
        presetOption2Button.setVisible(true);
        presetOption3.setVisible(true);
        presetOption3Button.setVisible(true);
        presetOption4.setVisible(true);
        presetOption4Button.setVisible(true);
    };

    // unanimated preset menu settings
    addAndMakeVisible(presetTitleLabel);
    presetTitleLabel.setBounds(481, 22, 120, 14);
    presetTitleLabel.setJustification(juce::Justification::centredLeft);
    presetTitleLabel.setFont(getCustomFont(18.0f));
    presetTitleLabel.setReadOnly(true);
    presetTitleLabel.setCaretVisible(false);
    presetTitleLabel.setMouseCursor(juce::MouseCursor::NormalCursor);

    addAndMakeVisible(presetBackgroundBox);
    presetBackgroundBox.setVisible(false);
    presetBackgroundBox.setBounds(481, 40, 140, 80);
    presetBackgroundBox.setColour(juce::TextEditor::textColourId, foregroundColor);
    presetBackgroundBox.setColour(juce::TextEditor::backgroundColourId, backgroundColor);
    presetBackgroundBox.setColour(juce::TextEditor::outlineColourId, foregroundColor);
    presetBackgroundBox.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    presetBackgroundBox.setReadOnly(true);
    presetBackgroundBox.setCaretVisible(false);
    presetBackgroundBox.setMouseCursor(juce::MouseCursor::NormalCursor);
    presetBackgroundBox.onFocusLost = [this]()
    {
        presetBackgroundBox.setVisible(false);
        // make all the menu option text boxes + buttons invisible here too
        presetOption1.setVisible(false);
        presetOption1Button.setVisible(false);
        presetOption2.setVisible(false);
        presetOption2Button.setVisible(false);
        presetOption3.setVisible(false);
        presetOption3Button.setVisible(false);
        presetOption4.setVisible(false);
        presetOption4Button.setVisible(false);
//        isMenuDown = false;
//        presetTitleButton.setEnabled(true);
    };

    addAndMakeVisible(presetOption1);
    presetOption1.setVisible(false);
    presetOption1.setBounds(481, 40, 140, 20);
    presetOption1.setJustification(juce::Justification::centredLeft);
    presetOption1.setFont(getCustomFont(18.0f));
    presetOption1.setColour(juce::TextEditor::textColourId, foregroundColor);
    presetOption1.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    presetOption1.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    presetOption1.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    presetOption1.setReadOnly(true);
    presetOption1.setCaretVisible(false);
    presetOption1.setMouseCursor(juce::MouseCursor::NormalCursor);
    presetOption1.setText("-", dontSendNotification);
    addAndMakeVisible(presetOption1Button);
    presetOption1Button.setVisible(false);
    presetOption1Button.setBounds(481, 40, 140, 20);

    addAndMakeVisible(presetOption2);
    presetOption2.setVisible(false);
    presetOption2.setBounds(481, 60, 140, 20);
    presetOption2.setJustification(juce::Justification::centredLeft);
    presetOption2.setFont(getCustomFont(18.0f));
    presetOption2.setColour(juce::TextEditor::textColourId, foregroundColor);
    presetOption2.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    presetOption2.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    presetOption2.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    presetOption2.setReadOnly(true);
    presetOption2.setCaretVisible(false);
    presetOption2.setMouseCursor(juce::MouseCursor::NormalCursor);
    presetOption2.setText("MINIMAL", dontSendNotification);
    addAndMakeVisible(presetOption2Button);
    presetOption2Button.setVisible(false);
    presetOption2Button.setBounds(481, 60, 140, 20);

    addAndMakeVisible(presetOption3);
    presetOption3.setVisible(false);
    presetOption3.setBounds(481, 80, 140, 20);
    presetOption3.setJustification(juce::Justification::centredLeft);
    presetOption3.setFont(getCustomFont(18.0f));
    presetOption3.setColour(juce::TextEditor::textColourId, foregroundColor);
    presetOption3.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    presetOption3.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    presetOption3.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    presetOption3.setReadOnly(true);
    presetOption3.setCaretVisible(false);
    presetOption3.setMouseCursor(juce::MouseCursor::NormalCursor);
    presetOption3.setText("MODERATE", dontSendNotification);
    addAndMakeVisible(presetOption3Button);
    presetOption3Button.setVisible(false);
    presetOption3Button.setBounds(481, 80, 140, 20);

    addAndMakeVisible(presetOption4);
    presetOption4.setVisible(false);
    presetOption4.setBounds(481, 100, 140, 20);
    presetOption4.setJustification(juce::Justification::centredLeft);
    presetOption4.setFont(getCustomFont(18.0f));
    presetOption4.setColour(juce::TextEditor::textColourId, foregroundColor);
    presetOption4.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    presetOption4.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    presetOption4.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    presetOption4.setReadOnly(true);
    presetOption4.setCaretVisible(false);
    presetOption4.setMouseCursor(juce::MouseCursor::NormalCursor);
    presetOption4.setText("MAXIMAL", dontSendNotification);
    addAndMakeVisible(presetOption4Button);
    presetOption4Button.setVisible(false);
    presetOption4Button.setBounds(481, 100, 140, 20);
}

void CounterTuneAudioProcessorEditor::paintPresetMenu(juce::Graphics& g)
{
    if (!presetTitleButton.isMouseOver())
    {
//        g.drawImage(presetMenuDefault, juce::Rectangle<float>(481, 21, 140, 18));
        g.drawImage(presetMenuDefault, juce::Rectangle<float>(481, 20, 140, 20));
        // clear text, then set color, then set text
        presetTitleLabel.setText("", dontSendNotification);
        presetTitleLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
        presetTitleLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
        presetTitleLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        presetTitleLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        presetTitleLabel.setText(presetTitleText, dontSendNotification);
    }
    else
    {
//        g.drawImage(presetMenuHover, juce::Rectangle<float>(481, 21, 140, 18));
        g.drawImage(presetMenuHover, juce::Rectangle<float>(481, 20, 140, 20));
        presetTitleLabel.setText("", dontSendNotification);
        presetTitleLabel.setColour(juce::TextEditor::textColourId, backgroundColor);
        presetTitleLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
        presetTitleLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        presetTitleLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        presetTitleLabel.setText(presetTitleText, dontSendNotification);
    }

    // button needs to be in front of the text
    addAndMakeVisible(presetTitleButton);
//    presetTitleButton.setBounds(481, 21, 140, 18);
    presetTitleButton.setBounds(481, 20, 140, 20);

    if (presetOption1Button.isMouseButtonDown())
        hiddenPresetKnob.setValue(1);
    if (presetOption2Button.isMouseButtonDown())
        hiddenPresetKnob.setValue(2);
    if (presetOption3Button.isMouseButtonDown())
        hiddenPresetKnob.setValue(3);
    if (presetOption4Button.isMouseButtonDown())
        hiddenPresetKnob.setValue(4);

    if (!presetOption1Button.isMouseOver())
    {
        presetOption1.setText("", dontSendNotification);
        presetOption1.setColour(juce::TextEditor::textColourId, foregroundColor);
        presetOption1.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
        presetOption1.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        presetOption1.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        presetOption1.setText("-", dontSendNotification);
    }
    else
    {
        presetOption1.setText("", dontSendNotification);
        presetOption1.setColour(juce::TextEditor::textColourId, backgroundColor);
        presetOption1.setColour(juce::TextEditor::backgroundColourId, foregroundColor);
        presetOption1.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        presetOption1.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        presetOption1.setText("-", dontSendNotification);
    }

    if (!presetOption2Button.isMouseOver())
    {
        presetOption2.setText("", dontSendNotification);
        presetOption2.setColour(juce::TextEditor::textColourId, foregroundColor);
        presetOption2.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
        presetOption2.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        presetOption2.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        presetOption2.setText("MINIMAL", dontSendNotification);
    }
    else
    {
        presetOption2.setText("", dontSendNotification);
        presetOption2.setColour(juce::TextEditor::textColourId, backgroundColor);
        presetOption2.setColour(juce::TextEditor::backgroundColourId, foregroundColor);
        presetOption2.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        presetOption2.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        presetOption2.setText("MINIMAL", dontSendNotification);
    }

    if (!presetOption3Button.isMouseOver())
    {
        presetOption3.setText("", dontSendNotification);
        presetOption3.setColour(juce::TextEditor::textColourId, foregroundColor);
        presetOption3.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
        presetOption3.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        presetOption3.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        presetOption3.setText("MODERATE", dontSendNotification);
    }
    else
    {
        presetOption3.setText("", dontSendNotification);
        presetOption3.setColour(juce::TextEditor::textColourId, backgroundColor);
        presetOption3.setColour(juce::TextEditor::backgroundColourId, foregroundColor);
        presetOption3.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        presetOption3.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        presetOption3.setText("MODERATE", dontSendNotification);
    }

    if (!presetOption4Button.isMouseOver())
    {
        presetOption4.setText("", dontSendNotification);
        presetOption4.setColour(juce::TextEditor::textColourId, foregroundColor);
        presetOption4.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
        presetOption4.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        presetOption4.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        presetOption4.setText("MAXIMAL", dontSendNotification);
    }
    else
    {
        presetOption4.setText("", dontSendNotification);
        presetOption4.setColour(juce::TextEditor::textColourId, backgroundColor);
        presetOption4.setColour(juce::TextEditor::backgroundColourId, foregroundColor);
        presetOption4.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        presetOption4.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        presetOption4.setText("MAXIMAL", dontSendNotification);
    }
}

void CounterTuneAudioProcessorEditor::setupParams()
{
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
//    keyValueLabel.setInputRestrictions(10, "0123456789.-+");
    keyValueLabel.setSelectAllWhenFocused(true);
    keyValueLabel.setFont(getCustomFont(18.0f));
    keyValueLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    keyValueLabel.setColour(juce::TextEditor::backgroundColourId, backgroundColor);
    keyValueLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    keyValueLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    updateKeyValueLabel();
    //auto commitKey = [this]()
    //{
    //    keyValueLabel.moveCaretToEnd(false);

    //    juce::String text = keyValueLabel.getText().trim();
    //    int value = text.getIntValue();
    //    if (text.isEmpty())
    //    {
    //        updateKeyValueLabel();
    //        return;
    //    }
    //    value = juce::jlimit(0, 12, value);
    //    keyKnob.setValue(value);
    //    updateKeyValueLabel();

    //    grabKeyboardFocus();
    //};
    //keyValueLabel.onReturnKey = commitKey;
    //keyValueLabel.onFocusLost = commitKey;

    auto commitKey = [this]()
        {
            keyValueLabel.moveCaretToEnd(false);

            juce::String text = keyValueLabel.getText().trim().toUpperCase();

            if (text.isEmpty())
            {
                updateKeyValueLabel();
                return;
            }

            // Try parsing as integer first (backward compat)
            bool isNum = true;
            for (auto c : text) { if (!juce::CharacterFunctions::isDigit(c)) { isNum = false; break; } }
            if (isNum)
            {
                int value = text.getIntValue();
                if (value >= 0 && value <= 12)
                {
                    keyKnob.setValue(value);
                    updateKeyValueLabel();
                    grabKeyboardFocus();
                    return;
                }
            }

            // Then try sharp/flat names
            int value = keyNames.indexOf(text);
            if (value == -1)
                value = altKeyNames.indexOf(text);

            if (value >= 0 && value <= 12)
            {
                keyKnob.setValue(value);
                updateKeyValueLabel();
            }
            else
            {
                updateKeyValueLabel(); // Revert if invalid
            }

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
    notesValueLabel.onReturnKey = commitNotes;
    notesValueLabel.onFocusLost = commitNotes;


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
//    chaosKnob.setRange(0.1, 1.0, 0.1);
    chaosKnob.setRange(1, 10, 1);
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
        //float value = text.getFloatValue();
        int value = text.getIntValue();
//        if (text.isEmpty() || !std::isfinite(value))
        if (text.isEmpty())
        {
            updateChaosValueLabel();
            return;
        }
//        value = juce::jlimit(0.1f, 1.0f, value);
        value = juce::jlimit(1, 10, value);
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

    addAndMakeVisible(loopValueLabel);
    loopValueLabel.setBounds(505, 100, 60, 16);
    loopValueLabel.setJustification(juce::Justification::centred);
    loopValueLabel.setFont(getCustomFont(18.0f));
    loopValueLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
    loopValueLabel.setColour(juce::TextEditor::backgroundColourId, backgroundColor);
    loopValueLabel.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    loopValueLabel.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    loopValueLabel.setReadOnly(true);
    loopValueLabel.setCaretVisible(false);
    loopValueLabel.setMouseCursor(juce::MouseCursor::NormalCursor);
    updateLoopValueLabel();

    loopAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(parameters, "loop", loopButton);
    loopButton.setClickingTogglesState(true);
    updateLoopButtonTheme();
    loopButton.setBounds(505, 80, 60, 20);
    loopButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    loopButton.onClick = [this]()
    { 
        updateLoopValueLabel();
        updateLoopButtonTheme();
    };
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
}
