/*
  ==============================================================================

    PLUGINPROCESSOR.CPP

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "Utils.h"



namespace IDs{
    static juce::Identifier oscilloscope { "oscilloscope" };
    static juce::Identifier fft {"FFT"};
}


//==============================================================================
KobolVCOAudioProcessor::KobolVCOAudioProcessor()

: foleys::MagicProcessor (juce::AudioProcessor::BusesProperties()
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
apvts (*this, nullptr, "PARAMETERS", createParameterLayout())

{

    FOLEYS_SET_SOURCE_PATH (__FILE__);

    castParameter(apvts, ParameterID::outputLevel, outputLevelParam);
    castParameter(apvts, ParameterID::waveF, waveFParam); //WAVEF
    
    apvts.state.addListener(this);
    
    
    // MAGIC GUI: register an oscilloscope to display in the GUI. keep a pointer to push samples into in processBlock(). Only interested in channel 0
    oscilloscope = magicState.createAndAddObject<foleys::MagicOscilloscope>(IDs::oscilloscope, 0);
    analyser = magicState.createAndAddObject<foleys::MagicAnalyser>(IDs::fft, 1);


    magicState.setGuiValueTree (BinaryData::magic_xml, BinaryData::magic_xmlSize); //Loads magic_XML config
}


//==============================================================================

KobolVCOAudioProcessor::~KobolVCOAudioProcessor(){
    
    apvts.state.removeListener(this);
}



//==============================================================================
//PrePareToPlay and releasesResources:
//==============================================================================

void KobolVCOAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // pre-playback initialisation
    synth.allocateResources(sampleRate, samplesPerBlock);
    parametersChanged.store(true);
    analyser->prepareToPlay(sampleRate, samplesPerBlock);
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
    reset();
}

//==============================================================================

void KobolVCOAudioProcessor::releaseResources()
{
    // When playback stops, free up any spare memory, etc.
    synth.deallocateResources();
}
//==============================================================================

#ifndef JucePlugin_PreferredChannelConfigurations
bool KobolVCOAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Check if the layout is supported. Thisc code only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


//==============================================================================
//HandleMIDI
//==============================================================================
void KobolVCOAudioProcessor::handleMIDI (uint8_t data0, uint8_t data1, uint8_t data2){
 
    synth.midiMessage(data0, data1, data2);
}


//==============================================================================
//RENDER AUDIO
//==============================================================================
void KobolVCOAudioProcessor::render(juce::AudioBuffer<float>& buffer, int sampleCount, int bufferOffset){
    
    float* outputBuffers[2]={nullptr, nullptr};
    outputBuffers[0]=buffer.getWritePointer(0)+bufferOffset;
    if(getTotalNumOutputChannels() > 1){
        outputBuffers[1]=buffer.getWritePointer(1);
    }
    synth.render(outputBuffers, sampleCount); //RENDER AUDIO
}


//==============================================================================
//SplitBufferByEvents (Here Audio is Rendered)
//==============================================================================
void KobolVCOAudioProcessor:: splitBufferByEvents(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages){
    
    int bufferOffset=0;
    for (const auto metadata : midiMessages){
        //render audio that happens before the event
        int samplesThisSegment = metadata.samplePosition - bufferOffset;
        if(samplesThisSegment > 0){
            render(buffer, samplesThisSegment, bufferOffset);
            bufferOffset += samplesThisSegment;
        }
        if (metadata.numBytes <=3){
            uint8_t data1 = (metadata.numBytes >= 2) ? metadata.data[1] : 0;
            uint8_t data2 = (metadata.numBytes ==3) ? metadata.data[2] :0 ;
            handleMIDI(metadata.data[0], data1, data2);
        }
    }
    int samplesLastSegment=buffer.getNumSamples() - bufferOffset;
    if(samplesLastSegment > 0){
        render (buffer, samplesLastSegment, bufferOffset);
        
    }
    midiMessages.clear();
}


//==============================================================================
//Reset
//==============================================================================

void KobolVCOAudioProcessor::reset(){
    
    synth.reset();
    synth.outputLevelSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(outputLevelParam->get()));
}

//==============================================================================
//PROCESS BLOCK
//==============================================================================

void KobolVCOAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    

    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    magicState.processMidiBuffer(midiMessages, buffer.getNumSamples() ); //Allows PGM keyboard to be used

    // Clear any output channels that don't contain input data.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }
    
    bool expected=true;
    if(isNonRealtime() || parametersChanged.compare_exchange_strong(expected, false)){
        update();
    }
    
    splitBufferByEvents(buffer, midiMessages); //RENDER here
    
    oscilloscope->pushSamples (buffer);
    analyser->pushSamples (buffer);
}

//==============================================================================
//UPDATE: update Parameters values: outputLevel and waveForm
//==============================================================================
void KobolVCOAudioProcessor::update(){
    
    float waveF=waveFParam->get();
    synth.waveForm=waveF;
    
    synth.outputLevelSmoother.setTargetValue(juce::Decibels::decibelsToGain(outputLevelParam->get()));
}


//==============================================================================
// PARAMETERS:APVTS
//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout KobolVCOAudioProcessor::createParameterLayout(){
    
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    

 
    layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterID::outputLevel,
            "Output Level",
            juce::NormalisableRange<float>(-24.0f, 6.0f, 0.1f),
            0.0f,
            juce::AudioParameterFloatAttributes().withLabel("dB")));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterID::waveF,
            "Wave Form",
            juce::NormalisableRange<float>(0.0f, 1.1f, 0.02f), //
            0.0f,
            juce::AudioParameterFloatAttributes().withLabel("VCO")));

    return layout;

}


//==============================================================================
//PARAMETERS: Saving and loading parameters state
//==============================================================================
void KobolVCOAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Method to store your parameters in the memory block.
    
    magicState.getStateInformation(destData);
    
}


//==============================================================================

void KobolVCOAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore your parameters from this memory block created by the getStateInformation() call.
        magicState.setStateInformation(data, sizeInBytes, getActiveEditor());
}


//==============================================================================
// This creates new instances of the plugin..

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KobolVCOAudioProcessor();
}
//==============================================================================



