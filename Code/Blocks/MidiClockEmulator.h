#ifndef MIDICLOCKEMULATOR_H_INCLUDED
#define MIDICLOCKEMULATOR_H_INCLUDED

#include "Common/Common.h"

struct ClockEmulator
{
    int32_t* in0; //bpm
    int32_t* in1;//gate
    int32_t* in2; //clockpulse;
    uint32_t mPhase;
    int32_t out0;//clock pulse value
    int32_t out1;//start/stop
};

void tickMidiClockEmulator(struct ClockEmulator* data)
{
    //**********************************************
    //Patchblock code starts here
    //**********************************************

#ifndef EMULATOR

    //Route straight to the output.
    data->out0 = data.in0;
    data->out1 = data.in1;

#else

    //Clocks per second computed with standard 10 fractional bits.
    const uint32_t nbBeatPerMinute = (uint32_t)(*data->in0);
    const uint32_t nbBeatPerSecond = divide(nbBeatPerMinute, (60 << 10), 10);
    const uint32_t nbClocksPerSecond = multiply(nbBeatPerSecond, (24 << 10), 10);

    //Set the clock pulse to either +0.5 or -0.5
    //How many samples should clock pulse be on?
    //I have no idea so I'll just assume it is on only for a single tick.
    //Requires all logic in subsequent blocks to identify the single tick of a zero crossing.
    //Does that break anything?
    //SMP_RATE can have two values: 100 or 20000
    //If we tick this at 100 then every subsequent block ticked at 20000 will pick up the zero crossing.
    //If we tick this at 100 then every subsequent block ticked at 100 will pick up the zero crossing
    //assuming they are all ticked in phase.
    //If we tick this at 20000 then every subsequent block ticked at 100 will most likely miss the zero crossing.
    //If we tick this at 20000 then every subsequent block ticked at 20000 will pick up the zero crossing.
    //What have we learnt?
    //Subsequent blocks must be ticked at the same rate as this block or at a higher rate than this block.
    //Note: phase uses 20 fractional bits.
    if(data->mPhase > (1 << 20))
    {
       //+0.5
       data->out0 = 511;

       //Subtract off 1.0
       data->mPhase &= ((1 << 20) - 1);
    }
    else
    {
       //-0.5
       data->out0 = -511;
    }

    //Set the gate.
    data->out1 = *data->in1;

    //Change in phase per tick.
    //Performed with 20 fractional bits.
    //nbClocksPerSecond must be less than 4096 (2^12) to avoid overflow.
    //120bpm results in a value of 48 so we are fine up to 10k bpm.
    const uint32_t deltaPhase = multiply((nbClocksPerSecond << 10), ((1 << 20) / (20000)), 20);
    data->mPhase += deltaPhase;

#endif // EMULATOR

    //**********************************************
    //Patchblock code ends here
    //**********************************************
}


#endif // MIDICLOCKEMULATOR_H_INCLUDED
