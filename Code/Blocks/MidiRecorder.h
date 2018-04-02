#ifndef MIDIRECORDER_H_INCLUDED
#define MIDIRECORDER_H_INCLUDED

#include "Common/Common.h"

struct MidiRecorder
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

    int32_t* in18;   //midi clock pulse
    int32_t* in19;   //midi clock start

    int32_t* in20;   //operational mode
    int32_t* in21;   //num count-in clocks
    int32_t* in22;   //num record clocks
    int32_t* in23;   //quantisation

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

    uint8_t mEventNoteOnsDuringClock[12];                    //2 for each input
    uint8_t mEventNoteOffsDuringClock[12];                   //2 for each input
    uint16_t mEventNoteOnTickCountsDuringClock[12];          //2 for each input
    uint16_t mEventNoteOffTickCountsDuringClock[12];         //2 for each input
    uint8_t mOpenEvents[6];                                 //1 for each input/output
    uint32_t mEvents[32];                                   //Max num events we can record for a loop.
    uint8_t mNbEventNoteOnsDuringClock;                     //Num note-on events recorded since last clock.
    uint8_t mNbEventNoteOffsDuringClock;                    //Num note-ff events recorded since last clock.
    uint8_t mNbOpenEvents;                                  //Num note-on events being played back still to reach note-off time.
    uint8_t mNbEvents;                                      //Number of recorded events in the loop.
    uint8_t mMidiClockTriggerReady;                         //Flag set true on -ve pulse set false on +ve pulse.
    uint16_t mMidiClockCount;                               //Count of midi clocks.
    uint8_t mPhase;                                         //Stopped, waiting for start, record count-in, record, playback.
    uint8_t mTide;                                          //Playback event counter.
    uint16_t mTickCounter;                                  //Tick count since last clock.
};

