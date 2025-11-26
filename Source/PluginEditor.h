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
//    bool isMenuDown = false;
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


    juce::StringArray keyNames{
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "AUTO"
    };

    juce::StringArray altKeyNames{
        "C", "DB", "D", "EB", "E", "F", "GB", "G", "AB", "A", "BB", "B", "AUTO"
    };


    void updateKeyValueLabel()
    {
        //int value = audioProcessor.getKeyInt();
        //juce::String text = juce::String(value);
        //keyValueLabel.setText(text, false);

        int value = audioProcessor.getKeyInt();
        juce::String text = (value >= 0 && value < keyNames.size()) ? keyNames[value] : juce::String(value);
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
//        float value = audioProcessor.getChaosFloat();
        int value = audioProcessor.getChaosInt();
        juce::String text = juce::String(value);
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
    juce::TextEditor loopValueLabel;
    juce::ImageButton loopButton;

    void updateLoopValueLabel()
    {
        bool value = audioProcessor.getLoopBool();
        juce::String text = value ? "ON" : "OFF";
        loopValueLabel.setText(text, dontSendNotification);


    }

    void updateLoopButtonTheme()
    {
        auto switchOn = juce::ImageCache::getFromMemory(BinaryData::switchon_png, BinaryData::switchon_pngSize);
        auto switchOff = juce::ImageCache::getFromMemory(BinaryData::switchoff_png, BinaryData::switchoff_pngSize);

        // Pick the correct image once
        const juce::Image& currentImage = audioProcessor.getLoopBool() ? switchOn : switchOff;

        // Use the exact same image for normal, over and down no visual change on hover or click
        loopButton.setImages(false, true, true,
            currentImage, 1.0f, {},
            currentImage, 1.0f, {},
            currentImage, 1.0f, {});
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
