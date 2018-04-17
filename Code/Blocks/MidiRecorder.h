#ifndef MIDIRECORDER2_H_INCLUDED
#define MIDIRECORDER2_H_INCLUDED

#include "Common/Common.h"

struct MidiRecorder
{
    //Input.
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
    int32_t* in18;   //midi clock pulse
    int32_t* in19;   //midi clock start
    int32_t* in20;   //arm recording

    //Output.
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
    int32_t out18;   //metronome clock pulse.


    //Editable input.
    uint32_t mNbCountInClocks;     //Editable input
    uint32_t mNbRecordClocks;      //Editable input
    uint32_t mQuantisation;         //Editable input
    uint32_t mMetronomeQuantisation;//Editable input.

    //Internal data.

    //Inputs and outputs.
    //Only zero these on first tick.
    uint8_t mInputToOutputMap[6];                           //Destination output channel of each input channel.
    uint8_t mOutputChannelMask;                            //8 bits to record used/free status of each output, 1 bit for each output.
    uint8_t mFirstTickComplete;

    //Record and playback events in-flight.
    uint8_t mInputRecordEventIds[6];                         //Ids of in-flight record events.
    uint8_t mOutputPlaybackEventIds[6];                      //Ids of in-flight playback events.

    //Recorded events.
    uint32_t mEvents[32];                                   //Max num events we can record for a loop.
    uint8_t mNbEventsPrev;                                  //Number of events recorded at the start of the loop ready for playback.
    uint8_t mNbEvents;                                      //Number of recorded events at the end of the loop.

    //Midi clock.
    uint8_t mMidiClockTriggerReady;                         //Flag set true on -ve pulse set false on +ve pulse.
    uint16_t mMidiClockCount;                               //Count of midi clocks.
    uint8_t mPhase;                                         //Stopped, waiting for start, record count-in, active.
    uint8_t mTide;                                          //Playback event counter.
    uint16_t mTickCounter;                                  //Tick count since last clock.
    uint16_t mNbTicksPerClock;                              //Num tick counts per clock (cached during record count-in);
    uint8_t mOperationalMode;                               //0 for playback only, 1 for playback and record
};

