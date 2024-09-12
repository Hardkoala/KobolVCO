/*
  ==============================================================================

    Voice.h

  ==============================================================================
*/

#pragma once
#include "Oscillator.h"



struct Voice
{
    int note;
    Oscillator osc;
    float waveForm;

    
    void reset(){//OSC reset
        note=0;
        osc.reset();
    }
    
    
    float render(){ //Render Nextsample
        return osc.nextSample(waveForm);
    }
    
};