void tickMidiRecorder(struct MidiRecorder* data)
{
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

    #define PACK_EVENT(on, off, note, vel)      ((on) | ((off) << 9) | ((note) << 18) | (vel) << 25)

    #define INC_NOTE_ON_CLOCK(e, inc)           ((e) + (inc))

    #define INC_NOTE_OFF_CLOCK(e, inc)          ((e) + ((inc) << 9))

    #define GET_NOTE_ON_CLOCK(e)                ((e) & ((1 << 9) - 1))

    #define GET_NOTE_OFF_CLOCK(e)               (((e) >> 9) & ((1 << 9) - 1))

    #define GET_NOTE_NR(e)                      (((e) >> 18) & ((1 << 7) - 1))

    #define GET_VELOCITY(e)                     (((e) >> 25) & ((1 << 7) - 1))

    #define QUANTIZE_ON(e, q)                   (((((e) & ((1 << 9) - 1)) + (q)/2)/(q))*(q))

    #define DURATION(e)                         ((((e) >> 9) & ((1 << 9) - 1)) -  ((e) & ((1 << 9) - 1)))

    #define QUANTIZE_OFF(e, q)                  (QUANTIZE_ON((e),(q)) + DURATION((e)))

    //****************************

    //*********************************
    //We will cycle through phases.
    //The trajectory through the phases depends on the operational mode.
    //ModePlay: ePhaseStopped -> ePhaseWaiting -> ePhasePlayback
    //ModeRecord: ePhaseStopped -> ePhaseWaiting -> ePhaseRecordCountIn -> ePhaseRecord -> ePhasePlayback
    //depending on the operational mode

    //Ideally, we'd use an enum here but I don't think that will work with patchblocks.
    static const uint8_t ePhaseStopped = 0;
    static const uint8_t ePhaseWaiting = 1;
    static const uint8_t ePhaseRecordCountIn = 2;
    static const uint8_t ePhaseRecord = 3;
    static const uint8_t ePhasePlayback = 4;

    //Ideally, we'd use an enum here but I don't think that will work with patchblocks.
    static const uint8_t eOperationalModePlay = 0;
    static const uint8_t eOperationalModeRecord = 1;
    //**********************************

    //Get the midi input data for all inputs.
    const int32_t midiGateIns[6] = {*data->in0, *data->in3, *data->in6, *data->in9, *data->in12, *data->in15};
    const int32_t midiNoteIns[6] = {*data->in1, *data->in4, *data->in7, *data->in10, *data->in13, *data->in16};
    const int32_t midiVelIns[6] = {*data->in2, *data->in5, *data->in8, *data->in11, *data->in14, *data->in17};

    //Store the output data for each output.
    int32_t* const midiGateOuts[6] = {&data->out0,&data->out3,&data->out6,&data->out9, &data->out12,&data->out15};
    int32_t* const midiNoteOuts[6] = {&data->out1,&data->out4,&data->out7,&data->out10, &data->out13,&data->out16};
    int32_t* const midiVelOuts[7] = {&data->out2,&data->out5,&data->out8,&data->out11, &data->out14,&data->out17};

    //Unpack the midi clock data.
    const int32_t midiClockTrigger = *data->in18;
    const int32_t midiStart = *data->in19;

    //Unpack the configuration data.
    const uint32_t operationalMode = (*data->in20) >> 10;
    const uint32_t maxNumCountInClocks = ((*data->in21) >> 10);
    const uint32_t maxNumLoopClocks = ((*data->in22) >> 10);
    const uint32_t quantisation = ((*data->in23) >> 10);

    //Compute the max size of all the buffers that we will use.
    const uint8_t maxNbInputs = sizeof(midiGateIns)/sizeof(int32_t);
    const uint8_t maxNbOutputs = sizeof(midiGateOuts)/sizeof(int32_t*);
    const uint8_t maxNbEvents = sizeof(data->mEvents) / sizeof(uint32_t);
    const uint8_t maxNbOpenEvents = sizeof(data->mOpenEvents) / sizeof(uint8_t);
    const uint8_t maxNbNoteOnsPerClock = sizeof(data->mEventNoteOnsDuringClock)/sizeof(uint8_t);
    const uint8_t maxNbNoteOffsPerClock = sizeof(data->mEventNoteOffsDuringClock)/sizeof(uint8_t);

    //Let's start the update code.
    //If midiStart is off then reset everything to zero and return.
    if (0 == midiStart)
    {
        //Record the old phase.
        const uint8_t prevPhase = data->mPhase;

        //Zero everything.
        data->mNbEventNoteOnsDuringClock = 0;
        data->mNbEventNoteOffsDuringClock = 0;
        data->mNbOpenEvents = 0;
        data->mNbEvents = (eOperationalModeRecord == operationalMode) ? 0 : data->mNbEvents;
        data->mMidiClockTriggerReady = 0;
        data->mMidiClockCount = 0;
        data->mPhase = ePhaseStopped;
        data->mTide = 0;
        data->mTickCounter = 0;

        //Set all open events to available.
        for(uint32_t i = 0; i < maxNbOpenEvents; i++)
        {
            data->mOpenEvents[i] = 0xff;
        }

        //Route input to output.
        for(uint32_t i = 0; i < maxNbInputs; i++)
        {
            *(midiGateOuts[i]) = midiGateIns[i];
            *(midiNoteOuts[i]) = midiNoteIns[i];
            *(midiVelOuts[i]) = midiVelIns[i];
        }

        //Zero all hanging notes from playback.
        if(ePhasePlayback == prevPhase)
        {
            for(uint32_t i = 0; i < maxNbInputs; i++)
            {
                *(midiGateOuts[i]) = 0;
            }
        }

        return;
    }

    //We must have started ie midiStart != 0
    //If we were in stopped phase then set phase to waiting and return.
    //Note: we are going to wait for the next clock pulse before either
    //playing or recording.
    //http://midi.teragonaudio.com/tech/midispec/seq.htm
    if (ePhaseStopped == data->mPhase)
    {
        data->mPhase = ePhaseWaiting;

        //Route input to output.
        for(uint32_t i = 0; i < maxNbInputs; i++)
        {
            *(midiGateOuts[i]) = midiGateIns[i];
            *(midiNoteOuts[i]) = midiNoteIns[i];
            *(midiVelOuts[i]) = midiVelIns[i];
        }

        return;
    }

    //Have we got the start of a clock pulse?
    //If so, increment phase and clock count.
    if ((midiClockTrigger> 0) && data->mMidiClockTriggerReady)
    {
       //Can't receive another pulse until clock trigger goes negative.
       data->mMidiClockTriggerReady = 0;

       if (eOperationalModePlay == operationalMode)
       {
          //Play mode.
          if (ePhaseWaiting == data->mPhase)
          {
             //We were waiting for a clock pulse and now we've got it.
             //Start playback.
             data->mPhase = ePhasePlayback;
          }
          else if (ePhasePlayback)
          {
             //We were playing back recorded events.
             //Increment clock count and continue playing back
             //recorded events.
             //If we've reached the end of the loop then
             //go back to the start of the loop.
             data->mTickCounter = 0;
             data->mMidiClockCount++;
             if (maxNumLoopClocks == data->mMidiClockCount)
             {
                data->mMidiClockCount = 0;
                data->mTide = 0;
             }
          }
       }
       else
       {
          //Record mode.
          if (ePhaseWaiting == data->mPhase)
          {
             //We were waiting for a clock pulse and now we've got it.
             //Start the count-in.
             data->mPhase = ePhaseRecordCountIn;
          }
          else if (ePhaseRecordCountIn == data->mPhase)
          {
             //We are in count-in mode.
             //Increment the clock count.
             //If we hit the end of the count-in then
             //start recording.
             data->mMidiClockCount++;
             if (maxNumCountInClocks == data->mMidiClockCount)
             {
                 //Set everything to zero.
                data->mMidiClockCount = 0;
                data->mTickCounter = 0;

                //Set the phase to record.
                data->mPhase = ePhaseRecord;

                //Set all events to zero before recording.
                for (uint32_t i = 0; i < maxNbEvents; i++)
                {
                   data->mEvents[i] = 0;
                }
             }
          }
          else if (ePhaseRecord == data->mPhase)
          {
              //const uint16_t tickCounterAtClock0 = 0;
              const uint16_t tickCounterAtClock1 = data->mTickCounter;

              //Iterate over all note-on events recorded since the previous clock
              //and decide if they have occurred at a tick count closer
              //to the previous clock or the current clock.
              for(uint32_t i = 0; i < data->mNbEventNoteOnsDuringClock;i++)
              {
                  const uint16_t tickCount = data->mEventNoteOnTickCountsDuringClock[i];
                  if((tickCounterAtClock1 - tickCount) < (tickCount - 0))
                  {
                      //We are closer to clock 1 than clock 0
                      //Add one on to the note-on clock count.
                      const uint8_t eventId = data->mEventNoteOnsDuringClock[i];
                      data->mEvents[eventId] = INC_NOTE_ON_CLOCK(data->mEvents[eventId], 1);
                  }
              }

              //Iterate over all note-off events recorded since the previous clock
              //and decide if they have occurred at a tick count closer
              //to the previous clock or the current clock.
              for(uint32_t i = 0; i < data->mNbEventNoteOffsDuringClock; i++)
              {
                  const uint8_t eventId = data->mEventNoteOffsDuringClock[i];
                  const uint16_t tickCount = data->mEventNoteOffTickCountsDuringClock[i];
                  if((tickCounterAtClock1 - tickCount) < (tickCount - 0))
                  {
                      data->mEvents[eventId] = INC_NOTE_OFF_CLOCK(data->mEvents[eventId], 1);
                  }

                  //Now ensure a minimum duration of 1 clock.
                  const uint32_t event = data->mEvents[eventId];
                  const uint32_t noteOffClockCount = GET_NOTE_OFF_CLOCK(event);
                  const uint32_t noteOnClockCount = GET_NOTE_ON_CLOCK(event);
                  if(noteOnClockCount == noteOffClockCount)
                  {
                      data->mEvents[eventId] = INC_NOTE_OFF_CLOCK(data->mEvents[eventId], 1);
                  }
              }

             //We are in record mode.
             //Increment the clock count.
             //If we hit the end of the loop then
             //stop recording and start playing back
             //what we recorded.
             data->mMidiClockCount++;
             if (maxNumLoopClocks == data->mMidiClockCount)
             {
                data->mMidiClockCount = 0;
                data->mPhase = ePhasePlayback;
             }

            data->mNbEventNoteOnsDuringClock = 0;
            data->mNbEventNoteOffsDuringClock = 0;
            data->mTickCounter=0;
          }
          else if (ePhasePlayback == data->mPhase)
          {
             //We are playing back what we recorded.
             //Increment the clock count.
             //If we hit the end of the loop then rewind
             //back to the start of the loop.
             data->mTickCounter = 0;
             data->mMidiClockCount++;
             if (maxNumLoopClocks == data->mMidiClockCount)
             {
                data->mMidiClockCount = 0;
                data->mTide = 0;
             }
          }
       }
    }
    else if (midiClockTrigger < 0)
    {
       //Trigger has gone negative, be ready to catch next crossing to positive.
       data->mMidiClockTriggerReady = 1;
    }

    //Process the phase.
    //We've already handled ePhaseStopped.
    //Still need to handle ePhaseWaiting, ePhaseRecordCountIn, ePhaseRecord, ePhasePlayback.
    //In ePhaseWaiting and ePhaseRecordCountIn we just need to route the input notes to the output.
    //In ePhaseRecord we want to record the input and route the input notes to the output.
    //In ePhasePlayback we want to route the recorded notes to the output as they are encountered.
    if ((ePhasePlayback == data->mPhase))
    {
        if(0 == data->mMidiClockCount && 0 == data->mTickCounter)
        {
            //Send note-off to all outputs for 1 tick
            //to ensure no hanging note-ons from previous
            //loop.
            for(uint32_t i = 0; i < maxNbOutputs; i++)
            {
                *(midiGateOuts[i]) = 0;
            }
        }
        else if(0 == data->mTickCounter)
        {
            //Handle all note off events.
            //Iterate over all open events and stop any that need stopped.
            for(uint32_t i = 0; i < maxNbOpenEvents; i++)
            {
                const uint8_t eventId = data->mOpenEvents[i];
                if(0xff != eventId)
                {
                    const uint32_t openEvent = data->mEvents[eventId];
                    const uint32_t openEventNoteOffClockCount = QUANTIZE_OFF(openEvent, quantisation);
                    if(openEventNoteOffClockCount == data->mMidiClockCount)
                    {
                        *(midiGateOuts[i]) = 0;
                        data->mOpenEvents[i] = 0xff;
                    }
                }
            }
        }
        else if (data->mTide < data->mNbEvents)
        {
            //Handle all note events.
            //Unpack the current event with some bit swizzling.
            const uint32_t event = data->mEvents[data->mTide];
            const uint32_t noteOnClockCount = QUANTIZE_ON(event, quantisation);
            const uint32_t midiNoteNumber = GET_NOTE_NR(event);
            const uint32_t midiVelocity = GET_VELOCITY(event);

            if(!event)
            {
                //Event has been nulled.
                //Ignore the event and move to the next.
                data->mTide++;
            }
            else if(noteOnClockCount == data->mMidiClockCount)
            {
                //We want to start an event.

                //Find an available open event.
                uint8_t availableOpenEventId = 0xff;
                for(uint32_t i = 0; i < maxNbOpenEvents; i++)
                {
                    if(0xff == data->mOpenEvents[i])
                    {
                        availableOpenEventId = i;
                        break;
                    }
                }

                if(0xff != availableOpenEventId)
                {
                    //We found an available output for a new event.
                    //Open the gate and send out the midi note.
                    //The values sent out will persist until we hit the note-off count.
                    *midiGateOuts[availableOpenEventId] = 1 << 10;
                    *midiNoteOuts[availableOpenEventId] = midiNoteNumber << 10;
                    *midiVelOuts[availableOpenEventId] = midiVelocity << 3;

                    data->mOpenEvents[availableOpenEventId] = data->mTide;
                    data->mTide++;
                }
                else
                {
                    //Are there any events older than the current event?
                    //Stop older events to make way for current event.
                    //Find the oldest open event.
                    uint8_t oldestOpenEventId = 0xff;
                    uint8_t oldestNoteOnClockCount = noteOnClockCount;
                    uint32_t oldestEvent = 0;
                    for(uint32_t i = 0; i < maxNbOpenEvents; i++)
                    {
                        const uint8_t candidateEventId = data->mOpenEvents[i];
                        const uint32_t candidateEvent = data->mEvents[candidateEventId];
                        const uint32_t candidateNoteOnClockCount = QUANTIZE_ON(candidateEvent, quantisation);
                        if(candidateNoteOnClockCount < oldestNoteOnClockCount)
                        {
                            oldestOpenEventId = i;
                            oldestNoteOnClockCount = candidateNoteOnClockCount;
                            oldestEvent = candidateEvent;
                        }
                        else if((oldestOpenEventId != 0xff) && (candidateNoteOnClockCount == oldestNoteOnClockCount))
                        {
                            const uint32_t otherNoteNumber = GET_NOTE_NR(candidateEvent);
                            const uint32_t oldestNoteNumber = GET_NOTE_NR(oldestEvent);
                            if(otherNoteNumber < oldestNoteNumber)
                            {
                                oldestOpenEventId = i;
                                oldestNoteOnClockCount = candidateNoteOnClockCount;
                                oldestEvent = candidateEvent;
                            }
                        }
                    }

                    if(oldestOpenEventId != 0xff)
                    {
                        //Stop the old event so we can start new event next tick.
                        *(midiGateOuts[oldestOpenEventId]) = 0;
                        data->mOpenEvents[oldestOpenEventId] = 0xff;
                    }
                    else
                    {
                        //We have saturated the output with simultaneous events.
                        //We could stop the event with the lowest note number but that
                        //would result in an output of just 1 tick duration.
                        //We probably don't want that.
                        //Better to just ignore the event completely and drop it.
                        data->mTide++;

                        /*
                        //Don't do this for the reasons given above.
                        //Find the lowest note of the simultaneous notes and stop it.
                        uint8_t lowestOpenEventId = 0;
                        const uint8_t lowestEventId = data->mOpenEvents[0];
                        uint8_t lowestNote = GET_NOTE_NR(data->mEvents[lowestEventId]);
                        for(uint32_t i = 1; i < maxNbOpenEvents; i++)
                        {
                            const uint8_t candidateEventId = data->mOpenEvents[i];
                            const uint8_t candidateNote = GET_NOTE_NR(data->mEvents[candidateEventId]);
                            if(candidateNote < lowestNote)
                            {
                                lowestOpenEventId = i;
                                lowestNote = candidateNote;
                            }
                        }

                        //Stop the old event so we can start new event next tick.
                        *(midiGateOuts[lowestOpenEventId]) = 0;
                        data->mOpenEvents[lowestOpenEventId] = 0xff;
                        */
                    }
                }
            }
        }

        //Increment the tick counter.
        data->mTickCounter++;
    }
    else if (ePhaseRecord == data->mPhase)
    {
        //Iterate over all the inputs.
        for(uint32_t i = 0; i < maxNbInputs; i++)
        {
            //Get the incoming midi information.
            const int32_t midiGateIn = midiGateIns[i];
            const int32_t midiNoteIn = midiNoteIns[i] >> 10;
            const int32_t midiVelocityIn = midiVelIns[i] >> 3;

            //There is one open event per input.
            const uint8_t eventId = data->mOpenEvents[i];
            const uint32_t event = (eventId != 0xff) ? data->mEvents[eventId] : 0;

            if(midiGateIn && !event)
            {
                //An event has started with a note-on.
                //Record it if we have enough memory.
                //Question: if we hit the limit maxNbNoteOnsPerClock we just ignore the notes turned on last but
                //should we instead attempt to drop the lowest notes?
                //Question: if we hit the limit maxNbNoteOnsPerClock we ignore note-ons but those note-ons might persist
                //to the next clock when we start the note-on count back at zero.  This means we don't actually drop
                //the note but we do end up quantising it to the wrong midi clock count. Is this a problem?
                //In playback mode we can only issue 6 note-ons per clock to the output so a lot of
                //notes will end up being ignored anyway.
                //Generating 12 note-ons in a single midi clock is very, very hard to do with human hands.
                if(data->mNbEvents < maxNbEvents && (data->mNbEventNoteOnsDuringClock < maxNbNoteOnsPerClock))
                {
                    data->mEventNoteOnsDuringClock[data->mNbEventNoteOnsDuringClock] = data->mNbEvents;
                    data->mEventNoteOnTickCountsDuringClock[data->mNbEventNoteOnsDuringClock] = data->mTickCounter;
                    data->mNbEventNoteOnsDuringClock++;

                    data->mOpenEvents[i] = data->mNbEvents;

                    data->mEvents[data->mNbEvents] = PACK_EVENT(data->mMidiClockCount, 0, midiNoteIn, midiVelocityIn);
                    data->mNbEvents++;
                }
            }
            else if((!midiGateIn || !midiVelocityIn) && event)
            {
                //An event has ended with a note-off.
                //Record the ending if we have enough memory.
                //Question: if we hit the limit maxNbNoteOffsPerClock we just drop the notes turned off last but
                //should we instead attempt to drop the lowest notes or the earliest note-ons or the latest note-ons?
                //Question: we null the event but we could just roll it over to the next midi clock like we do with note-ons.
                //This sounds dangerous because we might miss the note-off and then not know when to send the note-off and
                //leave a note hanging until the end of the playback loop.
                //Generating 12 note-offs in a single midi clock is very, very hard to do with human hands.
                if(data->mNbEventNoteOffsDuringClock < maxNbNoteOffsPerClock)
                {
                    data->mEventNoteOffsDuringClock[data->mNbEventNoteOffsDuringClock] = eventId;
                    data->mEventNoteOffTickCountsDuringClock[data->mNbEventNoteOffsDuringClock] = data->mTickCounter;
                    data->mNbEventNoteOffsDuringClock++;

                    data->mOpenEvents[i] = 0xff;

                    data->mEvents[eventId] = INC_NOTE_OFF_CLOCK(data->mEvents[eventId], data->mMidiClockCount);
                }
                else
                {
                    //Something bad has happened.
                    //Null the event.
                    data->mOpenEvents[i] = 0xff;
                    data->mEvents[eventId] = 0;
                }
            }
        }

        //Send the values to the output.
        for(uint32_t i = 0; i < maxNbInputs; i++)
        {
            *(midiGateOuts[i]) = midiGateIns[i];
            *(midiNoteOuts[i]) = midiNoteIns[i];
            *(midiVelOuts[i]) = midiVelIns[i];
        }

        //Increment tick counter.
        data->mTickCounter++;
    }
    else
    {
       //Send the values to the output.
        for(uint32_t i = 0; i < maxNbInputs; i++)
        {
            *(midiGateOuts[i]) = midiGateIns[i];
            *(midiNoteOuts[i]) = midiNoteIns[i];
            *(midiVelOuts[i]) = midiVelIns[i];
        }
    }
}

#endif // MIDIRECORDER_H_INCLUDED
