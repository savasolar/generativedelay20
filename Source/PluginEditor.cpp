// PluginEditor.cpp

#include "PluginProcessor.h"
#include "PluginEditor.h"

EnCounterAudioProcessorEditor::EnCounterAudioProcessorEditor (EnCounterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (1000, 496);
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::encounterbodywide_png, BinaryData::encounterbodywide_pngSize);

    customLookAndFeel = std::make_unique<CustomLookAndFeel>();
    setLookAndFeel (customLookAndFeel.get());
        
    // Section 1
    addAndMakeVisible(section1_RunButton);
    section1_RunButton.setBounds(80, 40, 80, 40);
    section1_RunButton.setButtonText("Run");
    section1_RunButton.onClick = [this]
    {
        int units = section1_InputBox.getText().getIntValue();
        double sampleRate = audioProcessor.getSampleRate();
        int samples = static_cast<int>(units * sampleRate);
        audioProcessor.section1_samplesToRecord.store(samples);
        audioProcessor.section1_writePos.store(0);
        audioProcessor.section1_recordingComplete.store(false);
        audioProcessor.section1_isRecording.store(true);
        section1_RunButton.setEnabled(false);
    };
    
    addAndMakeVisible(section1_PlayButton);
    section1_PlayButton.setBounds(80, 80, 80, 40);
    section1_PlayButton.setButtonText("Play");
    section1_PlayButton.setEnabled(false);
    section1_PlayButton.onClick = [this]
    {
        if (audioProcessor.section1_writePos.load() > 0)
        {
            audioProcessor.section1_readPos.store(0);
            audioProcessor.section1_isPlaying.store(true);
        }
    };
    
    addAndMakeVisible(section1_InputBox);
    section1_InputBox.setBounds(0, 40, 80, 80);
    section1_InputBox.setFont(juce::Font(22.0f));
    section1_InputBox.setText("8");
    section1_InputBox.setReadOnly(false);
    section1_InputBox.setJustification(juce::Justification::centred);
    section1_InputBox.onTextChange = [this]
    {
        DBG (section1_InputBox.getText());
    };

    addAndMakeVisible(section1_waveform);
    section1_waveform.setBounds(160, 0, 840, 120);
    
    // Section 2
    
    addAndMakeVisible(section2_RunButton);
    section2_RunButton.setEnabled(false);
    section2_RunButton.setBounds(80, 164, 80, 40);
    section2_RunButton.setButtonText("Run");
    section2_RunButton.onClick = [this]
    {
        audioProcessor.section2_isIsolating.store(true);
        section2_RunButton.setEnabled(false);
    };
    
    addAndMakeVisible(section2_PlayButton);
    section2_PlayButton.setEnabled(false);
    section2_PlayButton.setBounds(80, 204, 80, 40);
    section2_PlayButton.setButtonText("Play");
    section2_PlayButton.onClick = [this]
    {
        if (audioProcessor.section2_writePos.load() > 0)
        {
            audioProcessor.section2_readPos.store(0);
            audioProcessor.section2_isPlaying.store(true);
        }
    };
    
    addAndMakeVisible(section2_InputBox);
    section2_InputBox.setEnabled(false);
    section2_InputBox.setBounds(0, 164, 80, 80);
    section2_InputBox.setFont(juce::Font(22.0f));
    section2_InputBox.setText("NaN");
    section2_InputBox.setReadOnly(true);
    section2_InputBox.setJustification(juce::Justification::centred);
    section2_InputBox.onTextChange = [this]
    {
        DBG (section2_InputBox.getText());
    };

    addAndMakeVisible(section2_waveform);
    section2_waveform.setBounds(160, 124, 840, 124);
    
    // Section 3
    
    addAndMakeVisible(section3_RunButton);
    section3_RunButton.setEnabled(false);
    section3_RunButton.setBounds(80, 288, 80, 40);
    section3_RunButton.setButtonText("Run");
    section3_RunButton.onClick = [this]
    {
        int units = section3_InputBox.getText().getIntValue();
        audioProcessor.section3_timeStretchLength.store(units);
        audioProcessor.section3_isTimeStretching.store(true);
        section3_RunButton.setEnabled(false);
    };
    
    addAndMakeVisible(section3_PlayButton);
    section3_PlayButton.setEnabled(false);
    section3_PlayButton.setBounds(80, 328, 80, 40);
    section3_PlayButton.setButtonText("Play");
    section3_PlayButton.onClick = [this]
    {
        if (audioProcessor.section3_writePos.load() > 0)
        {
            audioProcessor.section3_readPos.store(0);
            audioProcessor.section3_isPlaying.store(true);
        }
    };
    
    addAndMakeVisible(section3_InputBox);
    section3_InputBox.setEnabled(false);
    section3_InputBox.setBounds(0, 288, 80, 80);
    section3_InputBox.setFont(juce::Font(22.0f));
    section3_InputBox.setText("8");
    section3_InputBox.setReadOnly(false);
    section3_InputBox.setJustification(juce::Justification::centred);
    section3_InputBox.onTextChange = [this]
    {
        DBG (section3_InputBox.getText());
    };
    
    addAndMakeVisible(section3_waveform);
    section3_waveform.setBounds(160, 248, 840, 124);
    
    // Section 4
    
    addAndMakeVisible(section4_RunButton);
    section4_RunButton.setEnabled(false);
    section4_RunButton.setBounds(80, 412, 80, 40);
    section4_RunButton.setButtonText("Run");
    section4_RunButton.onClick = []
    {
        DBG("test");
    };
    
    addAndMakeVisible(section4_PlayButton);
    section4_PlayButton.setEnabled(false);
    section4_PlayButton.setBounds(80, 452, 80, 40);
    section4_PlayButton.setButtonText("Play");
    section4_PlayButton.setEnabled(false);
    section4_PlayButton.onClick = []
    {
        DBG("test");
    };
    
    addAndMakeVisible(section4_InputBox);
    section4_InputBox.setEnabled(false);
    section4_InputBox.setBounds(0, 412, 80, 80);
    section4_InputBox.setFont(juce::Font(22.0f));
    section4_InputBox.setText("69");
    section4_InputBox.setReadOnly(false);
    section4_InputBox.setJustification(juce::Justification::centred);
    section4_InputBox.onTextChange = [this]
    {
        DBG(section4_InputBox.getText());
    };
    
    


    
    startTimer(30);
}