void tickMidiRecorder(struct MidiRecorder* data)
{
    //**********************************************
    //Patchblock code starts here
    //**********************************************

    //////////////////
    //Note
    /////////////////

    //Midi channel and note are output as Q10 numbers in range (0, 127)
    //Midi velocity is stored as a Q10 number in range (0, 1) where 1 represents a velocity of 127.
    //That means we right shift by 10 to get the note and channel but only by 3 to get the velocity.

    //****************************
    //We record data from different channels
    //and pack all the channels into a uint32_t.
    //Each channel requires a specified number of bytes.
    //9 bits for note on clock count    //4 measures would be 384 so we need 9 bits.
    //9 bits for note off clock count   //4 measures would be 384 so we need 9 bits.
    //7 bits for note number
    //7 bits for velocity

    //Pack note on clock, note off clock, note number and velocity into 32 bits.
    #define MR_PACK_EVENT(on, off, note, vel)      ((on) | ((off) << 9) | ((note) << 18) | (vel) << 25)

    //Increment the note-on clock value. inc can be +ve or -ve.
    #define MR_INC_NOTE_ON_CLOCK(e, inc)           ((e) + (inc))

    //Increment the note-off clock value. inc can be +ve or -ve.
    #define MR_INC_NOTE_OFF_CLOCK(e, inc)          ((e) + ((inc) << 9))

    //Get the note-on clock value in range (0,511)
    #define MR_GET_NOTE_ON_CLOCK(e)                ((e) & ((1 << 9) - 1))

    //Get the note-off clock value in range (0,511)
    #define MR_GET_NOTE_OFF_CLOCK(e)               (((e) >> 9) & ((1 << 9) - 1))

    //Get the note number value in range (0,127)
    #define MR_GET_NOTE_NR(e)                      (((e) >> 18) & ((1 << 7) - 1))

    //Get the velocity value in range (0,127)
    #define MR_GET_VELOCITY(e)                     (((e) >> 25) & ((1 << 7) - 1))

    //Quantise the note-on clock to the specified quantisation.
    #define MR_QUANTIZE_ON(e, q)                   (((((e) & ((1 << 9) - 1)) + (q)/2)/(q))*(q))

    //Get the duration in midi clocks between note-on and note-off.
    #define MR_DURATION(e)                         ((((e) >> 9) & ((1 << 9) - 1)) -  ((e) & ((1 << 9) - 1)))

    //Compute the quantised note-on midi clock and return the note-off midi clock that preserves unquantised duration.
    #define MR_QUANTIZE_OFF(e, q)                  (MR_QUANTIZE_ON((e),(q)) + MR_DURATION((e)))

    //****************************

    #define UNUSED_ID 0xff
    #define MARKED_FOR_REMOVE 0xfe

    //****************************



    //Availability of each output channel.
    #define MR_SET_CHANNEL_USED(e, channel)                ((e) | (1 << (channel)))
    #define MR_SET_CHANNEL_FREE(e, channel)                ((e) & ~(1 << (channel)))
    #define MR_IS_CHANNEL_USED(e, channel)                (((e) >> (channel)) & 1)

    //Support 6 channels at output.
    //If we have more than 6 then we need to
    //revisit wisdom of lookup table.
    static const uint8_t mask[64] =
    {
        0, 1, 0xff, 2,                //0-3
        0xff, 0xff, 0xff, 3,          //4-7
        0xff, 0xff, 0xff, 0xff,       //8-11
        0xff, 0xff, 0xff, 4,          //12-15
        0xff, 0xff, 0xff, 0xff,       //16-19
        0xff, 0xff, 0xff, 0xff,       //20-23
        0xff, 0xff, 0xff, 0xff,       //24-27
        0xff, 0xff, 0xff, 5,          //28-31
        0xff, 0xff, 0xff, 0xff,       //32-25
        0xff, 0xff, 0xff, 0xff,       //36-39
        0xff, 0xff, 0xff, 0xff,       //40-43
        0xff, 0xff, 0xff, 0xff,       //44-47
        0xff, 0xff, 0xff, 0xff,       //48-51
        0xff, 0xff, 0xff, 0xff,       //52-55
        0xff, 0xff, 0xff, 0xff,       //56-60
        0xff, 0xff, 0xff, 0xff       //60-63
    };
    #define MR_LOWEST_FREE_BIT(x)  (mask[(~(x) & ((x) + 1)) - 1])


    //*********************************
    //We will cycle through phases.
    //The trajectory through the phases depends on the operational mode.
    //ModePlay: ePhaseStopped -> ePhaseWaiting -> ePhaseActive
    //ModeRecord: ePhaseStopped -> ePhaseWaiting -> ePhaseRecordCountIn -> ePhaseActive

    //Ideally, we'd use an enum here but I don't think that will work with patchblocks.
    static const uint8_t ePhaseStopped = 0;
    static const uint8_t ePhaseWaiting = 1;
    static const uint8_t ePhaseRecordCountIn = 2;
    static const uint8_t ePhaseActive = 3;

    //Ideally, we'd use an enum here but I don't think that will work with patchblocks.
    //static const uint8_t eOperationalModePlay = 0;
    static const uint8_t eOperationalModeRecord = 1;


    //Get the midi input data for all inputs.
    const int32_t midiGateIns[6] = {*data->in0, *data->in3, *data->in6, *data->in9, *data->in12, *data->in15};
    const int32_t midiNoteIns[6] = {*data->in1, *data->in4, *data->in7, *data->in10, *data->in13, *data->in16};
    const int32_t midiVelIns[6] = {*data->in2, *data->in5, *data->in8, *data->in11, *data->in14, *data->in17};

    //Store the output data for each output.
    int32_t* const midiGateOuts[6] = {&data->out0,&data->out3,&data->out6,&data->out9, &data->out12,&data->out15};
    int32_t* const midiNoteOuts[6] = {&data->out1,&data->out4,&data->out7,&data->out10, &data->out13,&data->out16};
    int32_t* const midiVelOuts[6] = {&data->out2,&data->out5,&data->out8,&data->out11, &data->out14,&data->out17};

    //Store the metronome clock output.
    int32_t* const metronomeClockOut = &data->out18;

    //Unpack the midi clock data.
    const int32_t midiClockTrigger = *data->in18;
    const int32_t midiStart = *data->in19;

    //Unpack the configuration data.
    const uint8_t isRecordingArmed = (*data->in20) >> 10;
    const uint8_t nbCountInClocks = data->mNbCountInClocks >> 10;
    const uint8_t nbRecordClocks = data->mNbRecordClocks >> 10;
    const uint8_t quantisation = data->mQuantisation >> 10;
    const uint8_t metronomeQuantisation = data->mMetronomeQuantisation >> 10;

    //Compute the max size of all the buffers that we will use.
    const uint8_t maxNbInputs = sizeof(midiGateIns)/sizeof(int32_t);
    const uint8_t maxNbOutputs = sizeof(midiGateOuts)/sizeof(int32_t*);
    const uint8_t maxNbEvents = sizeof(data->mEvents) / sizeof(uint32_t);

    //Patchblocks requires loop counter to be declared externally to loop.
    //Must be using something pre-C99.
    uint32_t inputCounter, outputCounter;

    //Clock out is for metronome so we want that to be off
    //unless we are in record or record count-in phases.
    //Set this on later if we encounter the correct conditions.
    *metronomeClockOut = -511;

    //Cache the previous phase so we can track state changes.
    const uint8_t prevPhase = data->mPhase;

    //Let's start the update code.
    //If midiStart is off then reset everything to zero.
    if (0 == midiStart)
    {
        if(0 == data->mFirstTickComplete)
        {
            //Only wipe on very first tick.
            //Ideally, we'd do this in a constructor but I don't know how
            //patchblocks initialises data.
            //I couldn't get struct member variables to initialise to anything other than zero from xml.
            memset(data->mInputToOutputMap, UNUSED_ID, sizeof(data->mInputToOutputMap));
            data->mOutputChannelMask = 0;
            memset(data->mInputRecordEventIds, UNUSED_ID, sizeof(data->mInputRecordEventIds));
            memset(data->mOutputPlaybackEventIds,  UNUSED_ID, sizeof(data->mOutputPlaybackEventIds));
            memset(data->mEvents, 0, sizeof(data->mEvents));
            data->mNbEvents = 0;
            data->mFirstTickComplete = 1;
        }

        data->mNbEventsPrev = data->mNbEvents;

        data->mMidiClockTriggerReady = 0;
        data->mMidiClockCount = 0;
        data->mPhase = 0;
        data->mTide = 0;
        data->mTickCounter = 0;
        data->mNbTicksPerClock = 0;

        data->mOperationalMode = isRecordingArmed;
    }
    else if(ePhaseStopped == data->mPhase)
    {
        //We must have started ie midiStart != 0
        //If we were in stopped phase then set phase to waiting for midi clock.
        //Note: we are going to wait for the next clock pulse before either
        //playing or recording.
        //http://midi.teragonaudio.com/tech/midispec/seq.htm
        //Note we cannot immediately switch out of ePhaseWaiting because
        //mMidiClockTriggerReady is false.
        data->mPhase = ePhaseWaiting;
    }

    //Have we got the start of a clock pulse?
    //Increment clock count, increment phase as required, zero tick counter.
    uint8_t rewindToStart = 0;
    if ((midiClockTrigger> 0) && data->mMidiClockTriggerReady)
    {
        //Can't receive another pulse until clock trigger goes negative.
        data->mMidiClockTriggerReady = 0;

        if(ePhaseWaiting == data->mPhase)
        {
             //We were waiting for a clock pulse and now we've got it.
             //Start playback or record count-in depending on mode.
             data->mPhase = (eOperationalModeRecord == data->mOperationalMode) ? ePhaseRecordCountIn : ePhaseActive;
        }
        else if(ePhaseRecordCountIn == data->mPhase)
        {
            //Cache the number of ticks per clock.
            data->mNbTicksPerClock = data->mTickCounter;
            data->mTickCounter = 0;

            //We are in count-in mode.
            //Increment the clock count.
            //If we hit the end of the count-in then
            //start recording.
            data->mMidiClockCount++;
            if (nbCountInClocks == data->mMidiClockCount)
            {
                 //Set everything to zero.
                data->mMidiClockCount = 0;

                //Set the phase to active.
                data->mPhase = ePhaseActive;
            }
        }
        else if(ePhaseActive == data->mPhase)
        {
            //Set the tick counter back to zero.
            data->mTickCounter = 0;

            data->mMidiClockCount++;
            if (nbRecordClocks == data->mMidiClockCount)
            {
                //Wind back to the start again.
                data->mMidiClockCount = 0;
                data->mTide = 0;
                rewindToStart = 1;
            }
        }
    }
    else if (midiClockTrigger < 0)
    {
       //Trigger has gone negative, be ready to catch next crossing to positive.
       data->mMidiClockTriggerReady = 1;
    }

    //Have we just wound back to the start in active mode with recording armed?
    //We will need to sort the old and new events if that is the case.
    if((rewindToStart != 0) && (eOperationalModeRecord == data->mOperationalMode))
    {
        //Any hanging notes that were just recorded need to be truncated.
        for(inputCounter = 0; inputCounter < maxNbInputs; inputCounter++)
        {
            const uint8_t recordEventId = data->mInputRecordEventIds[inputCounter];
            if(UNUSED_ID != recordEventId)
            {
                const uint8_t recordEventId = data->mInputRecordEventIds[inputCounter];
                data->mEvents[recordEventId] = MR_INC_NOTE_OFF_CLOCK(data->mEvents[recordEventId], nbRecordClocks);
            }
        }

        //Sort all events recorded in the last loop with all events
        //recorded prior to last loop.
        if((data->mNbEvents > data->mNbEventsPrev) && (data->mNbEventsPrev > 0))
        {
            //Write to this array on the stack from data->mEvents.
            //I'm reasoning that writing to stack is less likely to
            //result in load-hit-store issues but this may be false.
            uint32_t events[maxNbEvents];
            uint8_t nbEvents = 0;

            //First ordered array.
            const uint32_t* const eventsA = data->mEvents;
            const uint8_t nbEventsA = data->mNbEventsPrev;
            uint8_t countA = 0;

            //Second ordered array.
            const uint32_t* const eventsB = data->mEvents + data->mNbEventsPrev;
            const uint8_t nbEventsB = data->mNbEvents - data->mNbEventsPrev;
            uint8_t countB = 0;

            //Sort until we reach the end of either ordered array.
            while((countA < nbEventsA) && (countB < nbEventsB))
            {
                const uint16_t onA = MR_GET_NOTE_ON_CLOCK(eventsA[countA]);
                const uint16_t onB = MR_GET_NOTE_ON_CLOCK(eventsB[countB]);
                events[nbEvents++] = (onA <= onB) ? eventsA[countA++] : eventsB[countB++];
            }

            //Sort the remainder with a direct copy.
            if(countA < nbEventsA)
            {
                memcpy(events + nbEvents, eventsA + countA, sizeof(uint32_t)*(nbEventsA - countA));
            }
            else
            {
               memcpy(events + nbEvents, eventsB + countB, sizeof(uint32_t)*(nbEventsB - countB));
            }

            //Copy from stack to heap.
            memcpy(data->mEvents, events, sizeof(uint32_t)*data->mNbEvents);
        }

        //Cache the number of events for next time we rewind.
        data->mNbEventsPrev = data->mNbEvents;
    }

    //Set the output metronome clock.
    //Remember that we already set it to -511 at the start of the function.
    if((ePhaseRecordCountIn == data->mPhase || ePhaseActive == data->mPhase) && (0 == (data->mMidiClockCount % metronomeQuantisation)))
    {
        //We are in record count-in phase or recording phase.
        //We are on the correct beat.
        //Set the metronome clock pulse to be the input pulse.
        *metronomeClockOut = midiClockTrigger;
    }

    //Work out changes at inputs and outputs.
    uint8_t noteOnInputChannels[maxNbInputs];
    uint8_t nbNoteOnInputChannels = 0;
    uint8_t noteOffInputChannels[maxNbInputs];
    uint8_t nbNoteOffInputChannels = 0;
    uint8_t noteOnInputChannelMask = 0;
    for(inputCounter = 0; inputCounter < maxNbInputs; inputCounter++)
    {
        const uint32_t gateIn = midiGateIns[inputCounter] >> 10;
        const uint8_t cachedOutputChannelId = data->mInputToOutputMap[inputCounter];

        if(gateIn && (UNUSED_ID == cachedOutputChannelId))
        {
            //Note-on event.
            noteOnInputChannelMask = MR_SET_CHANNEL_USED(noteOnInputChannelMask, inputCounter);
            noteOnInputChannels[nbNoteOnInputChannels] = inputCounter;
            nbNoteOnInputChannels++;
        }
        else if(!gateIn && (UNUSED_ID != cachedOutputChannelId))
        {
            //Note-off event.
            noteOffInputChannels[nbNoteOffInputChannels] = inputCounter;
            nbNoteOffInputChannels++;
        }
    }

    //If we are recording then process the changes at inputs as record events.
    if((eOperationalModeRecord == data->mOperationalMode) && (ePhaseActive == data->mPhase))
    {
        for(inputCounter = 0; inputCounter < nbNoteOnInputChannels; inputCounter++)
        {
            //Associate the input channel with an event if we have space for that.
            if(data->mNbEvents < maxNbEvents)
            {
                //Get the id of the input channel.
                const uint8_t inputChannelId = noteOnInputChannels[inputCounter];

                //Get the note-on information.
                const uint32_t noteNr = midiNoteIns[inputChannelId] >> 10;
                const uint32_t noteVel = midiVelIns[inputChannelId] >> 3;

                //Quantise to the nearest midi clock.
                const uint16_t timePassed = data->mTickCounter;
                const uint16_t timeRemaining = (data->mTickCounter > data->mNbTicksPerClock) ? 0 : (data->mNbTicksPerClock - data->mTickCounter);
                const uint16_t noteOnClock = (timePassed < timeRemaining) ? data->mMidiClockCount : data->mMidiClockCount + 1;

                //Cached the note-on event.
                data->mEvents[data->mNbEvents] = MR_PACK_EVENT(noteOnClock, 0, noteNr, noteVel);
                data->mInputRecordEventIds[inputChannelId] = data->mNbEvents;
                data->mNbEvents++;
            }
        }

        //If we just switched to record mode then record every input not picked up
        //as an input state change.
        if(prevPhase == ePhaseRecordCountIn)
        {
            for(inputCounter = 0; inputCounter < maxNbInputs; inputCounter++)
            {
                const uint32_t gateIn = midiGateIns[inputCounter] >> 10;
                const uint8_t channelIsAlreadyManaged = MR_IS_CHANNEL_USED(noteOnInputChannelMask, inputCounter);
                if((data->mNbEvents < maxNbEvents) && gateIn && !channelIsAlreadyManaged)
                {
                    //Get the note-on information.
                    const uint32_t noteNr = midiNoteIns[inputCounter] >> 10;
                    const uint32_t noteVel = midiVelIns[inputCounter] >> 3;

                    //Cached the note-on event.
                    //We know it is at clock 0.
                    data->mEvents[data->mNbEvents] = MR_PACK_EVENT(0, 0, noteNr, noteVel);
                    data->mInputRecordEventIds[inputCounter] = data->mNbEvents;
                    data->mNbEvents++;
                }
            }
        }

        for(inputCounter = 0; inputCounter < nbNoteOffInputChannels; inputCounter++)
        {
            //Get the id of the input channel.
            const uint8_t inputChannelId = noteOffInputChannels[inputCounter];

            //Get the id of the in-flight record event of the input channel.
            const uint8_t recordEventId = data->mInputRecordEventIds[inputChannelId];

            //If we had filled the event buffer at note-on clock then we will have
            //an invalid record event id.
            if(recordEventId != UNUSED_ID)
            {
                //Get the record event itself and the note-on clock.
                const uint32_t recordEvent = data->mEvents[recordEventId];
                const uint16_t noteOnClock = MR_GET_NOTE_ON_CLOCK(recordEvent);

                //Quantise the note-off clock to nearest clock.
                const uint16_t timePassed = data->mTickCounter;
                const uint16_t timeRemaining = (data->mTickCounter > data->mNbTicksPerClock) ? 0 : (data->mNbTicksPerClock - data->mTickCounter);
                const uint16_t noteOffClock = (timePassed < timeRemaining) ? data->mMidiClockCount : data->mMidiClockCount + 1;

                //Store the note-off clock and ensure a minimum 1 clock duration.
                const uint16_t deltaClock = (noteOffClock > noteOnClock) ? noteOffClock - noteOnClock : 1;
                const uint16_t minDurationNoteOffClock = noteOnClock + deltaClock;
                data->mEvents[recordEventId] = MR_INC_NOTE_OFF_CLOCK(recordEvent, minDurationNoteOffClock);

                //Free the input channel.
                data->mInputRecordEventIds[inputChannelId] = UNUSED_ID;
            }
        }
    }

    //Process all the live note-on events by mapping them to the output.
    //If we can't map them to an available output then find the oldest
    //playback output and mark it so we can stop it this tick and and
    //hopefully have a free output channel next tick.
    for(inputCounter = 0; inputCounter < nbNoteOnInputChannels; inputCounter++)
    {
        const uint32_t inputChannelId = noteOnInputChannels[inputCounter];

        //Find an available output.
        const uint8_t freeOutputChannelId = MR_LOWEST_FREE_BIT(data->mOutputChannelMask);

        if(UNUSED_ID != freeOutputChannelId)
        {
            //Mark the output as used.
            data->mOutputChannelMask = MR_SET_CHANNEL_USED(data->mOutputChannelMask, freeOutputChannelId);
            data->mInputToOutputMap[inputChannelId] = freeOutputChannelId;
        }
        else
        {
            //Find the oldest playback channel and mark it for stopping.
            //Find the oldest playback event and mark it to be stopped.
            //If there is a tie for oldest playback then choose the one with
            //the lowest note nr.
            //We'll catch the event next tick.
            uint16_t oldestNoteOnClockCount = data->mMidiClockCount;
            uint8_t oldestNoteOnNoteNr = midiNoteIns[inputChannelId] >> 10;
            uint8_t oldestOutputChannelId = UNUSED_ID;
            for(outputCounter = 0; outputCounter < maxNbOutputs; outputCounter++)
            {
                const uint8_t candidateEventId = data->mOutputPlaybackEventIds[outputCounter];
                if((UNUSED_ID != candidateEventId) && (MARKED_FOR_REMOVE != candidateEventId))
                {
                    const uint32_t candidateEvent = data->mEvents[candidateEventId];
                    const uint32_t candidateNoteOnClockCount = MR_QUANTIZE_ON(candidateEvent, quantisation);
                    const uint32_t candidateNoteNr = MR_GET_NOTE_NR(candidateEvent);
                    if(candidateNoteOnClockCount < oldestNoteOnClockCount)
                    {
                        oldestNoteOnClockCount = candidateNoteOnClockCount;
                        oldestOutputChannelId = outputCounter;
                        oldestNoteOnNoteNr = candidateNoteNr;
                    }
                    else if((candidateNoteOnClockCount == oldestNoteOnClockCount) && (candidateNoteNr < oldestNoteOnNoteNr))
                    {
                        oldestOutputChannelId = outputCounter;
                        oldestNoteOnNoteNr = candidateNoteNr;
                    }
                }
            }

            if(oldestOutputChannelId != UNUSED_ID)
            {
                //Mark the output for stopping
                //The output will be free next tick and we can pick it up then.
                data->mOutputPlaybackEventIds[oldestOutputChannelId] = MARKED_FOR_REMOVE;
            }
            else
            {
                //Something has gone badly wrong.
                //With 6 inputs and 6 outputs we should be able to find a free output
                //by stopping all playback events.
                //Do nothing?
                //Well, we cannot map the input to the output.
            }
        }
    }

    //Iterate over all open playback events and mark any that are due to stop on this midi clock.
    //Mark note numbers already playing but due to stop.
    uint32_t playbackToEndNoteMask[4];
    if(ePhaseActive == data->mPhase)
    {
        memset(playbackToEndNoteMask, 0, sizeof(playbackToEndNoteMask));

        for(outputCounter = 0; outputCounter < maxNbOutputs;  outputCounter++)
        {
            const uint32_t playbackEventId = data->mOutputPlaybackEventIds[outputCounter];
            if(UNUSED_ID != playbackEventId && MARKED_FOR_REMOVE != playbackEventId)
            {
                const uint32_t  playbackEvent = data->mEvents[playbackEventId];
                const uint16_t noteOffClockCount = MR_QUANTIZE_OFF(playbackEvent, quantisation);

                //If the note is due to end then mark it for remove.
                if((noteOffClockCount == data->mMidiClockCount) || rewindToStart)
                {
                    data->mOutputPlaybackEventIds[outputCounter] = MARKED_FOR_REMOVE;
                    const uint16_t noteOffNr = MR_GET_NOTE_NR(playbackEvent);
                    playbackToEndNoteMask[noteOffNr>>5] |= (1 << (noteOffNr&31));
                }
            }
        }
    }
    else if((ePhaseStopped == data->mPhase) && (ePhaseActive == prevPhase))
    {
        for(outputCounter = 0; outputCounter < maxNbOutputs;  outputCounter++)
        {
            const uint32_t playbackEventId = data->mOutputPlaybackEventIds[outputCounter];
            if(UNUSED_ID != playbackEventId && MARKED_FOR_REMOVE != playbackEventId)
            {
                data->mOutputPlaybackEventIds[outputCounter] = MARKED_FOR_REMOVE;
            }
        }
    }

    //If we are in active mode then process playback of events recorded in previous loops.
    //Route the note-on event to an output channel.
    //If we can't do that then mark an playback output channel for stopping.
    if((ePhaseActive == data->mPhase) && (data->mTide < data->mNbEventsPrev))
    {
        //Handle 1 note-on event.
        //Unpack the current event with some bit swizzling.
        const uint32_t event = data->mEvents[data->mTide];
        const uint16_t noteOnClockCount = MR_QUANTIZE_ON(event, quantisation);
        const uint8_t noteOnNoteNr = MR_GET_NOTE_NR(event);

        //If the event should start then try to start it.
        if(noteOnClockCount == data->mMidiClockCount)
        {
             //Are we already playing back this note?
             //Are we already playing back the note but it is scheduled to end?
             const uint32_t isNotePlayingAndDueToEnd = ((playbackToEndNoteMask[noteOnNoteNr>>5] >> (noteOnNoteNr&31)) & 1);

             //a) If !isNoteBeingPlayedBackAlready we just trigger the note.
             //b) If isNoteBeingPlayedBackAlready and isNoteMarkedForRemove then let the note
             //end and retrigger it next tick.
             //c) If isNoteBeingPlayedBackAlready and !isNoteMarkedForRemove we have a difficult
             //choice to make.  What should we do?
             //Should we set the end time of the note to be the largest end time?
             //     No way, that would defeat our non-destructive requirement.
            //Should we ignore the new note trigger?
             //     No way, that would defeat our non-destructive requirement.
            //Should we leave it for the user to merge the notes into one long note with a
            //gather/distribution block and some logic merging to avoid a re-trigger?
            //      Yes, let's pass this on to the user to decide what to do.
            //Summary: issue the new note unless (isNoteBeingPlayedBackAlready && isNoteMarkedForRemove)
             if(!isNotePlayingAndDueToEnd)
             {
                //Find an available output channel from the channels used by playback.
                const uint8_t freeOutputChannelId = MR_LOWEST_FREE_BIT(data->mOutputChannelMask);

                if(UNUSED_ID != freeOutputChannelId)
                {
                    data->mOutputPlaybackEventIds[freeOutputChannelId] = data->mTide;
                    data->mOutputChannelMask = MR_SET_CHANNEL_USED(data->mOutputChannelMask, freeOutputChannelId);
                    data->mTide++;
                }
                else
                {
                    //Find the oldest playback event and mark it to be stopped.
                    //If there is a tie for oldest playback then choose the one with
                    //the lowest note nr.
                    uint16_t oldestNoteOnClockCount = noteOnClockCount;
                    uint8_t oldestNoteOnNoteNr = noteOnNoteNr;
                    uint8_t oldestOutputChannelId = UNUSED_ID;
                    for(outputCounter = 0; outputCounter < maxNbOutputs; outputCounter++)
                    {
                        const uint8_t candidateEventId = data->mOutputPlaybackEventIds[outputCounter];
                        if((UNUSED_ID != candidateEventId) && (MARKED_FOR_REMOVE != candidateEventId))
                        {
                            const uint32_t candidateEvent = data->mEvents[candidateEventId];
                            const uint32_t candidateNoteOnClockCount = MR_QUANTIZE_ON(candidateEvent, quantisation);
                            const uint32_t candidateNoteNr = MR_GET_NOTE_NR(candidateEvent);
                            if(candidateNoteOnClockCount < oldestNoteOnClockCount)
                            {
                                oldestNoteOnClockCount = candidateNoteOnClockCount;
                                oldestOutputChannelId = outputCounter;
                                oldestNoteOnNoteNr = candidateNoteNr;
                            }
                            else if((candidateNoteOnClockCount == oldestNoteOnClockCount) && (candidateNoteNr < oldestNoteOnNoteNr))
                            {
                                oldestOutputChannelId = outputCounter;
                                oldestNoteOnNoteNr = candidateNoteNr;
                            }
                        }
                    }

                    if((oldestOutputChannelId != UNUSED_ID) && (oldestNoteOnClockCount < noteOnClockCount))
                    {
                        //We found a playback event that we can stop.
                        //Mark it and stop it later on.
                        data->mOutputPlaybackEventIds[oldestOutputChannelId] = MARKED_FOR_REMOVE;
                    }
                    else
                    {
                        //We have saturated the output with simultaneous events.
                        //We could stop the event with the lowest note number but that
                        //would result in an output of just 1 tick duration.
                        //We probably don't want that.
                        //Better to just ignore the event completely and drop it.
                        data->mTide++;
                    }
                }
            }
        }
    }

    //Iterate over all playback events that are due to stop and actually stop them.
    //Iterate over all active playback events and route them to the output.
    if(ePhaseActive == data->mPhase || ((ePhaseStopped == data->mPhase) && (ePhaseActive == prevPhase)))
    {
        for(outputCounter = 0; outputCounter < maxNbOutputs;  outputCounter++)
        {
            const uint32_t playbackEventId = data->mOutputPlaybackEventIds[outputCounter];
            if(MARKED_FOR_REMOVE == playbackEventId)
            {
                *midiGateOuts[outputCounter] = 0;
                data->mOutputChannelMask = MR_SET_CHANNEL_FREE(data->mOutputChannelMask, outputCounter);
                data->mOutputPlaybackEventIds[outputCounter] = UNUSED_ID;
            }
            else if(UNUSED_ID != playbackEventId)
            {
                const uint32_t playbackEvent = data->mEvents[playbackEventId];
                *midiGateOuts[outputCounter] = 1 << 10;
                *midiNoteOuts[outputCounter] = MR_GET_NOTE_NR(playbackEvent) << 10;
                *midiVelOuts[outputCounter] = MR_GET_VELOCITY(playbackEvent) << 3;
            }
        }
    }

    //Iterate over all live note-off events and clear the output.
    for(inputCounter = 0; inputCounter < nbNoteOffInputChannels; inputCounter++)
    {
        const uint8_t inputChannelId = noteOffInputChannels[inputCounter];
        const uint8_t outputChannelId = data->mInputToOutputMap[inputChannelId];

        data->mInputToOutputMap[inputChannelId] = UNUSED_ID;
        data->mOutputChannelMask = MR_SET_CHANNEL_FREE(data->mOutputChannelMask, outputChannelId);

        *midiGateOuts[outputChannelId] = 0;
    }

    //Iterate over all live note-on events and set the output.
    for(inputCounter = 0; inputCounter < nbNoteOnInputChannels; inputCounter++)
    {
        const uint8_t inputChannelId = noteOnInputChannels[inputCounter];
        const uint8_t outputChannelId = data->mInputToOutputMap[inputChannelId];
        *midiGateOuts[outputChannelId] = midiGateIns[inputChannelId];
        *midiNoteOuts[outputChannelId] = midiNoteIns[inputChannelId];
        *midiVelOuts[outputChannelId] = midiVelIns[inputChannelId];
    }

    //Increment the tick counter.
    data->mTickCounter++;

    //**********************************************
    //Patchblock code ends here
    //**********************************************
}

#endif // MIDIRECORDER2_H_INCLUDED
