// PluginEditor.h

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class EnCounterAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    EnCounterAudioProcessorEditor (EnCounterAudioProcessor&);
    ~EnCounterAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:

    void timerCallback() override;
    
    EnCounterAudioProcessor& audioProcessor;
    
    juce::Image backgroundImage;
    

    struct CustomLookAndFeel : public juce::LookAndFeel_V4
    {
        juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override
        {
            return juce::Font (16.0f);
        }
    };
    

    std::unique_ptr<juce::LookAndFeel> customLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnCounterAudioProcessorEditor)
};
