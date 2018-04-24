#ifndef MIDINOTEROUTER_H_INCLUDED
#define MIDINOTEROUTER_H_INCLUDED

#include "Common/Common.h"

struct MidiNoteRouter
{
    int32_t* in0;   //midi gate
    int32_t* in1;   //midi note
    int32_t* in2;   //midi velocity
    int32_t* in3;   //midi gate
    int32_t* in4;   //midi note
    int32_t* in5;   //midi velocity
    int32_t* in6;   //midi gate
    int32_t* in7;   //midi note
    int32_t* in8;   //midi velocity
    int32_t* in9;   //midi gate
    int32_t* in10;   //midi note
    int32_t* in11;   //midi velocity
    int32_t* in12;   //midi gate
    int32_t* in13;   //midi note
    int32_t* in14;   //midi velocity
    int32_t* in15;   //midi gate
    int32_t* in16;   //midi note
    int32_t* in17;   //midi velocity
    int32_t out0;   //midi gate
    int32_t out1;   //midi note
    int32_t out2;   //midi velocity
    int32_t out3;   //midi gate
    int32_t out4;   //midi note
    int32_t out5;   //midi velocity
    int32_t out6;   //midi gate
    int32_t out7;   //midi note
    int32_t out8;   //midi velocity
    int32_t out9;   //midi gate
    int32_t out10;   //midi note
    int32_t out11;   //midi velocity
    int32_t out12;   //midi gate
    int32_t out13;   //midi note
    int32_t out14;   //midi velocity
    int32_t out15;   //midi gate
    int32_t out16;   //midi note
    int32_t out17;   //midi velocity

    //Editable values
    int32_t mNoteNumbers[6];
};

void tickMidiNoteRouter(struct MidiNoteRouter* data)
{
    const int32_t midiGateIns[6] = {*data->in0, *data->in3, *data->in6, *data->in9, *data->in12, *data->in15};
    const int32_t midiNoteIns[6] = {*data->in1, *data->in4, *data->in7, *data->in10, *data->in13, *data->in16};
    const int32_t midiVelIns[6] = {*data->in2, *data->in5, *data->in8, *data->in11, *data->in14, *data->in17};
    const uint8_t maxNbInputs = 6;

    int32_t* midiGateOuts[6] = {&data->out0, &data->out3, &data->out6, &data->out9, &data->out12, &data->out15};
    int32_t* midiNoteOuts[6] = {&data->out1, &data->out4, &data->out7, &data->out10, &data->out13, &data->out16};
    int32_t* midiVelOuts[6] = {&data->out2, &data->out5, &data->out8, &data->out11, &data->out14, &data->out17};
    const uint8_t maxNbOutputs = 6;

    uint8_t counter = 0;

    uint8_t noteNrToOutputChannel[128];
    memset(noteNrToOutputChannel, 0xff, sizeof(noteNrToOutputChannel));
    for(counter = 0; counter < maxNbInputs; counter++)
    {
        const int32_t noteNr = data->mNoteNumbers[counter] >> 10;
        noteNrToOutputChannel[noteNr] = counter;
    }

    for(counter = 0; counter < maxNbInputs; counter++)
    {
        const int32_t midiNoteIn = midiNoteIns[counter] >> 10;
        const uint8_t outputChannel = noteNrToOutputChannel[midiNoteIn];
        if(outputChannel < maxNbOutputs)
        {
            *midiGateOuts[outputChannel] = midiGateIns[counter];
            *midiNoteOuts[outputChannel] = midiNoteIns[counter];
            *midiVelOuts[outputChannel] = midiVelIns[counter];
        }
    }
}


#endif // MIDINOTEROUTER_H_INCLUDED
