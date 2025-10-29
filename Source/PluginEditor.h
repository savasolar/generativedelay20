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
    

    juce::TextButton startButton;
    juce::TextButton stopButton;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnCounterAudioProcessorEditor)
};
