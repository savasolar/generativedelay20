// PluginEditor.h

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CounterTuneAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    CounterTuneAudioProcessorEditor (CounterTuneAudioProcessor&);
    ~CounterTuneAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    juce::AudioProcessorValueTreeState& parameters;

private:

    void timerCallback() override;
    
    CounterTuneAudioProcessor& audioProcessor;

    bool firstLoad = true;
    
    juce::Image backgroundImage;

    struct TransparentButton : public juce::TextButton
    {
        void paintButton(juce::Graphics&, bool, bool) override {}
    };

    juce::Typeface::Ptr customTypeface;
    juce::Font getCustomFont(float height)
    {
        if (customTypeface == nullptr)
        {
            customTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::ChivoMonoMedium_ttf, BinaryData::ChivoMonoMedium_ttfSize);
        }
        if (customTypeface != nullptr)
        {
            juce::Font font(customTypeface);
            font.setHeight(height);
            return font;
        }
        else
        {
            return juce::Font(height);
        }
    }

    juce::Colour foregroundColor = juce::Colour(0xffffffff);
    juce::Colour backgroundColor = juce::Colour(0xff000000);


    juce::TextEditor presetTitleLabel;
    juce::String presetTitleText{ "" };
    TransparentButton presetTitleButton;
    juce::TextEditor presetBackgroundBox;
    juce::TextEditor presetOption1;
    TransparentButton presetOption1Button;
    juce::TextEditor presetOption2;
    TransparentButton presetOption2Button;
    juce::TextEditor presetOption3;
    TransparentButton presetOption3Button;
    juce::TextEditor presetOption4;
    TransparentButton presetOption4Button;
    juce::Slider hiddenPresetKnob;

    void updatePresetLabel()
    {
        int value = audioProcessor.getPresetInt();
        juce::String text;
        
        
        if (value == 1)
        {
            text = "NO PRESET";
        }
        if (value == 2)
        {
            text = "MINIMAL";
        }
        if (value == 3)
        {
            text = "MODERATE";
        }
        if (value == 4)
        {
            text = "MAXIMAL";
        }
                
        presetTitleText = text;
    }
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> presetAttachment;

    juce::Image presetMenuDefault;
    juce::Image presetMenuHover;

    void setupPresetMenu();
    void paintPresetMenu(juce::Graphics& g);

    // param ui and functionality setup

    juce::TextEditor tempoTitleLabel;
    juce::Slider tempoKnob;
    juce::TextEditor tempoValueLabel;
    void updateTempoValueLabel()
    {
        float value = audioProcessor.getTempoFloat();
        juce::String text = juce::String(value);
        tempoValueLabel.setText(text, false);
    }
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tempoAttachment;

    juce::TextEditor beatsTitleLabel;
    juce::Slider beatsKnob;
    juce::TextEditor beatsValueLabel;
    void updateBeatsValueLabel()
    {
        float value = audioProcessor.getBeatsFloat();
        juce::String text = juce::String(value, 2);
        beatsValueLabel.setText(text, false);
    }
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> beatsAttachment;

    juce::TextEditor keyTitleLabel;
    juce::Slider keyKnob;
    juce::TextEditor keyValueLabel;
    void updateKeyValueLabel()
    {
        int value = audioProcessor.getKeyInt();
        juce::String text = juce::String(value);
        keyValueLabel.setText(text, false);
    }
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> keyAttachment;

    juce::TextEditor notesTitleLabel;
    juce::Slider notesKnob;
    juce::TextEditor notesValueLabel;
    void updateNotesValueLabel()
    {
        int value = audioProcessor.getNotesInt();
        juce::String text = juce::String(value);
        notesValueLabel.setText(text, false);
    }
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> notesAttachment;

    juce::TextEditor chaosTitleLabel;
    juce::Slider chaosKnob;
    juce::TextEditor chaosValueLabel;
    void updateChaosValueLabel()
    {
        float value = audioProcessor.getChaosFloat();
        juce::String text = juce::String(value, 2);
        chaosValueLabel.setText(text, false);
    }
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> chaosAttachment;
    
    juce::TextEditor octaveTitleLabel;
    juce::Slider octaveKnob;
    juce::TextEditor octaveValueLabel;
    void updateOctaveValueLabel()
    {
        int value = audioProcessor.getOctaveInt();
        juce::String text = value > 0 ? "+ " + juce::String(value) : value < 0 ? "- " + juce::String(std::abs(value)) : "0";
        octaveValueLabel.setText(text, false);
    }
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> octaveAttachment;

    juce::TextEditor detuneTitleLabel;
    juce::Slider detuneKnob;
    juce::TextEditor detuneValueLabel;
    void updateDetuneValueLabel()
    {
        float value = audioProcessor.getDetuneFloat();
        juce::String text = juce::String(value, 2);
        detuneValueLabel.setText(text, false);
    }
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> detuneAttachment;
    
    juce::TextEditor loopTitleLabel;
    TransparentButton loopButton;
    juce::TextEditor loopOnLabel;
    juce::TextEditor loopOffLabel;

    void updateLoopValueLabel()
    {
        bool value = audioProcessor.getLoopBool();

        if (value)
        {
            loopOnLabel.setText("", dontSendNotification);
            loopOnLabel.setColour(juce::TextEditor::textColourId, backgroundColor);
            loopOnLabel.setText("ON", dontSendNotification);
            loopOnLabel.setColour(juce::TextEditor::backgroundColourId, foregroundColor);

            loopOffLabel.setText("", dontSendNotification);
            loopOffLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
            loopOffLabel.setText("OFF", dontSendNotification);
            loopOffLabel.setColour(juce::TextEditor::backgroundColourId, backgroundColor);
        }
        else
        {
            loopOnLabel.setText("", dontSendNotification);
            loopOnLabel.setColour(juce::TextEditor::textColourId, foregroundColor);
            loopOnLabel.setText("ON", dontSendNotification);
            loopOnLabel.setColour(juce::TextEditor::backgroundColourId, backgroundColor);

            loopOffLabel.setText("", dontSendNotification);
            loopOffLabel.setColour(juce::TextEditor::textColourId, backgroundColor);
            loopOffLabel.setText("OFF", dontSendNotification);
            loopOffLabel.setColour(juce::TextEditor::backgroundColourId, foregroundColor);
        }
    }
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> loopAttachment;


    juce::TextEditor mixTitleLabel;
    juce::Slider mixKnob;
    juce::TextEditor mixValueLabel;
    void updateMixValueLabel()
    {
        float value = audioProcessor.getMixFloat();
        juce::String text = juce::String(value, 2);
        mixValueLabel.setText(text, false);
    }
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;



    void setupParams();
    void drawDottedLine(juce::Graphics& g, int x, int y, int width, int height);



    juce::Image capturedTextureImage;
    juce::Image overlapTextureImage;
    juce::Image genTextureImage;

    // Add member variables for melodies, after other members like WaveformViewer
    std::vector<int> capturedMelody;
    std::vector<int> generatedMelody;




    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CounterTuneAudioProcessorEditor)
};
