#ifndef MIDINOTEEMULATOR_H_INCLUDED
#define MIDINOTEEMULATOR_H_INCLUDED

#include "Common/Common.h"

struct MidiNoteEmulator
{
    //Input
    int32_t* in0;           //clock pulse
    int32_t* in1;           //start/stop

    //Output
    int32_t  out0;          //gate
    int32_t  out1;          //note
    int32_t  out2;          //velocity
    int32_t  out3;          //gate
    int32_t  out4;          //note
    int32_t  out5;          //velocity
    int32_t  out6;          //gate
    int32_t  out7;          //note
    int32_t  out8;          //velocity
    int32_t  out9;          //gate
    int32_t  out10;         //note
    int32_t  out11;         //velocity
    int32_t  out12;         //gate
    int32_t  out13;         //note
    int32_t  out14;         //velocity
    int32_t  out15;         //gate
    int32_t  out16;         //note
    int32_t  out17;         //velocity
    int32_t  out18;         //clock pulse
    int32_t  out19;         //start/stop

    //Editable events
    int32_t* mEvents;       //{onClock, offClock, noteNr, outputChannel}
    int32_t  mEvents_length;

    //Progress.
    uint8_t mTide;
    uint8_t mOutputEvents[6];
    uint16_t mMidiClockCounter;
    uint8_t mFirstTickComplete;
    uint8_t mMidiClockTriggerReady;
    uint8_t mOutputChannel;
};

void tickMidiNoteEmulator(struct MidiNoteEmulator* data)
{
    //**********************************************
    //Patchblock code starts here
    //**********************************************

    //Route straight to the output.
    data->out18 = *data->in0;
    data->out19 = *data->in1;

#ifdef EMULATOR

    const int32_t midiClockTrigger = *data->in0;
    const int32_t midiStart = *data->in1;

    int32_t* const midiGateOuts[6] = {&data->out0, &data->out3, &data->out6, &data->out9, &data->out12, &data->out15};
    int32_t* const midiNoteOuts[6] = {&data->out1, &data->out4, &data->out7, &data->out10, &data->out13, &data->out16};
    int32_t* const midiVelOuts[6] = {&data->out2, &data->out5, &data->out8, &data->out11, &data->out14, &data->out17};

    const uint8_t maxNbOutputs = sizeof(midiGateOuts)/sizeof(int32_t*);

    if(0 == data->mFirstTickComplete)
    {
        memset(data->mOutputEvents, 0xff, sizeof(data->mOutputEvents));
        data->mMidiClockCounter = -1;
        data->mFirstTickComplete = 1;
    }

    if (midiStart && midiClockTrigger>0 && data->mMidiClockTriggerReady)
    {
        //Can't receive another pulse until clock trigger goes negative.
        data->mMidiClockTriggerReady = 0;

        //Increment the midi clock counter.
        data->mMidiClockCounter++;
    }
    else if (midiClockTrigger < 0)
    {
       //Trigger has gone negative, be ready to catch next crossing to positive.
       data->mMidiClockTriggerReady = 1;
    }

    //Trigger one event.
    if(data->mTide*4 < data->mEvents_length)
    {
        const int32_t noteOnClock = data->mEvents[4*data->mTide + 0] >> 10;
        if(noteOnClock == data->mMidiClockCounter)
        {
            const int32_t outputChannel = data->mEvents[4*data->mTide + 3] >> 10;

            if(0xff != data->mOutputEvents[outputChannel])
            {
                //Already got an output.
                //Stop it and wait for next tick.
                *midiGateOuts[outputChannel] = 0;
                data->mOutputEvents[outputChannel] = 0xff;
            }
            else
            {
                //Free output.
                //Use it.
                const int32_t noteNr = data->mEvents[4*data->mTide + 2] >> 10;
                *midiGateOuts[outputChannel] = 1 << 10;
                *midiNoteOuts[outputChannel] = noteNr << 10;
                *midiVelOuts[outputChannel] = 100 << 7;
                data->mOutputEvents[outputChannel]= data->mTide;
                data->mTide++;
            }
        }
    }

    //Stop one event.
    const uint8_t eventId = data->mOutputEvents[data->mOutputChannel];
    if(0xff != eventId)
    {
        const uint32_t noteOffClock = data->mEvents[4*eventId + 1] >> 10;
        if(noteOffClock == data->mMidiClockCounter)
        {
             const int32_t outputChannel = data->mEvents[4*eventId + 3] >> 10;
            *midiGateOuts[outputChannel] = 0;
            data->mOutputEvents[outputChannel] = 0xff;
        }
    }

    data->mOutputChannel++;
    if(maxNbOutputs == data->mOutputChannel)
    {
        data->mOutputChannel = 0;
    }


#endif // EMULATOR

    //**********************************************
    //Patchblock code ends here
    //**********************************************
}


#endif // MIDINOTEEMULATOR_H_INCLUDED
