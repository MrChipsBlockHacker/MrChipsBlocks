#ifndef MIDICLOCKEMULATOR_H_INCLUDED
#define MIDICLOCKEMULATOR_H_INCLUDED

#include "Common/Common.h"

struct MidiClockEmulator
{
    int32_t* in0;           //clock pulse
    int32_t* in1;           //start/stop
    int32_t out0;           //clock pulse
    int32_t out1;           //start/stop
    uint32_t mBPM;
    uint32_t mClockPulseWidth;
    uint32_t mPhase;
    uint32_t mStartTick;
    uint32_t mTickCounter;
};

void tickMidiClockEmulator(struct MidiClockEmulator* data)
{
    //**********************************************
    //Patchblock code starts here
    //**********************************************

#ifndef EMULATOR

    //Route straight to the output.
    data->out0 = *data.in0;
    data->out1 = *data.in1;

#else

    #define MCE_MULTIPLY(a, b, n) ((((a)*(b)) + (1 << ((n)-1))) >> (n))
    #define MCE_DIVIDE(a, b, n) (((a) << (n))/(b))

    //Clocks per second computed with standard 10 fractional bits.
    const uint32_t nbBeatPerSecond = MCE_DIVIDE(data->mBPM, 60 << 10, 10);
    const uint32_t nbClocksPerSecond = MCE_MULTIPLY(nbBeatPerSecond, 24 << 10, 10);

    //Phase uses 17 fractional bits.
    const uint32_t one17 = 1 << 17;
    const uint32_t width17  = data->mClockPulseWidth << 7;
    const uint32_t nbClocksPerSecond17 = nbClocksPerSecond << 7;

    //Set the clock pulse to either +0.5 or -0.5
    //Phase > 1 cycles back round to 0.
    //Phase > 1-width generates 0.5f/on
    //Phase <= 1-width generates -0.5f/off
    //A width of 0 generates on for a single tick.

    if(data->mPhase > one17)
    {
        //+0.5
        data->out0 = 511;

        //Subtract off 1.0
        data->mPhase &= (one17 - 1);
    }
    else if(data->mPhase > (one17 - width17))
    {
        //+0.5
        data->out0 = 511;
    }
    else
    {
        //-0.5
        data->out0 = -511;
    }

    //Set the gate on according to the tick counter.
    if(data->mTickCounter < (data->mStartTick >> 10))
    {
        data->mTickCounter++;
        data->out1 = 0;
    }
    else
    {
        data->out1 = 1 << 10;
    }

    //Change in phase per tick.
    //Performed with 17 fractional bits.
    data->mPhase += nbClocksPerSecond17/SMP_RATE;


#endif // EMULATOR

    //**********************************************
    //Patchblock code ends here
    //**********************************************
}


#endif // MIDICLOCKEMULATOR_H_INCLUDED
