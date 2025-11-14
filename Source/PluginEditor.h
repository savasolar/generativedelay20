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


    // add: 
    //  - dropdown menu /////////////////////////////////////////////////////////////////////////

    juce::TextEditor presetTitleLabel;
    juce::String presetTitleText{ "loading" };
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
            text = "DEFAULT PRESET";
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


    // visual 
    struct WaveformViewer : public juce::Component
    {
        WaveformViewer() = default;

        void setAudioBuffer(const juce::AudioBuffer<float>* buffer, int numSamplesToDisplay)
        {
            audioBuffer = buffer;
            numSamples = numSamplesToDisplay;
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            if (audioBuffer == nullptr || numSamples == 0) return;
            if (audioBuffer->getNumChannels() == 0) return;

            g.setColour(juce::Colour::fromRGB(255, 255, 255));

            float w = static_cast<float> (getWidth());
            float h = static_cast<float> (getHeight()) / 2.0f;
            float centre = static_cast<float> (getHeight()) / 2.0f;

            auto* data = audioBuffer->getReadPointer(0);

            juce::Path p;
            float step = static_cast<float> (numSamples) / w;
            if (step < 1.0f) step = 1.0f;

            bool first = true;
            for (float x = 0; x < w; x += 1.0f)
            {
                int idx = static_cast<int> (x * step);
                if (idx >= numSamples) break;

                float sample = juce::jlimit(-1.0f, 1.0f, data[idx]);
                float y = centre - sample * centre;

                if (first)
                {
                    p.startNewSubPath(x, y);
                    first = false;
                }
                else
                {
                    p.lineTo(x, y);
                }
            }

            g.strokePath(p, juce::PathStrokeType(1.0f));
        }

    private:
        const juce::AudioBuffer<float>* audioBuffer = nullptr;
        int numSamples = 0;
    };

    WaveformViewer voiceBuffer_waveform;




    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CounterTuneAudioProcessorEditor)
};