EnCounterAudioProcessorEditor::~EnCounterAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void EnCounterAudioProcessorEditor::timerCallback()
{
    if (audioProcessor.section1_recordingComplete.load())
    {
        audioProcessor.section1_recordingComplete.store(false);
        section1_PlayButton.setEnabled(true);
        section1_RunButton.setEnabled(true);
        // set waveform:

//        section1_waveform.numSamples = audioProcessor.section1_writePos.load();
//        section1_waveform.repaint();

        section1_waveform.setAudioBuffer(&audioProcessor.section1_audio, audioProcessor.section1_writePos.load());

        // enable next section to be run
        section2_RunButton.setEnabled(true);
        section2_InputBox.setEnabled(true);
    }

    if (audioProcessor.section2_isolationComplete.load())
    {
        audioProcessor.section2_isolationComplete.store(false);
        section2_PlayButton.setEnabled(true);
        section2_RunButton.setEnabled(true);
        // do section 2 waveform

        section2_waveform.setAudioBuffer(&audioProcessor.section2_audio, audioProcessor.section2_writePos.load());

        // do section 2 pitch value display
        section2_InputBox.setText(juce::String(audioProcessor.section2_midiValue.load()));

        // enable next section to be run
        section3_RunButton.setEnabled(true);
        section3_InputBox.setEnabled(true);
    }
    
    if (audioProcessor.section3_isTimeStretchComplete.load())
    {
        audioProcessor.section3_isTimeStretchComplete.store(false);
        section3_PlayButton.setEnabled(true);
        section3_RunButton.setEnabled(true);
        
        section3_waveform.setAudioBuffer(&audioProcessor.section3_audio, audioProcessor.section1_writePos.load());
        
        section4_RunButton.setEnabled(true);
    }
}

void EnCounterAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.drawImage(backgroundImage, getLocalBounds().toFloat());
    
}

void EnCounterAudioProcessorEditor::resized()
{

}
