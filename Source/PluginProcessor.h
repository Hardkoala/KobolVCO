/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Synth.h"


//#define USE_PGM 0
#define USE_PGM 1

//==============================================================================
//ParameterID:
//==============================================================================

namespace ParameterID
{
    #define PARAMETER_ID(str) const juce::ParameterID str(#str, 1);

   

    PARAMETER_ID(outputLevel)

    PARAMETER_ID(waveF)//WAVEF


    #undef PARAMETER_ID
}


//==============================================================================
//CLASS DEFINITION:
//==============================================================================
/**
*/
//class KOSCAudioProcessor  : public juce::AudioProcessor, private juce::ValueTree::Listener
class KobolVCOAudioProcessor  :
#if USE_PGM==0
public  juce::AudioProcessor, private juce::ValueTree::Listener
#else
public  foleys::MagicProcessor, private juce::ValueTree::Listener
#endif
{
public:
    
    //==============================================================================
    KobolVCOAudioProcessor();
    ~KobolVCOAudioProcessor() override;
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    //APVTS
    juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters", createParameterLayout()};

    
    //==============================================================================
#if USE_PGM==0
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
#endif
    
    //==============================================================================


    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    
private:
    
    Synth synth;
 
    void splitBufferByEvents(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void handleMIDI (uint8_t data0, uint8_t data1, uint8_t data2);
    void render(juce::AudioBuffer<float>& buffer, int sampleCount, int bufferOffset);
    
    void update();
    
  
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    std::atomic<bool> parametersChanged{false};
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override {
        
        parametersChanged.store(true);
    }
    
    

    juce::AudioParameterFloat* outputLevelParam;
    juce::AudioParameterFloat* waveFParam; //wavefParam



    // FOLEYS MAGIC GUI:
    foleys::MagicPlotSource* oscilloscope = nullptr;
    foleys::MagicPlotSource* analyser = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KobolVCOAudioProcessor)
};
