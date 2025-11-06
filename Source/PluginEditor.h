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

private:

    void timerCallback() override;
    
    CounterTuneAudioProcessor& audioProcessor;
    
    juce::Image backgroundImage;

    juce::Typeface::Ptr customTypeface;
    juce::Font getCustomFont(float height)
    {
        if (customTypeface == nullptr)
        {
            customTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::B612MonoRegular_ttf, BinaryData::B612MonoRegular_ttfSize);
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

    juce::Colour foregroundColor = juce::Colour(0xff00ffae);
    juce::Colour backgroundColor = juce::Colour(0xff000000);


    // add: 
    //  - dropdown menu
    //  



    juce::Label testLabel;



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

            g.setColour(juce::Colour::fromRGB(0, 255, 174));

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
