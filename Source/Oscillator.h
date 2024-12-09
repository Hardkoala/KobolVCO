/*
  ==============================================================================

 Oscillator.h


  ==============================================================================
*/

#pragma once


class Oscillator {
public:
    
    float oscValue;
    float amplitude;
    float inc;
    float phase;
    
    float rectThreshold;
    float rectGain;
    float compThreshold;
    float compGain;

    Oscillator();
    void reset();
    
    void setWaveformParameters(float value);
    float comparator(float inputSample, float threshold, float value);
    float rectifier(float inputSample, float threshold);
    float nextSample(float X);
    
    
private:

    const float splitPointA = 0.4f;
    const float splitPointB = 0.8f;

};
