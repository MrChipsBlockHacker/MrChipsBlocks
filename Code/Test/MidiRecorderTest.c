#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "Blocks/MidiRecorder.h"
#include "MidiRecorderTest.h"
#include "CommonTest.h"

void testMacros()
{
    //Test packing and unpacking.
    {
        uint32_t e = MR_PACK_EVENT(5, 8, 60, 100);
        const uint32_t on = MR_GET_NOTE_ON_CLOCK(e);
        TEST_ASSERT(5 == on);
        const uint32_t off = MR_GET_NOTE_OFF_CLOCK(e);
        TEST_ASSERT(8 == off);
        const uint32_t note = MR_GET_NOTE_NR(e);
        TEST_ASSERT(60 == note);
        const uint32_t vel = MR_GET_VELOCITY(e);
        TEST_ASSERT(100 == vel);
    }

    //Test up to maximum of range.
    {
        uint32_t e = MR_PACK_EVENT(511, 511, 127, 127);
        const uint32_t on = MR_GET_NOTE_ON_CLOCK(e);
        TEST_ASSERT(511 == on);
        const uint32_t off = MR_GET_NOTE_OFF_CLOCK(e);
        TEST_ASSERT(511 == off);
        const uint32_t note = MR_GET_NOTE_NR(e);
        TEST_ASSERT(127 == note);
        const uint32_t vel = MR_GET_VELOCITY(e);
        TEST_ASSERT(127 == vel);
    }

    //Test minimum of range.
    {
        uint32_t e = MR_PACK_EVENT(0, 0, 0, 0);
        const uint32_t on = MR_GET_NOTE_ON_CLOCK(e);
        TEST_ASSERT(0 == on);
        const uint32_t off = MR_GET_NOTE_OFF_CLOCK(e);
        TEST_ASSERT(0 == off);
        const uint32_t note = MR_GET_NOTE_NR(e);
        TEST_ASSERT(0 == note);
        const uint32_t vel = MR_GET_VELOCITY(e);
        TEST_ASSERT(0 == vel);
    }

    //Test note-on increment.
    {
        uint32_t e = MR_PACK_EVENT(17, 35, 60, 100);
        e = MR_INC_NOTE_ON_CLOCK(e, 22);
        int32_t on = MR_GET_NOTE_ON_CLOCK(e);
        TEST_ASSERT(on == 39);
        e = MR_INC_NOTE_ON_CLOCK(e, 0);
        on = MR_GET_NOTE_ON_CLOCK(e);
        TEST_ASSERT(on == 39);
        e = MR_INC_NOTE_ON_CLOCK(e, -7);
        on = MR_GET_NOTE_ON_CLOCK(e);
        TEST_ASSERT(on == 32);
        TEST_ASSERT(MR_GET_NOTE_OFF_CLOCK(e) == 35);
        TEST_ASSERT(MR_GET_NOTE_NR(e) == 60);
        TEST_ASSERT(MR_GET_VELOCITY(e) == 100);
    }

    //Test note-on increment.
    {
        uint32_t e = MR_PACK_EVENT(17, 35, 60, 100);
        e = MR_INC_NOTE_OFF_CLOCK(e, 22);
        int32_t off = MR_GET_NOTE_OFF_CLOCK(e);
        TEST_ASSERT(off == 57);
        e = MR_INC_NOTE_OFF_CLOCK(e, 0);
        off = MR_GET_NOTE_OFF_CLOCK(e);
        TEST_ASSERT(off == 57);
        e = MR_INC_NOTE_OFF_CLOCK(e, -17);
        off = MR_GET_NOTE_OFF_CLOCK(e);
        TEST_ASSERT(off == 40);
        TEST_ASSERT(MR_GET_NOTE_ON_CLOCK(e) == 17);
        TEST_ASSERT(MR_GET_NOTE_NR(e) == 60);
        TEST_ASSERT(MR_GET_VELOCITY(e) == 100);
    }

    //Test quantisation on
    {
        TEST_ASSERT(MR_QUANTIZE_ON(MR_PACK_EVENT(42 + 3, 55, 60, 100), 6) == 48);
        TEST_ASSERT(MR_QUANTIZE_ON(MR_PACK_EVENT(42 + 4, 55, 60, 100), 6) == 48);
        TEST_ASSERT(MR_QUANTIZE_ON(MR_PACK_EVENT(42 + 5, 55, 60, 100), 6) == 48);
        TEST_ASSERT(MR_QUANTIZE_ON(MR_PACK_EVENT(48, 55, 60, 100), 6) == 48);
        TEST_ASSERT(MR_QUANTIZE_ON(MR_PACK_EVENT(48 + 1, 55, 60, 100), 6) == 48);
        TEST_ASSERT(MR_QUANTIZE_ON(MR_PACK_EVENT(48 + 2, 55, 60, 100), 6) == 48);
        TEST_ASSERT(MR_QUANTIZE_ON(MR_PACK_EVENT(48 + 3, 55, 60, 100), 6) == 54);
        TEST_ASSERT(MR_GET_NOTE_ON_CLOCK(MR_PACK_EVENT(48 + 3, 55, 60, 100)) == 51);
        TEST_ASSERT(MR_GET_NOTE_OFF_CLOCK(MR_PACK_EVENT(48 + 3, 55, 60, 100)) == 55);
        TEST_ASSERT(MR_GET_NOTE_NR(MR_PACK_EVENT(48 + 3, 55, 60, 100)) == 60);
        TEST_ASSERT(MR_GET_VELOCITY(MR_PACK_EVENT(48 + 3, 55, 60, 100)) == 100);
    }

    //Test quantisation off preserves duration.
    {
        TEST_ASSERT(MR_QUANTIZE_OFF(MR_PACK_EVENT(42 + 3, 55, 60, 100), 6) == 58);
        TEST_ASSERT(MR_QUANTIZE_OFF(MR_PACK_EVENT(42 + 4, 55, 60, 100), 6) == 57);
        TEST_ASSERT(MR_QUANTIZE_OFF(MR_PACK_EVENT(42 + 5, 55, 60, 100), 6) == 56);
        TEST_ASSERT(MR_QUANTIZE_OFF(MR_PACK_EVENT(48, 55, 60, 100), 6) == 55);
        TEST_ASSERT(MR_QUANTIZE_OFF(MR_PACK_EVENT(48 + 1, 55, 60, 100), 6) == 54);
        TEST_ASSERT(MR_QUANTIZE_OFF(MR_PACK_EVENT(48 + 2, 55, 60, 100), 6) == 53);
        TEST_ASSERT(MR_QUANTIZE_OFF(MR_PACK_EVENT(48 + 3, 55, 60, 100), 6) == 58);
        TEST_ASSERT(MR_GET_NOTE_ON_CLOCK(MR_PACK_EVENT(48 + 3, 55, 60, 100)) == 51);
        TEST_ASSERT(MR_GET_NOTE_OFF_CLOCK(MR_PACK_EVENT(48 + 3, 55, 60, 100)) == 55);
        TEST_ASSERT(MR_GET_NOTE_NR(MR_PACK_EVENT(48 + 3, 55, 60, 100)) == 60);
        TEST_ASSERT(MR_GET_VELOCITY(MR_PACK_EVENT(48 + 3, 55, 60, 100)) == 100);
    }
}

struct MidiNoteInputEvent
{
    int32_t mMidiNote;
    int32_t mMidiVelocity;
    uint32_t mTickCountOn;
    uint32_t mTickCountOff;
    uint32_t mInput;
};

struct MidiRecorderTestData
{
    //Schedule of events.
    struct MidiNoteInputEvent mNoteEvents[1024];
    uint8_t mNbNoteEvents;

    //How often to raise midi clock trigger.
    uint32_t mClockPulseTickPeriod;
    uint32_t mClockStartTick;
    uint32_t mClockStopTick;

    //Config data.
    int32_t mOperationalMode;
    int32_t mNbCountInClocks;
    int32_t mNbRecordClocks;
    int32_t mQuantisation;

    //Buffer for midi recorder int32 pointers.
    int32_t mBuffer[24];
};

struct MidiRecorderOutputStream
{
    uint8_t mPrevGate[6];
    uint8_t mPrevNote[6];
    uint8_t mPrevVelocity[6];

    uint32_t mPackedEvents[6][1024];
    uint32_t mNb[6];

    uint32_t mSortedEvents[6*1024];
    uint32_t mNbSortedEvents;
};

void initialiseTest(struct MidiRecorderTestData* test)
{
    test->mNbNoteEvents = 0;

    test->mClockPulseTickPeriod = 417;
    test->mClockStartTick = 912;
    test->mClockStopTick = 0xffffffff;

    test->mOperationalMode = 1;
    test->mNbCountInClocks = 96;
    test->mNbRecordClocks = 48;
    test->mQuantisation = 1;

     memset(test->mBuffer, 0, sizeof(int32_t)*24);
}

void initialiseStream(struct MidiRecorderOutputStream* stream)
{
    memset(stream, 0, sizeof(struct MidiRecorderOutputStream));
}


int cmpfunc (const void * _x, const void * _y)
{
    const uint32_t x = *(uint32_t*)_x;
    const uint32_t y = *(uint32_t*)_y;
    const uint32_t noteOnX = MR_GET_NOTE_ON_CLOCK(x);
    const uint32_t noteOnY = MR_GET_NOTE_ON_CLOCK(y);
    return (noteOnX - noteOnY);
}

void sortStream(struct MidiRecorderOutputStream* stream)
{
    //Store the per output events in a single array.
    for(uint32_t i = 0; i < 6; i++)
    {
        for(uint32_t j = 0; j < stream->mNb[i]; j++)
        {
            stream->mSortedEvents[stream->mNbSortedEvents] = stream->mPackedEvents[i][j];
            stream->mNbSortedEvents++;
        }
    }

    //Sort the output events chronologically.
    qsort((void*)stream->mSortedEvents, stream->mNbSortedEvents, sizeof(uint32_t), cmpfunc);
}

void initialiseMidiRecorder(struct MidiRecorderTestData* testData, struct MidiRecorder* midiRecorder)
{
    midiRecorder->mNbEventNoteOnsDuringClock = 0;
    midiRecorder->mNbEventNoteOffsDuringClock = 0;
    midiRecorder->mNbOpenEvents = 0;
    midiRecorder->mNbEvents = 0;
    midiRecorder->mMidiClockTriggerReady = 0;
    midiRecorder->mMidiClockCount = 0;
    midiRecorder->mPhase = 0;
    midiRecorder->mTide= 0;
    midiRecorder->mTickCounter = 0;
    midiRecorder->mNbEvents = 0;

    //Set the int32_t pointers to a buffer.
    midiRecorder->in0 = testData->mBuffer + 0;
    midiRecorder->in1 = testData->mBuffer + 1;
    midiRecorder->in2 = testData->mBuffer + 2;
    midiRecorder->in3 = testData->mBuffer + 3;
    midiRecorder->in4 = testData->mBuffer + 4;
    midiRecorder->in5 = testData->mBuffer + 5;
    midiRecorder->in6 = testData->mBuffer + 6;
    midiRecorder->in7 = testData->mBuffer + 7;
    midiRecorder->in8 = testData->mBuffer + 8;
    midiRecorder->in9 = testData->mBuffer + 9;
    midiRecorder->in10 = testData->mBuffer + 10;
    midiRecorder->in11 = testData->mBuffer + 11;
    midiRecorder->in12 = testData->mBuffer + 12;
    midiRecorder->in13 = testData->mBuffer + 13;
    midiRecorder->in14 = testData->mBuffer + 14;
    midiRecorder->in15 = testData->mBuffer + 15;
    midiRecorder->in16 = testData->mBuffer + 16;
    midiRecorder->in17 = testData->mBuffer + 17;
    midiRecorder->in18 = testData->mBuffer + 18;
    midiRecorder->in19 = testData->mBuffer + 19;
    midiRecorder->in20 = testData->mBuffer + 20;

    //Set the configuration data.
    *midiRecorder->in20 = testData->mOperationalMode << 10;
    midiRecorder->mNbCountInClocks = testData->mNbCountInClocks << 10;
    midiRecorder->mNbRecordClocks = testData->mNbRecordClocks << 10;
    midiRecorder->mQuantisation = testData->mQuantisation << 10;
}

void tickMidiRecorderTest
(const uint32_t tickCount,
 struct MidiRecorderTestData* testData, struct MidiRecorder* midiRecorder, struct MidiRecorderOutputStream* stream)
{
    //Set the configuration data.
    *midiRecorder->in20 = testData->mOperationalMode << 10;
    midiRecorder->mNbCountInClocks = testData->mNbCountInClocks << 10;
    midiRecorder->mNbRecordClocks = testData->mNbRecordClocks << 10;
    midiRecorder->mQuantisation = testData->mQuantisation << 10;

    //Set the clock trigger.
    if(0 == (tickCount % testData->mClockPulseTickPeriod))
    {
        *midiRecorder->in18 = toFX(0.5f, 10);
    }
    else
    {
        *midiRecorder->in18 = toFX(-0.5f, 10);
    }

    //Set midi start.
    if(tickCount >= testData->mClockStartTick && tickCount < testData->mClockStopTick)
    {
        *midiRecorder->in19 = 1 << 10;
    }
    else
    {
        *midiRecorder->in19 = 0;
    }

    //Start and stop notes.
    for(uint32_t i = 0; i < testData->mNbNoteEvents; i++)
    {
        struct MidiNoteInputEvent noteEvent = testData->mNoteEvents[i];
        if(tickCount == noteEvent.mTickCountOn)
        {
            if(0 == noteEvent.mInput)
            {
                *midiRecorder->in0 = 1 << 10;
                *midiRecorder->in1 = noteEvent.mMidiNote << 10;
                *midiRecorder->in2 = noteEvent.mMidiVelocity << 3;
            }
            else if(1 == noteEvent.mInput)
            {
                *midiRecorder->in3 = 1 << 10;
                *midiRecorder->in4 = noteEvent.mMidiNote << 10;
                *midiRecorder->in5 = noteEvent.mMidiVelocity << 3;
            }
            else if(2 == noteEvent.mInput)
            {
                *midiRecorder->in6 = 1 << 10;
                *midiRecorder->in7 = noteEvent.mMidiNote << 10;
                *midiRecorder->in8 = noteEvent.mMidiVelocity << 3;
            }
            else if(3 == noteEvent.mInput)
            {
                *midiRecorder->in9 = 1 << 10;
                *midiRecorder->in10 = noteEvent.mMidiNote << 10;
                *midiRecorder->in11 = noteEvent.mMidiVelocity << 3;
            }
            else if(4 == noteEvent.mInput)
            {
                *midiRecorder->in12 = 1 << 10;
                *midiRecorder->in13 = noteEvent.mMidiNote << 10;
                *midiRecorder->in14 = noteEvent.mMidiVelocity << 3;
            }
            else if(5 == noteEvent.mInput)
            {
                *midiRecorder->in15 = 1 << 10;
                *midiRecorder->in16 = noteEvent.mMidiNote << 10;
                *midiRecorder->in17 = noteEvent.mMidiVelocity << 3;
            }
        }
        else if(noteEvent.mTickCountOff == tickCount)
        {
            if(0 == noteEvent.mInput)
            {
                *midiRecorder->in0 = 0;
            }
            else if(1 == noteEvent.mInput)
            {
                *midiRecorder->in3 = 0;
            }
            else if(2 == noteEvent.mInput)
            {
                *midiRecorder->in6 = 0;
            }
            else if(3 == noteEvent.mInput)
            {
                *midiRecorder->in9 = 0;
            }
            else if(4 == noteEvent.mInput)
            {
                *midiRecorder->in12 = 0;
            }
            else if(5 == noteEvent.mInput)
            {
                *midiRecorder->in15 = 0;
            }
        }
    }

    tickMidiRecorder(midiRecorder);

    if(stream)
    {
        const uint8_t gateOuts[6] =
        {
            midiRecorder->out0 >> 10,
            midiRecorder->out3 >> 10,
            midiRecorder->out6 >> 10,
            midiRecorder->out9 >> 10,
            midiRecorder->out12 >> 10,
            midiRecorder->out15 >> 10
        };
        const uint8_t noteOuts[6] =
        {
            midiRecorder->out1 >> 10,
            midiRecorder->out4 >> 10,
            midiRecorder->out7 >> 10,
            midiRecorder->out10 >> 10,
            midiRecorder->out13 >> 10,
            midiRecorder->out16 >> 10
        };
        const uint8_t velOuts[6] =
        {
            midiRecorder->out2 >> 3,
            midiRecorder->out5 >> 3,
            midiRecorder->out8 >> 3,
            midiRecorder->out11 >> 3,
            midiRecorder->out14 >> 3,
            midiRecorder->out17 >> 3
        };

        for(uint32_t i = 0; i < 6; i++)
        {
            const uint8_t gateOut = gateOuts[i];
            const uint8_t noteOut = noteOuts[i];
            const uint8_t velOut = velOuts[i];
            const uint8_t prevGate = stream->mPrevGate[i];

            if(gateOut != prevGate)
            {
                if(gateOut)
                {
                    //Start.
                    const uint32_t nb = stream->mNb[i];
                    stream->mPackedEvents[i][nb] = MR_PACK_EVENT(midiRecorder->mMidiClockCount, 0, noteOut, velOut);
                }
                else
                {
                    //End.
                    const uint32_t nb = stream->mNb[i];
                    stream->mPackedEvents[i][nb] = MR_INC_NOTE_OFF_CLOCK(stream->mPackedEvents[i][nb], midiRecorder->mMidiClockCount);
                    stream->mNb[i]++;
                }
            }

            stream->mPrevGate[i] = gateOut;
            stream->mPrevNote[i] = noteOut;
            stream->mPrevVelocity[i] = velOut;
        }
    }
}

void testPhaseTranstionStoppedToWaiting()
{
    struct MidiRecorderTestData testData;
    initialiseTest(&testData);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&testData, &midiRecorder);

    //Phase should be zero before we enable midi start.
    const uint32_t tickStart = testData.mClockStartTick;
    for(uint32_t i = 0; i < tickStart; i++)
    {
        tickMidiRecorderTest(i, &testData, &midiRecorder, NULL);
    }
    TEST_ASSERT(0 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);

    //Phase should be 1 as soon as we enable midi start.
    for(uint32_t i = tickStart; i < tickStart + 1; i++)
    {
        tickMidiRecorderTest(i, &testData, &midiRecorder, NULL);
    }
    TEST_ASSERT(1 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);

    //Phase should be 1 after we enable midi start and before we do anything else.
    for(uint32_t i = tickStart + 1; i < tickStart + 2; i++)
    {
        tickMidiRecorderTest(i, &testData, &midiRecorder, NULL);
    }
    TEST_ASSERT(1 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);
}

void testPhaseTranstionWaitingToCountIn()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t tickStart = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;

    //Iterate to waiting.
    for(uint32_t i = 0; i < tickStart + 1; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(1 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);

    //Iterate until just before the next clock.
    uint32_t nextClockAfterStart = clockPeriod*((tickStart + clockPeriod)/clockPeriod);
    for(uint32_t i = tickStart + 1; i < nextClockAfterStart; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(1 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);

    //Iterate over next clock.
    for(uint32_t i = nextClockAfterStart; i < nextClockAfterStart + 1; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(2 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);
}

void testPhaseTranstionRecordCountInToRecord()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t tickStart = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    uint32_t nextClockAfterStart = clockPeriod*((tickStart + clockPeriod)/clockPeriod);

    //Iterate until just before next clock.
    for(uint32_t i = 0; i < nextClockAfterStart + clockPeriod; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(2 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);

    //Iterate over next clock.
    for(uint32_t i = nextClockAfterStart + clockPeriod; i < nextClockAfterStart + clockPeriod + 1; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(2 == midiRecorder.mPhase);
    TEST_ASSERT(1 == midiRecorder.mMidiClockCount);

    //Iterate until just before phase change.
    for(uint32_t i = nextClockAfterStart + clockPeriod + 1; i < nextClockAfterStart + test.mNbCountInClocks*clockPeriod; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(2 == midiRecorder.mPhase);
    TEST_ASSERT((test.mNbCountInClocks-1) == midiRecorder.mMidiClockCount);

    //Iterate over phase change to start recording.
    for(uint32_t i = (nextClockAfterStart + test.mNbCountInClocks*clockPeriod); i < (nextClockAfterStart + 1 + test.mNbCountInClocks*clockPeriod); i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);
}

void testPhaseTransitionRecordToPlayback()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t tickStart = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    uint32_t nextClockAfterStart = clockPeriod*((tickStart + clockPeriod)/clockPeriod);

    //Iterate over phase change to start recording.
    uint32_t start = 0;
    uint32_t stop = (nextClockAfterStart + 1 + test.mNbCountInClocks*clockPeriod);
    for(uint32_t i = 0; i < stop; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);

    //Iterate to just before 1st clock count.
    start = stop;
    stop = nextClockAfterStart + test.mNbCountInClocks*clockPeriod + clockPeriod;
    for(uint32_t i = start; i < stop; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);

    //Iterate over clock count.
    start = stop;
    stop = (nextClockAfterStart + test.mNbCountInClocks*clockPeriod + clockPeriod + 1);
    for(uint32_t i = start; i < stop; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(1 == midiRecorder.mMidiClockCount);

    //Iterate over the remaining clock counts until just before the last one.
    start = stop;
    stop = nextClockAfterStart + (test.mNbCountInClocks + test.mNbRecordClocks)*clockPeriod;
    for(uint32_t i = start; i < stop; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT((test.mNbRecordClocks-1) == midiRecorder.mMidiClockCount);

    start =  stop;
    stop = (nextClockAfterStart + (test.mNbCountInClocks + test.mNbRecordClocks)*clockPeriod) + 1;
    for(uint32_t i = start; i < stop; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);
}

void testPhaseTransitionPlaybackToPlayback()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t tickStart = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    uint32_t nextClockAfterStart = clockPeriod*((tickStart + clockPeriod)/clockPeriod);

    //Iterate to playback.
    uint32_t start =  0;
    uint32_t stop = (nextClockAfterStart + (test.mNbCountInClocks + test.mNbRecordClocks)*clockPeriod) + 1;
    for(uint32_t i = start; i < stop; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);

    //Iterate just before count.
    start = stop;
    stop = (nextClockAfterStart + (test.mNbCountInClocks + test.mNbRecordClocks + 1)*clockPeriod);
    for(uint32_t i = start; i < stop; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);

    //Iterate over count.
    start = stop;
    stop = (nextClockAfterStart + (test.mNbCountInClocks + test.mNbRecordClocks + 1)*clockPeriod + 1);
    for(uint32_t i = start; i < stop; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(1 == midiRecorder.mMidiClockCount);

    //Iterate to just before end of loop.
    start = stop;
    stop = (nextClockAfterStart + (test.mNbCountInClocks + 2*test.mNbRecordClocks)*clockPeriod);
    for(uint32_t i = start; i < stop; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT((test.mNbRecordClocks-1) == midiRecorder.mMidiClockCount);

    //Iterate over end of loop to start of loop.
    start = stop;
    stop = (nextClockAfterStart + (test.mNbCountInClocks + 2*test.mNbRecordClocks)*clockPeriod) + 1;
    for(uint32_t i = start; i < stop; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);
}

void testInputRoutesToOutputPhaseStopped()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    //Never start midi so that we remain in stopped state.
    test.mClockStartTick = 0xffffffff;

    struct MidiNoteInputEvent e0 = {60, 20, 1000, 5000, 0};
    struct MidiNoteInputEvent e1 = {61, 21, 2000, 6000, 1};
    struct MidiNoteInputEvent e2 = {62, 22, 3000, 7000, 2};
    struct MidiNoteInputEvent e3 = {63, 23, 4000, 8000, 3};
    struct MidiNoteInputEvent e4 = {64, 24, 5000, 9000, 4};
    struct MidiNoteInputEvent e5 = {65, 25, 6000, 10000, 5};

    test.mNoteEvents[0] = e0;
    test.mNoteEvents[1] = e1;
    test.mNoteEvents[2] = e2;
    test.mNoteEvents[3] = e3;
    test.mNoteEvents[4] = e4;
    test.mNoteEvents[5] = e5;
    test.mNbNoteEvents = 6;

    for(uint32_t i = 0; i < 1000; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(0 == midiRecorder.out0);
        TEST_ASSERT(0 == midiRecorder.out3);
        TEST_ASSERT(0 == midiRecorder.out6);
        TEST_ASSERT(0 == midiRecorder.out9);
        TEST_ASSERT(0 == midiRecorder.out12);
        TEST_ASSERT(0 == midiRecorder.out15);
    }

    for(uint32_t i = 1000; i < 2000; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(1<<10 == midiRecorder.out0);
        TEST_ASSERT(60<<10 == midiRecorder.out1);
        TEST_ASSERT(20<<3 == midiRecorder.out2);
        TEST_ASSERT(0 == midiRecorder.out3);
        TEST_ASSERT(0 == midiRecorder.out6);
        TEST_ASSERT(0 == midiRecorder.out9);
        TEST_ASSERT(0 == midiRecorder.out12);
        TEST_ASSERT(0 == midiRecorder.out15);
    }

    for(uint32_t i = 2000; i < 3000; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(1<<10 == midiRecorder.out0);
        TEST_ASSERT(60<<10 == midiRecorder.out1);
        TEST_ASSERT(20<<3 == midiRecorder.out2);
        TEST_ASSERT(1<<10 == midiRecorder.out3);
        TEST_ASSERT(61<<10 == midiRecorder.out4);
        TEST_ASSERT(21<<3 == midiRecorder.out5);
        TEST_ASSERT(0 == midiRecorder.out6);
        TEST_ASSERT(0 == midiRecorder.out9);
        TEST_ASSERT(0 == midiRecorder.out12);
        TEST_ASSERT(0 == midiRecorder.out15);
    }

    for(uint32_t i = 3000; i < 4000; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(1<<10 == midiRecorder.out0);
        TEST_ASSERT(60<<10 == midiRecorder.out1);
        TEST_ASSERT(20<<3 == midiRecorder.out2);
        TEST_ASSERT(1<<10 == midiRecorder.out3);
        TEST_ASSERT(61<<10 == midiRecorder.out4);
        TEST_ASSERT(21<<3 == midiRecorder.out5);
        TEST_ASSERT(1<<10 == midiRecorder.out6);
        TEST_ASSERT(62<<10 == midiRecorder.out7);
        TEST_ASSERT(22<<3 == midiRecorder.out8);
        TEST_ASSERT(0 == midiRecorder.out9);
        TEST_ASSERT(0 == midiRecorder.out12);
        TEST_ASSERT(0 == midiRecorder.out15);
    }

    for(uint32_t i = 4000; i < 5000; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(1<<10 == midiRecorder.out0);
        TEST_ASSERT(60<<10 == midiRecorder.out1);
        TEST_ASSERT(20<<3 == midiRecorder.out2);
        TEST_ASSERT(1<<10 == midiRecorder.out3);
        TEST_ASSERT(61<<10 == midiRecorder.out4);
        TEST_ASSERT(21<<3 == midiRecorder.out5);
        TEST_ASSERT(1<<10 == midiRecorder.out6);
        TEST_ASSERT(62<<10 == midiRecorder.out7);
        TEST_ASSERT(22<<3 == midiRecorder.out8);
        TEST_ASSERT(1<<10 == midiRecorder.out9);
        TEST_ASSERT(63<<10 == midiRecorder.out10);
        TEST_ASSERT(23<<3 == midiRecorder.out11);
        TEST_ASSERT(0 == midiRecorder.out12);
        TEST_ASSERT(0 == midiRecorder.out15);
    }

    for(uint32_t i = 5000; i < 6000; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(0 == midiRecorder.out0);
        TEST_ASSERT(1<<10 == midiRecorder.out3);
        TEST_ASSERT(61<<10 == midiRecorder.out4);
        TEST_ASSERT(21<<3 == midiRecorder.out5);
        TEST_ASSERT(1<<10 == midiRecorder.out6);
        TEST_ASSERT(62<<10 == midiRecorder.out7);
        TEST_ASSERT(22<<3 == midiRecorder.out8);
        TEST_ASSERT(1<<10 == midiRecorder.out9);
        TEST_ASSERT(63<<10 == midiRecorder.out10);
        TEST_ASSERT(23<<3 == midiRecorder.out11);
        TEST_ASSERT(1<<10 == midiRecorder.out12);
        TEST_ASSERT(64<<10 == midiRecorder.out13);
        TEST_ASSERT(24<<3 == midiRecorder.out14);
        TEST_ASSERT(0 == midiRecorder.out15);
    }

    for(uint32_t i = 6000; i < 7000; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(0 == midiRecorder.out0);
        TEST_ASSERT(0 == midiRecorder.out3);
        TEST_ASSERT(1<<10 == midiRecorder.out6);
        TEST_ASSERT(62<<10 == midiRecorder.out7);
        TEST_ASSERT(22<<3 == midiRecorder.out8);
        TEST_ASSERT(1<<10 == midiRecorder.out9);
        TEST_ASSERT(63<<10 == midiRecorder.out10);
        TEST_ASSERT(23<<3 == midiRecorder.out11);
        TEST_ASSERT(1<<10 == midiRecorder.out12);
        TEST_ASSERT(64<<10 == midiRecorder.out13);
        TEST_ASSERT(24<<3 == midiRecorder.out14);
        TEST_ASSERT(1<<10 == midiRecorder.out15);
        TEST_ASSERT(65<<10 == midiRecorder.out16);
        TEST_ASSERT(25<<3 == midiRecorder.out17);
    }

    for(uint32_t i = 7000; i < 8000; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(0 == midiRecorder.out0);
        TEST_ASSERT(0 == midiRecorder.out3);
        TEST_ASSERT(0 == midiRecorder.out6);
        TEST_ASSERT(1<<10 == midiRecorder.out9);
        TEST_ASSERT(63<<10 == midiRecorder.out10);
        TEST_ASSERT(23<<3 == midiRecorder.out11);
        TEST_ASSERT(1<<10 == midiRecorder.out12);
        TEST_ASSERT(64<<10 == midiRecorder.out13);
        TEST_ASSERT(24<<3 == midiRecorder.out14);
        TEST_ASSERT(1<<10 == midiRecorder.out15);
        TEST_ASSERT(65<<10 == midiRecorder.out16);
        TEST_ASSERT(25<<3 == midiRecorder.out17);
    }

    for(uint32_t i = 8000; i < 9000; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(0 == midiRecorder.out0);
        TEST_ASSERT(0 == midiRecorder.out3);
        TEST_ASSERT(0 == midiRecorder.out6);
        TEST_ASSERT(0 == midiRecorder.out9);
        TEST_ASSERT(1<<10 == midiRecorder.out12);
        TEST_ASSERT(64<<10 == midiRecorder.out13);
        TEST_ASSERT(24<<3 == midiRecorder.out14);
        TEST_ASSERT(1<<10 == midiRecorder.out15);
        TEST_ASSERT(65<<10 == midiRecorder.out16);
        TEST_ASSERT(25<<3 == midiRecorder.out17);
    }

    for(uint32_t i = 9000; i < 10000; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(0 == midiRecorder.out0);
        TEST_ASSERT(0 == midiRecorder.out3);
        TEST_ASSERT(0 == midiRecorder.out6);
        TEST_ASSERT(0 == midiRecorder.out9);
        TEST_ASSERT(0 == midiRecorder.out12);
        TEST_ASSERT(1<<10 == midiRecorder.out15);
        TEST_ASSERT(65<<10 == midiRecorder.out16);
        TEST_ASSERT(25<<3 == midiRecorder.out17);
    }

    for(uint32_t i = 10000; i < 11000; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(0 == midiRecorder.out0);
        TEST_ASSERT(0 == midiRecorder.out3);
        TEST_ASSERT(0 == midiRecorder.out6);
        TEST_ASSERT(0 == midiRecorder.out9);
        TEST_ASSERT(0 == midiRecorder.out12);
        TEST_ASSERT(0 == midiRecorder.out15);
    }
}

void testInputRoutesToOutputPhaseWaiting()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    //Never start midi so that we remain in stopped state.
    uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);

    //Issue a note on during stopped and a note off during waiting.
    struct MidiNoteInputEvent e0 = {60, 20, midiStartTick - 50, nextClockAfterStart -50, 5};
    test.mNoteEvents[0] = e0;
    test.mNbNoteEvents = 1;

    for(uint32_t i = 0; i < e0.mTickCountOn; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(0 == midiRecorder.out15);
        TEST_ASSERT(0 == midiRecorder.mPhase);
    }
    for(uint32_t i = e0.mTickCountOn; i < midiStartTick; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(1<<10 == midiRecorder.out15);
        TEST_ASSERT(0 == midiRecorder.mPhase);
    }
    for(uint32_t i = midiStartTick; i < midiStartTick+1; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(1<<10 == midiRecorder.out15);
        TEST_ASSERT(1 == midiRecorder.mPhase);
    }
    for(uint32_t i = midiStartTick+1; i < e0.mTickCountOff; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(1<<10 == midiRecorder.out15);
        TEST_ASSERT(1 == midiRecorder.mPhase);
    }
    for(uint32_t i = e0.mTickCountOff; i < e0.mTickCountOff+1; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        TEST_ASSERT(0 == midiRecorder.out15);
        TEST_ASSERT(1 == midiRecorder.mPhase);
    }
}

void testInputRoutesToOutputPhaseCountin()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);

    //Issue a note on during stopped and a note off during count-in.
    struct MidiNoteInputEvent e0 = {60, 20, midiStartTick - 50, nextClockAfterStart + 50, 3};
    test.mNoteEvents[0] = e0;
    test.mNbNoteEvents = 1;

    for(uint32_t i = 0; i < e0.mTickCountOn; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(0 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.out9);

    for(uint32_t i = e0.mTickCountOn; i < e0.mTickCountOn+1; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(0 == midiRecorder.mPhase);
    TEST_ASSERT(1<<10 == midiRecorder.out9);
    TEST_ASSERT(60<<10 == midiRecorder.out10);
    TEST_ASSERT(20<<3 == midiRecorder.out11);

    for(uint32_t i = e0.mTickCountOn+1; i < e0.mTickCountOff; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(2 == midiRecorder.mPhase);
    TEST_ASSERT(1<<10 == midiRecorder.out9);
    TEST_ASSERT(60<<10 == midiRecorder.out10);
    TEST_ASSERT(20<<3 == midiRecorder.out11);

    for(uint32_t i = e0.mTickCountOff; i < e0.mTickCountOff+1; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(2 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.out9);
}

void testInputRoutesToOutputPhaseRecord()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;

    //Issue a note on during stopped and a note off during record.
    struct MidiNoteInputEvent e0 = {60, 20, midiStartTick - 50, recordStart + 200, 3};
    test.mNoteEvents[0] = e0;
    //Issue a note on during record and a note off during record.
    struct MidiNoteInputEvent e1 = {61, 21, recordStart + 100, recordEnd - 5000, 4};
    test.mNoteEvents[1] = e1;
    test.mNbNoteEvents = 2;

    for(uint32_t i = 0; i < midiStartTick; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(0 == midiRecorder.mPhase);
    TEST_ASSERT(1 << 10 == midiRecorder.out9);
    TEST_ASSERT(60 << 10 == midiRecorder.out10);
    TEST_ASSERT(20 << 3 == midiRecorder.out11);
    TEST_ASSERT(0 << 10 == midiRecorder.out12);

    for(uint32_t i = midiStartTick; i < recordStart+101; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(1 << 10 == midiRecorder.out9);
    TEST_ASSERT(60 << 10 == midiRecorder.out10);
    TEST_ASSERT(20 << 3 == midiRecorder.out11);
    TEST_ASSERT(1 << 10 == midiRecorder.out12);
    TEST_ASSERT(61 << 10 == midiRecorder.out13);
    TEST_ASSERT(21 << 3 == midiRecorder.out14);

    for(uint32_t i = recordStart+101; i < recordStart+201; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(0 << 10 == midiRecorder.out9);
    TEST_ASSERT(1 << 10 == midiRecorder.out12);
    TEST_ASSERT(61 << 10 == midiRecorder.out13);
    TEST_ASSERT(21 << 3 == midiRecorder.out14);

    for(uint32_t i = recordStart+201; i < recordEnd-5000+1; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(0 << 10 == midiRecorder.out9);
    TEST_ASSERT(0 << 10 == midiRecorder.out12);
}

void testMidiClockQuantisation()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    const uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    const uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    const uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;

    //An on event early in midi clock count 15, off early in midi clock count 16
    {
        struct MidiNoteInputEvent e0 = {60, 20, 0, 0, 1};
        {
            const uint32_t onTick = recordStart + 15*test.mClockPulseTickPeriod + 4*(test.mClockPulseTickPeriod/10);
            const uint32_t offTick = recordStart + 16*test.mClockPulseTickPeriod + 4*(test.mClockPulseTickPeriod/10);
            e0.mTickCountOn = onTick;
            e0.mTickCountOff = offTick;
        }
        test.mNoteEvents[0] = e0;
        test.mNbNoteEvents = 1;

        //Iterate until we hit playback.
        for(uint32_t i = 0; i < (recordEnd + 1); i++)
        {
            tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        }
        TEST_ASSERT(4 == midiRecorder.mPhase);
        TEST_ASSERT(1 == midiRecorder.mNbEvents);
        TEST_ASSERT(15 == MR_GET_NOTE_ON_CLOCK(midiRecorder.mEvents[0]));
        TEST_ASSERT(16 == MR_GET_NOTE_OFF_CLOCK(midiRecorder.mEvents[0]));
    }

    //An on event late in midi clock count 15, off late in midi clock count 16
    {
        struct MidiNoteInputEvent e0 = {60, 20, 0, 0, 1};
        {
            const uint32_t onTick = recordStart + 15*test.mClockPulseTickPeriod + 6*(test.mClockPulseTickPeriod/10);
            const uint32_t offTick = recordStart + 16*test.mClockPulseTickPeriod + 6*(test.mClockPulseTickPeriod/10);
            e0.mTickCountOn = onTick;
            e0.mTickCountOff = offTick;
        }
        test.mNoteEvents[0] = e0;
        test.mNbNoteEvents = 1;

        //Iterate until we hit playback.
        for(uint32_t i = 0; i < (recordEnd + 1); i++)
        {
            tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        }
        TEST_ASSERT(4 == midiRecorder.mPhase);
        TEST_ASSERT(1 == midiRecorder.mNbEvents);
        TEST_ASSERT(16 == MR_GET_NOTE_ON_CLOCK(midiRecorder.mEvents[0]));
        TEST_ASSERT(17 == MR_GET_NOTE_OFF_CLOCK(midiRecorder.mEvents[0]));
    }

    //An on event early in midi clock count 15, off late in midi clock count 16
    {
        struct MidiNoteInputEvent e0 = {60, 20, 0, 0, 1};
        {
            const uint32_t onTick = recordStart + 15*test.mClockPulseTickPeriod + 4*(test.mClockPulseTickPeriod/10);
            const uint32_t offTick = recordStart + 16*test.mClockPulseTickPeriod + 6*(test.mClockPulseTickPeriod/10);
            e0.mTickCountOn = onTick;
            e0.mTickCountOff = offTick;
        }
        test.mNoteEvents[0] = e0;
        test.mNbNoteEvents = 1;

        //Iterate until we hit playback.
        for(uint32_t i = 0; i < (recordEnd + 1); i++)
        {
            tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        }
        TEST_ASSERT(4 == midiRecorder.mPhase);
        TEST_ASSERT(1 == midiRecorder.mNbEvents);
        TEST_ASSERT(15 == MR_GET_NOTE_ON_CLOCK(midiRecorder.mEvents[0]));
        TEST_ASSERT(17 == MR_GET_NOTE_OFF_CLOCK(midiRecorder.mEvents[0]));
    }

    //An on event late in midi clock count 15, off early in midi clock count 16
    //End should be quantised to 1 clock duration.
    {
        struct MidiNoteInputEvent e0 = {60, 20, 0, 0, 1};
        {
            const uint32_t onTick = recordStart + 15*test.mClockPulseTickPeriod + 6*(test.mClockPulseTickPeriod/10);
            const uint32_t offTick = recordStart + 16*test.mClockPulseTickPeriod + 4*(test.mClockPulseTickPeriod/10);
            e0.mTickCountOn = onTick;
            e0.mTickCountOff = offTick;
        }
        test.mNoteEvents[0] = e0;
        test.mNbNoteEvents = 1;

        //Iterate until we hit playback.
        for(uint32_t i = 0; i < (recordEnd + 1); i++)
        {
            tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
        }
        TEST_ASSERT(4 == midiRecorder.mPhase);
        TEST_ASSERT(1 == midiRecorder.mNbEvents);
        TEST_ASSERT(16 == MR_GET_NOTE_ON_CLOCK(midiRecorder.mEvents[0]));
        TEST_ASSERT(17 == MR_GET_NOTE_OFF_CLOCK(midiRecorder.mEvents[0]));
    }
}

void testOverlappingConsecutiveChords()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    const uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    const uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    const uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;

    //Three notes almost simultaneously on and almost simultaneously off.  Start at 12 and end at 21.
    struct MidiNoteInputEvent e0 = {61, 21, recordStart + 11*clockPeriod + 9*clockPeriod/10, recordStart + 21*clockPeriod - clockPeriod/9, 1};
    struct MidiNoteInputEvent e1 = {60, 20, recordStart + 12*clockPeriod + clockPeriod/10, recordStart + 21*clockPeriod + clockPeriod/10, 0};
    struct MidiNoteInputEvent e2 = {62, 22, recordStart + 12*clockPeriod + 9*clockPeriod/10, recordStart + 21*clockPeriod - 3*clockPeriod/8, 5};

    //Three notes almost simultaneously on and almost simultaneously off.  Start at 21 and end at 31.
    //Overlap start of these three with end of previous three.
    struct MidiNoteInputEvent e3 = {71, 31, recordStart + 20*clockPeriod + 9*clockPeriod/10, recordStart + 31*clockPeriod - clockPeriod/9, 4};
    struct MidiNoteInputEvent e4 = {72, 32, recordStart + 21*clockPeriod - clockPeriod/12, recordStart + 31*clockPeriod - 3*clockPeriod/8, 2};
    struct MidiNoteInputEvent e5 = {70, 30, recordStart + 21*clockPeriod + clockPeriod/10, recordStart + 31*clockPeriod + clockPeriod/10, 3};

    //All events.
    test.mNoteEvents[0] = e0;
    test.mNoteEvents[1] = e1;
    test.mNoteEvents[2] = e2;
    test.mNoteEvents[3] = e3;
    test.mNoteEvents[4] = e4;
    test.mNoteEvents[5] = e5;
    test.mNbNoteEvents = 6;

    //Iterate until we hit playback.
    for(uint32_t i = 0; i < (recordEnd + 1); i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(6 == midiRecorder.mNbEvents);
    TEST_ASSERT(midiRecorder.mEvents[0] == MR_PACK_EVENT(12, 21, 61, 21));
    TEST_ASSERT(midiRecorder.mEvents[1] == MR_PACK_EVENT(12, 21, 60, 20));
    TEST_ASSERT(midiRecorder.mEvents[2] == MR_PACK_EVENT(13, 21, 62, 22));
    TEST_ASSERT(midiRecorder.mEvents[3] == MR_PACK_EVENT(21, 31, 71, 31));
    TEST_ASSERT(midiRecorder.mEvents[4] == MR_PACK_EVENT(21, 31, 72, 32));
    TEST_ASSERT(midiRecorder.mEvents[5] == MR_PACK_EVENT(21, 31, 70, 30));
}

void testEventLimit()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    const uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    const uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    const uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;

    const uint32_t maxNbEvents = sizeof(midiRecorder.mEvents)/sizeof(uint32_t);
    const uint32_t maxNbTestEvents = sizeof(test.mNoteEvents)/sizeof(struct MidiNoteInputEvent);

    uint32_t eventCount = 0;
    uint32_t clockCount = 0;
    while(eventCount < 2*maxNbEvents)
    {
        //Issue notes of 1 clock duration.
        struct MidiNoteInputEvent e0 = {12, 50, recordStart + 2*clockCount*clockPeriod + clockPeriod/10, recordStart + 2*clockCount*clockPeriod + 9*clockPeriod/10, 0};
        struct MidiNoteInputEvent e1 = {13, 51, recordStart + 2*clockCount*clockPeriod + clockPeriod/10, recordStart + 2*clockCount*clockPeriod + 9*clockPeriod/10, 1};
        struct MidiNoteInputEvent e2 = {14, 52, recordStart + 2*clockCount*clockPeriod + clockPeriod/10, recordStart + 2*clockCount*clockPeriod + 9*clockPeriod/10, 2};
        struct MidiNoteInputEvent e3 = {15, 53, recordStart + 2*clockCount*clockPeriod + clockPeriod/10, recordStart + 2*clockCount*clockPeriod + 9*clockPeriod/10, 3};
        struct MidiNoteInputEvent e4 = {16, 54, recordStart + 2*clockCount*clockPeriod + clockPeriod/10, recordStart + 2*clockCount*clockPeriod + 9*clockPeriod/10, 4};
        struct MidiNoteInputEvent e5 = {17, 55, recordStart + 2*clockCount*clockPeriod + clockPeriod/10, recordStart + 2*clockCount*clockPeriod + 9*clockPeriod/10, 5};

        TEST_ASSERT(eventCount < maxNbTestEvents);
        test.mNoteEvents[eventCount] = e0;
        eventCount++;

        TEST_ASSERT(eventCount < maxNbTestEvents);
        test.mNoteEvents[eventCount] = e1;
        eventCount++;

        TEST_ASSERT(eventCount < maxNbTestEvents);
        test.mNoteEvents[eventCount] = e2;
        eventCount++;

        TEST_ASSERT(eventCount < maxNbTestEvents);
        test.mNoteEvents[eventCount] = e3;
        eventCount++;

        TEST_ASSERT(eventCount < maxNbTestEvents);
        test.mNoteEvents[eventCount] = e4;
        eventCount++;

        TEST_ASSERT(eventCount < maxNbTestEvents);
        test.mNoteEvents[eventCount] = e5;
        eventCount++;

        clockCount++;
    }
    test.mNbNoteEvents = 2*maxNbEvents;

    //Iterate until we hit playback.
    for(uint32_t i = 0; i < (recordEnd + 1); i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(maxNbEvents == midiRecorder.mNbEvents);

    TEST_ASSERT(MR_PACK_EVENT(0, 1, 12, 50) == midiRecorder.mEvents[0]);
    TEST_ASSERT(MR_PACK_EVENT(0, 1, 13, 51) == midiRecorder.mEvents[1]);
    TEST_ASSERT(MR_PACK_EVENT(0, 1, 14, 52) == midiRecorder.mEvents[2]);
    TEST_ASSERT(MR_PACK_EVENT(0, 1, 15, 53) == midiRecorder.mEvents[3]);
    TEST_ASSERT(MR_PACK_EVENT(0, 1, 16, 54) == midiRecorder.mEvents[4]);
    TEST_ASSERT(MR_PACK_EVENT(0, 1, 17, 55) == midiRecorder.mEvents[5]);

    TEST_ASSERT(MR_PACK_EVENT(2, 3, 12, 50) == midiRecorder.mEvents[6]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 13, 51) == midiRecorder.mEvents[7]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 14, 52) == midiRecorder.mEvents[8]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 15, 53) == midiRecorder.mEvents[9]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 16, 54) == midiRecorder.mEvents[10]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 17, 55) == midiRecorder.mEvents[11]);

    TEST_ASSERT(MR_PACK_EVENT(4, 5, 12, 50) == midiRecorder.mEvents[12]);
    TEST_ASSERT(MR_PACK_EVENT(4, 5, 13, 51) == midiRecorder.mEvents[13]);
    TEST_ASSERT(MR_PACK_EVENT(4, 5, 14, 52) == midiRecorder.mEvents[14]);
    TEST_ASSERT(MR_PACK_EVENT(4, 5, 15, 53) == midiRecorder.mEvents[15]);
    TEST_ASSERT(MR_PACK_EVENT(4, 5, 16, 54) == midiRecorder.mEvents[16]);
    TEST_ASSERT(MR_PACK_EVENT(4, 5, 17, 55) == midiRecorder.mEvents[17]);

    TEST_ASSERT(MR_PACK_EVENT(6, 7, 12, 50) == midiRecorder.mEvents[18]);
    TEST_ASSERT(MR_PACK_EVENT(6, 7, 13, 51) == midiRecorder.mEvents[19]);
    TEST_ASSERT(MR_PACK_EVENT(6, 7, 14, 52) == midiRecorder.mEvents[20]);
    TEST_ASSERT(MR_PACK_EVENT(6, 7, 15, 53) == midiRecorder.mEvents[21]);
    TEST_ASSERT(MR_PACK_EVENT(6, 7, 16, 54) == midiRecorder.mEvents[22]);
    TEST_ASSERT(MR_PACK_EVENT(6, 7, 17, 55) == midiRecorder.mEvents[23]);

    TEST_ASSERT(MR_PACK_EVENT(8, 9, 12, 50) == midiRecorder.mEvents[24]);
    TEST_ASSERT(MR_PACK_EVENT(8, 9, 13, 51) == midiRecorder.mEvents[25]);
    TEST_ASSERT(MR_PACK_EVENT(8, 9, 14, 52) == midiRecorder.mEvents[26]);
    TEST_ASSERT(MR_PACK_EVENT(8, 9, 15, 53) == midiRecorder.mEvents[27]);
    TEST_ASSERT(MR_PACK_EVENT(8, 9, 16, 54) == midiRecorder.mEvents[28]);
    TEST_ASSERT(MR_PACK_EVENT(8, 9, 17, 55) == midiRecorder.mEvents[29]);

    TEST_ASSERT(MR_PACK_EVENT(10, 11, 12, 50) == midiRecorder.mEvents[30]);
    TEST_ASSERT(MR_PACK_EVENT(10, 11, 13, 51) == midiRecorder.mEvents[31]);
}

void testNoteOnLimitPerClock()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    const uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    const uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    const uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;

    //Check our test is testing the correct limits.
    const uint32_t maxNbClockNoteOns = sizeof(midiRecorder.mEventNoteOnsDuringClock)/sizeof(uint8_t);
    TEST_ASSERT(12 == maxNbClockNoteOns);
    const uint32_t maxNbClockNoteOffs = sizeof(midiRecorder.mEventNoteOffsDuringClock)/sizeof(uint8_t);
    TEST_ASSERT(12 == maxNbClockNoteOffs);

    //Issue 18 ons in same clock, 12 offs in same clock, 6 offs at later clocks.
    //6 note-ons and rapid note-offs.
    struct MidiNoteInputEvent e0 = {12, 50, recordStart + 2*clockPeriod + 1, recordStart + 2*clockPeriod + 10, 0};
    struct MidiNoteInputEvent e1 = {13, 51, recordStart + 2*clockPeriod + 1, recordStart + 2*clockPeriod + 10, 1};
    struct MidiNoteInputEvent e2 = {14, 52, recordStart + 2*clockPeriod + 1, recordStart + 2*clockPeriod + 10, 2};
    struct MidiNoteInputEvent e3 = {15, 53, recordStart + 2*clockPeriod + 1, recordStart + 2*clockPeriod + 10, 3};
    struct MidiNoteInputEvent e4 = {16, 54, recordStart + 2*clockPeriod + 1, recordStart + 2*clockPeriod + 10, 4};
    struct MidiNoteInputEvent e5 = {17, 55, recordStart + 2*clockPeriod + 1, recordStart + 2*clockPeriod + 10, 5};
    //6 more note-ons and rapid note-offs.
    struct MidiNoteInputEvent e6 = {18, 56, recordStart + 2*clockPeriod + 20, recordStart + 2*clockPeriod + 30, 0};
    struct MidiNoteInputEvent e7 = {19, 57, recordStart + 2*clockPeriod + 20, recordStart + 2*clockPeriod + 30, 1};
    struct MidiNoteInputEvent e8 = {20, 58, recordStart + 2*clockPeriod + 20, recordStart + 2*clockPeriod + 30, 2};
    struct MidiNoteInputEvent e9 = {21, 59, recordStart + 2*clockPeriod + 20, recordStart + 2*clockPeriod + 30, 3};
    struct MidiNoteInputEvent e10 = {22, 60, recordStart + 2*clockPeriod + 20, recordStart + 2*clockPeriod + 30, 4};
    struct MidiNoteInputEvent e11 = {23, 61, recordStart + 2*clockPeriod + 20, recordStart + 2*clockPeriod + 30, 5};
    //Start 6 more straight away and end them in a later clock.
    //These cannot be recorded in clock 2 due to the limit being tested so they will slip to clock 3.
    struct MidiNoteInputEvent e12 = {24, 62, recordStart + 2*clockPeriod + 40, recordStart + (2+5)*clockPeriod + 10, 0};
    struct MidiNoteInputEvent e13 = {25, 63, recordStart + 2*clockPeriod + 40, recordStart + (2+6)*clockPeriod + 10, 1};
    struct MidiNoteInputEvent e14 = {26, 64, recordStart + 2*clockPeriod + 40, recordStart + (2+7)*clockPeriod + 10, 2};
    struct MidiNoteInputEvent e15 = {27, 65, recordStart + 2*clockPeriod + 40, recordStart + (2+8)*clockPeriod + 10, 3};
    struct MidiNoteInputEvent e16 = {28, 66, recordStart + 2*clockPeriod + 40, recordStart + (2+9)*clockPeriod + 10, 4};
    struct MidiNoteInputEvent e17 = {29, 67, recordStart + 2*clockPeriod + 40, recordStart + (2+10)*clockPeriod + 10, 5};
    //Issue 1 more note and check we can successfully record it after hitting the limit.
    struct MidiNoteInputEvent e18 = {30, 68, recordStart + (2+11)*clockPeriod + 10, recordStart + (2+14)*clockPeriod + 10, 5};

    test.mNoteEvents[0] = e0;
    test.mNoteEvents[1] = e1;
    test.mNoteEvents[2] = e2;
    test.mNoteEvents[3] = e3;
    test.mNoteEvents[4] = e4;
    test.mNoteEvents[5] = e5;
    test.mNoteEvents[6] = e6;
    test.mNoteEvents[7] = e7;
    test.mNoteEvents[8] = e8;
    test.mNoteEvents[9] = e9;
    test.mNoteEvents[10] = e10;
    test.mNoteEvents[11] = e11;
    test.mNoteEvents[12] = e12;
    test.mNoteEvents[13] = e13;
    test.mNoteEvents[14] = e14;
    test.mNoteEvents[15] = e15;
    test.mNoteEvents[16] = e16;
    test.mNoteEvents[17] = e17;
    test.mNoteEvents[18] = e18;
    test.mNbNoteEvents = 19;

    //Iterate until we hit playback.
    for(uint32_t i = 0; i < (recordEnd + 1); i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(19 == midiRecorder.mNbEvents);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 12, 50) == midiRecorder.mEvents[0]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 13, 51) == midiRecorder.mEvents[1]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 14, 52) == midiRecorder.mEvents[2]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 15, 53) == midiRecorder.mEvents[3]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 16, 54) == midiRecorder.mEvents[4]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 17, 55) == midiRecorder.mEvents[5]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 18, 56) == midiRecorder.mEvents[6]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 19, 57) == midiRecorder.mEvents[7]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 20, 58) == midiRecorder.mEvents[8]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 21, 59) == midiRecorder.mEvents[9]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 22, 60) == midiRecorder.mEvents[10]);
    TEST_ASSERT(MR_PACK_EVENT(2, 3, 23, 61) == midiRecorder.mEvents[11]);
    TEST_ASSERT(MR_PACK_EVENT(3, 7, 24, 62) == midiRecorder.mEvents[12]);
    TEST_ASSERT(MR_PACK_EVENT(3, 8, 25, 63) == midiRecorder.mEvents[13]);
    TEST_ASSERT(MR_PACK_EVENT(3, 9, 26, 64) == midiRecorder.mEvents[14]);
    TEST_ASSERT(MR_PACK_EVENT(3, 10, 27, 65) == midiRecorder.mEvents[15]);
    TEST_ASSERT(MR_PACK_EVENT(3, 11, 28, 66) == midiRecorder.mEvents[16]);
    TEST_ASSERT(MR_PACK_EVENT(3, 12, 29, 67) == midiRecorder.mEvents[17]);
    TEST_ASSERT(MR_PACK_EVENT(13, 16, 30, 68) == midiRecorder.mEvents[18]);
}

void testNoteOffLimitPerClock()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    const uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    const uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    const uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;

    //Check our test is testing the correct limits.
    const uint32_t maxNbClockNoteOffs = sizeof(midiRecorder.mEventNoteOffsDuringClock)/sizeof(uint8_t);
    TEST_ASSERT(12 == maxNbClockNoteOffs);

    //6 end in same clock.
    struct MidiNoteInputEvent e0 = {12, 50, recordStart + 19*clockPeriod + 1, recordStart + 20*clockPeriod + 10, 0};
    struct MidiNoteInputEvent e1 = {13, 51, recordStart + 19*clockPeriod + 1, recordStart + 20*clockPeriod + 10, 1};
    struct MidiNoteInputEvent e2 = {14, 52, recordStart + 19*clockPeriod + 1, recordStart + 20*clockPeriod + 10, 2};
    struct MidiNoteInputEvent e3 = {15, 53, recordStart + 19*clockPeriod + 1, recordStart + 20*clockPeriod + 10, 3};
    struct MidiNoteInputEvent e4 = {16, 54, recordStart + 19*clockPeriod + 1, recordStart + 20*clockPeriod + 10, 4};
    struct MidiNoteInputEvent e5 = {17, 55, recordStart + 19*clockPeriod + 1, recordStart + 20*clockPeriod + 10, 5};

    //6 more end in same clock.
    struct MidiNoteInputEvent e6 = {18, 56, recordStart + 20*clockPeriod + 20, recordStart + 20*clockPeriod + 100, 0};
    struct MidiNoteInputEvent e7 = {19, 57, recordStart + 20*clockPeriod + 20, recordStart + 20*clockPeriod + 100, 1};
    struct MidiNoteInputEvent e8 = {20, 58, recordStart + 20*clockPeriod + 20, recordStart + 20*clockPeriod + 100, 2};
    struct MidiNoteInputEvent e9 = {21, 59, recordStart + 20*clockPeriod + 20, recordStart + 20*clockPeriod + 100, 3};
    struct MidiNoteInputEvent e10 = {22, 60, recordStart + 20*clockPeriod + 20, recordStart + 20*clockPeriod + 100, 4};
    struct MidiNoteInputEvent e11 = {23, 61, recordStart + 20*clockPeriod + 20, recordStart + 20*clockPeriod + 100, 5};

    //6 more end in same clock.
    struct MidiNoteInputEvent e12 = {24, 62, recordStart + 20*clockPeriod + 120, recordStart + 20*clockPeriod + 200, 0};
    struct MidiNoteInputEvent e13 = {25, 63, recordStart + 20*clockPeriod + 120, recordStart + 20*clockPeriod + 200, 1};
    struct MidiNoteInputEvent e14 = {26, 64, recordStart + 20*clockPeriod + 120, recordStart + 20*clockPeriod + 200, 2};
    struct MidiNoteInputEvent e15 = {27, 65, recordStart + 20*clockPeriod + 120, recordStart + 20*clockPeriod + 200, 3};
    struct MidiNoteInputEvent e16 = {28, 66, recordStart + 20*clockPeriod + 120, recordStart + 20*clockPeriod + 200, 4};
    struct MidiNoteInputEvent e17 = {29, 67, recordStart + 20*clockPeriod + 120, recordStart + 20*clockPeriod + 200, 5};

    //Issue 1 more note and check we can successfully record it after dropping notes.
    struct MidiNoteInputEvent e18 = {30, 68, recordStart + 22*clockPeriod + 10, recordStart + 23*clockPeriod + 10, 5};

    test.mNoteEvents[0] = e0;
    test.mNoteEvents[1] = e1;
    test.mNoteEvents[2] = e2;
    test.mNoteEvents[3] = e3;
    test.mNoteEvents[4] = e4;
    test.mNoteEvents[5] = e5;
    test.mNoteEvents[6] = e6;
    test.mNoteEvents[7] = e7;
    test.mNoteEvents[8] = e8;
    test.mNoteEvents[9] = e9;
    test.mNoteEvents[10] = e10;
    test.mNoteEvents[11] = e11;
    test.mNoteEvents[12] = e12;
    test.mNoteEvents[13] = e13;
    test.mNoteEvents[14] = e14;
    test.mNoteEvents[15] = e15;
    test.mNoteEvents[16] = e16;
    test.mNoteEvents[17] = e17;
    test.mNoteEvents[18] = e18;
    test.mNbNoteEvents = 19;

    //Iterate until we hit playback.
    for(uint32_t i = 0; i < (recordEnd + 1); i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(19 == midiRecorder.mNbEvents);

    TEST_ASSERT(MR_PACK_EVENT(19, 20, 12, 50) == midiRecorder.mEvents[0]);
    TEST_ASSERT(MR_PACK_EVENT(19, 20, 13, 51) == midiRecorder.mEvents[1]);
    TEST_ASSERT(MR_PACK_EVENT(19, 20, 14, 52) == midiRecorder.mEvents[2]);
    TEST_ASSERT(MR_PACK_EVENT(19, 20, 15, 53) == midiRecorder.mEvents[3]);
    TEST_ASSERT(MR_PACK_EVENT(19, 20, 16, 54) == midiRecorder.mEvents[4]);
    TEST_ASSERT(MR_PACK_EVENT(19, 20, 17, 55) == midiRecorder.mEvents[5]);
    TEST_ASSERT(MR_PACK_EVENT(20, 21, 18, 56) == midiRecorder.mEvents[6]);
    TEST_ASSERT(MR_PACK_EVENT(20, 21, 19, 57) == midiRecorder.mEvents[7]);
    TEST_ASSERT(MR_PACK_EVENT(20, 21, 20, 58) == midiRecorder.mEvents[8]);
    TEST_ASSERT(MR_PACK_EVENT(20, 21, 21, 59) == midiRecorder.mEvents[9]);
    TEST_ASSERT(MR_PACK_EVENT(20, 21, 22, 60) == midiRecorder.mEvents[10]);
    TEST_ASSERT(MR_PACK_EVENT(20, 21, 23, 61) == midiRecorder.mEvents[11]);
    TEST_ASSERT(0 == midiRecorder.mEvents[12]);
    TEST_ASSERT(0 == midiRecorder.mEvents[13]);
    TEST_ASSERT(0 == midiRecorder.mEvents[14]);
    TEST_ASSERT(0 == midiRecorder.mEvents[15]);
    TEST_ASSERT(0 == midiRecorder.mEvents[16]);
    TEST_ASSERT(0 == midiRecorder.mEvents[17]);
    TEST_ASSERT(MR_PACK_EVENT(22, 23, 30, 68) == midiRecorder.mEvents[18]);
}

void testHangingNoteOnsInPlaybackMode()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    const uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    const uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    const uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;
    const uint32_t playbackEnd = recordEnd + test.mNbRecordClocks*clockPeriod;

    //A note that never ends.
    struct MidiNoteInputEvent e0 = {12, 50, recordStart + 1*clockPeriod + 1, recordStart + test.mNbRecordClocks*clockPeriod + 10, 0};
    test.mNoteEvents[0] = e0;
    test.mNbNoteEvents = 1;

    //Run through recording.
    //Should record a note-on but no note-off.

    //Record ends with note on.
    for(uint32_t i = 0; i < recordEnd ; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(1 == midiRecorder.mNbEvents);
    TEST_ASSERT(MR_GET_NOTE_ON_CLOCK(midiRecorder.mEvents[0]) == 1);
    TEST_ASSERT(MR_GET_NOTE_OFF_CLOCK(midiRecorder.mEvents[0]) == 0);
    TEST_ASSERT(1<<10 == midiRecorder.out0);

    //First frame of playback should close the hanging note.
    for(uint32_t i = recordEnd; i < (recordEnd + 1); i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(1 == midiRecorder.mNbEvents);
    TEST_ASSERT(MR_GET_NOTE_ON_CLOCK(midiRecorder.mEvents[0]) == 1);
    TEST_ASSERT(MR_GET_NOTE_OFF_CLOCK(midiRecorder.mEvents[0]) == 0);
    TEST_ASSERT(0 == midiRecorder.out0);

    //Run through playback.
    //Should end playback with hanging note.
    for(uint32_t i = recordEnd + 1; i < playbackEnd; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(1<<10 == midiRecorder.out0);
    TEST_ASSERT(12<<10 == midiRecorder.out1);
    TEST_ASSERT(50<<3 == midiRecorder.out2);

    //First tick of next playback should close the hanging note.
    for(uint32_t i = playbackEnd; i < playbackEnd+1; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(0 == midiRecorder.out0);
    TEST_ASSERT(12<<10 == midiRecorder.out1);
    TEST_ASSERT(50<<3 == midiRecorder.out2);
    TEST_ASSERT(0 == midiRecorder.mTide);
    TEST_ASSERT(0 == midiRecorder.mMidiClockCount);
}

void testHangingNoteOnsInStoppedMode()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    const uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    const uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    const uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;
    const uint32_t playbackEnd = recordEnd + test.mNbRecordClocks*clockPeriod;

    //A note that ends (but we'll stop before we get to the end).
    struct MidiNoteInputEvent e0 = {12, 50, recordStart + 1*clockPeriod + 1, recordStart + 20*clockPeriod + 10, 0};
    test.mNoteEvents[0] = e0;
    test.mNbNoteEvents = 1;

    //Run through recording.
    //Should record a note-on and a note-off.
    //Record ends with note on.
    for(uint32_t i = 0; i < recordEnd ; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(1 == midiRecorder.mNbEvents);
    TEST_ASSERT(MR_GET_NOTE_ON_CLOCK(midiRecorder.mEvents[0]) == 1);
    TEST_ASSERT(MR_GET_NOTE_OFF_CLOCK(midiRecorder.mEvents[0]) == 20);
    TEST_ASSERT(0<<10 == midiRecorder.out0);

    //Run through playback.
    //Should end playback with closed note.
    for(uint32_t i = recordEnd; i < playbackEnd; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(0 == midiRecorder.out0);
    TEST_ASSERT(12<<10 == midiRecorder.out1);
    TEST_ASSERT(50<<3 == midiRecorder.out2);

    //Now stop the midi clock at 5 clock periods so that issue a note-on but no note-off.
    for(uint32_t i = playbackEnd; i < playbackEnd + test.mClockPulseTickPeriod*5; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(1<<10 == midiRecorder.out0);
    TEST_ASSERT(12<<10 == midiRecorder.out1);
    TEST_ASSERT(50<<3 == midiRecorder.out2);

    //Stop the midi clock gate.
    test.mClockStopTick = playbackEnd + test.mClockPulseTickPeriod*5;
    for(uint32_t i = playbackEnd + test.mClockPulseTickPeriod*5; i < playbackEnd + test.mClockPulseTickPeriod*5 + 1; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(0 == midiRecorder.mPhase);
    TEST_ASSERT(0<<10 == midiRecorder.out0);
    TEST_ASSERT(12<<10 == midiRecorder.out1);
    TEST_ASSERT(50<<3 == midiRecorder.out2);
}

void testPlaybackQuantisationA()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    const uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    const uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    const uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;
    //const uint32_t playbackEnd = recordEnd + test.mNbRecordClocks*clockPeriod;

    struct MidiNoteInputEvent e0 = {12, 50, recordStart + 12*clockPeriod + 1, recordStart + 17*clockPeriod + 10, 0};
    struct MidiNoteInputEvent e1 = {13, 51, recordStart + 13*clockPeriod + 1, recordStart + 18*clockPeriod + 10, 1};
    struct MidiNoteInputEvent e2 = {14, 52, recordStart + 14*clockPeriod + 1, recordStart + 19*clockPeriod + 10, 2};
    struct MidiNoteInputEvent e3 = {15, 53, recordStart + 15*clockPeriod + 1, recordStart + 20*clockPeriod + 10, 3};
    struct MidiNoteInputEvent e4 = {16, 54, recordStart + 16*clockPeriod + 1, recordStart + 21*clockPeriod + 10, 4};
    struct MidiNoteInputEvent e5 = {17, 55, recordStart + 17*clockPeriod + 1, recordStart + 22*clockPeriod + 10, 5};

    test.mNoteEvents[0] = e0;
    test.mNoteEvents[1] = e1;
    test.mNoteEvents[2] = e2;
    test.mNoteEvents[3] = e3;
    test.mNoteEvents[4] = e4;
    test.mNoteEvents[5] = e5;
    test.mNbNoteEvents = 6;

    //Record all the events.
    for(uint32_t i = 0; i < recordEnd; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(6 == midiRecorder.mNbEvents);

    //What do we expect on playback for each quantisation?
    const uint32_t quantisations[8] = {1,2,3,4,6,8,12,16};
    const uint32_t expectedOns[8][6] =
    {
        {12,    13,     14,     15,     16,     17},
        {12,    14,     14,     16,     16,     18},
        {12,    12,     15,     15,     15,     18},
        {12,    12,     16,     16,     16,     16},
        {12,    12,     12,     18,     18,     18},
        {16,    16,     16,     16,     16,     16},
        {12,    12,     12,     12,     12,     12},
        {16,    16,     16,     16,     16,     16}
    };
    const uint32_t expectedDurations[8][6] =
    {
        {5,    5,     5,     5,     5,     5},
        {5,    5,     5,     5,     5,     5},
        {5,    5,     5,     5,     5,     5},
        {5,    5,     5,     5,     5,     5},
        {5,    5,     5,     5,     5,     5},
        {5,    5,     5,     5,     5,     5},
        {5,    5,     5,     5,     5,     5},
        {5,    5,     5,     5,     5,     5}
    };

    //Test each quantisation.
    for(uint32_t q = 0; q < 8; q++)
    {
        test.mQuantisation = quantisations[q];

        struct MidiRecorderOutputStream outputStream;
        initialiseStream(&outputStream);

        //Run through one playback.
        const uint32_t start = recordEnd + test.mNbRecordClocks*clockPeriod*q;
        const uint32_t stop =start + test.mNbRecordClocks*clockPeriod;
        for(uint32_t i = start; i < stop; i++)
        {
            tickMidiRecorderTest(i, &test, &midiRecorder, &outputStream);
        }
        TEST_ASSERT(4 == midiRecorder.mPhase);
        TEST_ASSERT(6 == midiRecorder.mNbEvents);
        TEST_ASSERT(47 == midiRecorder.mMidiClockCount);

        sortStream(&outputStream);
        TEST_ASSERT(6 == outputStream.mNbSortedEvents);

        for(int32_t i = 0; i < 6; i++)
        {
            const uint32_t onClock = MR_GET_NOTE_ON_CLOCK(outputStream.mSortedEvents[i]);
            const uint32_t offClock = MR_GET_NOTE_OFF_CLOCK(outputStream.mSortedEvents[i]);
            const uint32_t duration = offClock - onClock;
            TEST_ASSERT(onClock == expectedOns[q][i])
            TEST_ASSERT(duration == expectedDurations[q][i])
        }
    }
}

void testPlaybackQuantisationB()
{
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    const uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    const uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    const uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;
    //const uint32_t playbackEnd = recordEnd + test.mNbRecordClocks*clockPeriod;

    struct MidiNoteInputEvent e6 = {18, 56, recordStart + 18*clockPeriod + 1, recordStart + 23*clockPeriod + 10, 0};
    struct MidiNoteInputEvent e7 = {19, 57, recordStart + 19*clockPeriod + 1, recordStart + 24*clockPeriod + 10, 1};
    struct MidiNoteInputEvent e8 = {20, 58, recordStart + 20*clockPeriod + 1, recordStart + 25*clockPeriod + 10, 2};
    struct MidiNoteInputEvent e9 = {21, 59, recordStart + 21*clockPeriod + 1, recordStart + 26*clockPeriod + 10, 3};
    struct MidiNoteInputEvent e10 = {23, 60, recordStart + 22*clockPeriod + 1, recordStart + 27*clockPeriod + 10, 4};
    struct MidiNoteInputEvent e11 = {24, 61, recordStart + 23*clockPeriod + 1, recordStart + 28*clockPeriod + 10, 5};

    test.mNoteEvents[0] = e6;
    test.mNoteEvents[1] = e7;
    test.mNoteEvents[2] = e8;
    test.mNoteEvents[3] = e9;
    test.mNoteEvents[4] = e10;
    test.mNoteEvents[5] = e11;
    test.mNbNoteEvents = 6;

    //Record all the events.
    for(uint32_t i = 0; i < recordEnd; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(6 == midiRecorder.mNbEvents);

    //What do we expect on playback for each quantisation?
    const uint32_t quantisations[8] = {1,2,3,4,6,8,12,16};
    const uint32_t expectedOns[8][6] =
    {
        {18,     19,     20,     21,     22,     23},
        {18,     20,     20,     22,     22,     24},
        {18,     18,     21,     21,     21,     24},
        {20,     20,     20,     20,     24,     24},
        {18,     18,     18,     24,     24,     24},
        {16,     16,     24,     24,     24,     24},
        {24,     24,     24,     24,     24,     24},
        {16,     16,     16,     16,     16,     16}
    };
    const uint32_t expectedDurations[8][12] =
    {
        {5,     5,     5,     5,     5,     5},
        {5,     5,     5,     5,     5,     5},
        {5,     5,     5,     5,     5,     5},
        {5,     5,     5,     5,     5,     5},
        {5,     5,     5,     5,     5,     5},
        {5,     5,     5,     5,     5,     5},
        {5,     5,     5,     5,     5,     5},
        {5,     5,     5,     5,     5,     5}
    };

    //Test each quantisation.
    for(uint32_t q = 0; q < 8; q++)
    {
        test.mQuantisation = quantisations[q];

        struct MidiRecorderOutputStream outputStream;
        initialiseStream(&outputStream);

        //Run through one playback.
        const uint32_t start = recordEnd + test.mNbRecordClocks*clockPeriod*q;
        const uint32_t stop =start + test.mNbRecordClocks*clockPeriod;
        for(uint32_t i = start; i < stop; i++)
        {
            tickMidiRecorderTest(i, &test, &midiRecorder, &outputStream);
        }
        TEST_ASSERT(4 == midiRecorder.mPhase);
        TEST_ASSERT(6 == midiRecorder.mNbEvents);
        TEST_ASSERT(47 == midiRecorder.mMidiClockCount);

        sortStream(&outputStream);
        TEST_ASSERT(6 == outputStream.mNbSortedEvents);

        for(int32_t i = 0; i < 6; i++)
        {
            const uint32_t onClock = MR_GET_NOTE_ON_CLOCK(outputStream.mSortedEvents[i]);
            const uint32_t offClock = MR_GET_NOTE_OFF_CLOCK(outputStream.mSortedEvents[i]);
            const uint32_t duration = offClock - onClock;
            TEST_ASSERT(onClock == expectedOns[q][i])
            TEST_ASSERT(duration == expectedDurations[q][i])
        }
    }
}

void testOutputSaturationFromQuantisationA()
{
    //1 note on its own, then 6 notes that are played simultaneously after quantisation.
    //The 1st note should stop to make way for the 7th.
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    const uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    const uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    const uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;
    //const uint32_t playbackEnd = recordEnd + test.mNbRecordClocks*clockPeriod;

    struct MidiNoteInputEvent e0 = {10, 50, recordStart + 6*clockPeriod + 1, recordStart + 20*clockPeriod + 10, 0};
    struct MidiNoteInputEvent e1 = {20, 57, recordStart + 22*clockPeriod + 1, recordStart + 30*clockPeriod + 10, 1};
    struct MidiNoteInputEvent e2 = {21, 58, recordStart + 23*clockPeriod + 1, recordStart + 31*clockPeriod + 10, 2};
    struct MidiNoteInputEvent e3 = {22, 59, recordStart + 24*clockPeriod + 1, recordStart + 32*clockPeriod + 10, 3};
    struct MidiNoteInputEvent e4 = {23, 60, recordStart + 25*clockPeriod + 1, recordStart + 33*clockPeriod + 10, 4};
    struct MidiNoteInputEvent e5 = {24, 61, recordStart + 26*clockPeriod + 1, recordStart + 34*clockPeriod + 10, 5};
    struct MidiNoteInputEvent e6 = {25, 62, recordStart + 27*clockPeriod + 1, recordStart + 35*clockPeriod + 10, 0};
    test.mNoteEvents[0] = e0;
    test.mNoteEvents[1] = e1;
    test.mNoteEvents[2] = e2;
    test.mNoteEvents[3] = e3;
    test.mNoteEvents[4] = e4;
    test.mNoteEvents[5] = e5;
    test.mNoteEvents[6] = e6;
    test.mNbNoteEvents = 7;

    //Record all the events.
    for(uint32_t i = 0; i < recordEnd; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(7 == midiRecorder.mNbEvents);

    test.mQuantisation = 12;
    struct MidiRecorderOutputStream outputStream;
    initialiseStream(&outputStream);

    //Run through one playback.
    const uint32_t start = recordEnd + test.mNbRecordClocks*clockPeriod;
    const uint32_t stop =start + test.mNbRecordClocks*clockPeriod;
    for(uint32_t i = start; i < stop; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, &outputStream);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(7 == midiRecorder.mNbEvents);
    TEST_ASSERT(47 == midiRecorder.mMidiClockCount);

    sortStream(&outputStream);
    TEST_ASSERT(7 == outputStream.mNbSortedEvents);

    uint32_t expectedOns[7] = {12, 24, 24, 24, 24, 24, 24};
    uint32_t expectedDurations[7] = {12, 8, 8, 8, 8, 8, 8};

    for(int32_t i = 0; i < 7; i++)
    {
        const uint32_t onClock = MR_GET_NOTE_ON_CLOCK(outputStream.mSortedEvents[i]);
        const uint32_t offClock = MR_GET_NOTE_OFF_CLOCK(outputStream.mSortedEvents[i]);
        const uint32_t duration = offClock - onClock;
        TEST_ASSERT(onClock == expectedOns[i]);
        TEST_ASSERT(duration == expectedDurations[i]);
    }
}

void testOutputSaturationFromQuantisationB()
{
    //2 note on their own, then 5 notes that are played simultaneously after quantisation.
    //The lowest of the first 2 notes should stop to make way for the 7th.
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    const uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    const uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    const uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;
    //const uint32_t playbackEnd = recordEnd + test.mNbRecordClocks*clockPeriod;

    struct MidiNoteInputEvent e0 = {10, 50, recordStart + 6*clockPeriod + 1, recordStart + 21*clockPeriod + 10, 0};
    struct MidiNoteInputEvent e1 = {11, 50, recordStart + 6*clockPeriod + 1, recordStart + 21*clockPeriod + 10, 1};
    struct MidiNoteInputEvent e2 = {20, 57, recordStart + 22*clockPeriod + 1, recordStart + 30*clockPeriod + 10, 2};
    struct MidiNoteInputEvent e3 = {21, 58, recordStart + 23*clockPeriod + 1, recordStart + 31*clockPeriod + 10, 3};
    struct MidiNoteInputEvent e4 = {22, 59, recordStart + 24*clockPeriod + 1, recordStart + 32*clockPeriod + 10, 4};
    struct MidiNoteInputEvent e5 = {23, 60, recordStart + 25*clockPeriod + 1, recordStart + 33*clockPeriod + 10, 5};
    struct MidiNoteInputEvent e6 = {24, 61, recordStart + 26*clockPeriod + 1, recordStart + 34*clockPeriod + 10, 0};
    test.mNoteEvents[0] = e0;
    test.mNoteEvents[1] = e1;
    test.mNoteEvents[2] = e2;
    test.mNoteEvents[3] = e3;
    test.mNoteEvents[4] = e4;
    test.mNoteEvents[5] = e5;
    test.mNoteEvents[6] = e6;
    test.mNbNoteEvents = 7;

    //Record all the events.
    for(uint32_t i = 0; i < recordEnd; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(7 == midiRecorder.mNbEvents);

    test.mQuantisation = 12;
    struct MidiRecorderOutputStream outputStream;
    initialiseStream(&outputStream);

    //Run through one playback.
    const uint32_t start = recordEnd + test.mNbRecordClocks*clockPeriod;
    const uint32_t stop =start + test.mNbRecordClocks*clockPeriod;
    for(uint32_t i = start; i < stop; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, &outputStream);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(7 == midiRecorder.mNbEvents);
    TEST_ASSERT(47 == midiRecorder.mMidiClockCount);

    sortStream(&outputStream);
    TEST_ASSERT(7 == outputStream.mNbSortedEvents);

    uint32_t expectedOns[7] = {12, 12, 24, 24, 24, 24, 24};
    uint32_t expectedDurations[7] = {15, 12, 8, 8, 8, 8, 8};
    uint32_t expectedNoteNrs[7] ={11, 10, 20, 21, 22, 23, 24};

    for(int32_t i = 0; i < 7; i++)
    {
        const uint32_t onClock = MR_GET_NOTE_ON_CLOCK(outputStream.mSortedEvents[i]);
        const uint32_t offClock = MR_GET_NOTE_OFF_CLOCK(outputStream.mSortedEvents[i]);
        const uint32_t duration = offClock - onClock;
        const uint32_t noteNr = MR_GET_NOTE_NR(outputStream.mSortedEvents[i]);
        TEST_ASSERT(onClock == expectedOns[i]);
        TEST_ASSERT(duration == expectedDurations[i]);
        TEST_ASSERT(noteNr == expectedNoteNrs[i]);
    }
}

void testOutputSaturationFromQuantisationC()
{
    //7 notes that are played simultaneously after quantisation.
    //We will have to drop one of them.
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    const uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    const uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    const uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;
    //const uint32_t playbackEnd = recordEnd + test.mNbRecordClocks*clockPeriod;

    struct MidiNoteInputEvent e0 = {18, 50, recordStart + 20*clockPeriod + 1, recordStart + 25*clockPeriod + 10, 0};
    struct MidiNoteInputEvent e1 = {19, 51, recordStart + 21*clockPeriod + 1, recordStart + 26*clockPeriod + 10, 1};
    struct MidiNoteInputEvent e2 = {20, 52, recordStart + 22*clockPeriod + 1, recordStart + 27*clockPeriod + 10, 2};
    struct MidiNoteInputEvent e3 = {21, 53, recordStart + 23*clockPeriod + 1, recordStart + 28*clockPeriod + 10, 3};
    struct MidiNoteInputEvent e4 = {22, 54, recordStart + 24*clockPeriod + 1, recordStart + 29*clockPeriod + 10, 4};
    struct MidiNoteInputEvent e5 = {23, 55, recordStart + 25*clockPeriod + 1, recordStart + 30*clockPeriod + 10, 5};
    struct MidiNoteInputEvent e6 = {24, 56, recordStart + 26*clockPeriod + 1, recordStart + 31*clockPeriod + 10, 0};
    test.mNoteEvents[0] = e0;
    test.mNoteEvents[1] = e1;
    test.mNoteEvents[2] = e2;
    test.mNoteEvents[3] = e3;
    test.mNoteEvents[4] = e4;
    test.mNoteEvents[5] = e5;
    test.mNoteEvents[6] = e6;
    test.mNbNoteEvents = 7;

    //Record all the events.
    for(uint32_t i = 0; i < recordEnd; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(7 == midiRecorder.mNbEvents);

    test.mQuantisation = 12;
    struct MidiRecorderOutputStream outputStream;
    initialiseStream(&outputStream);

    //Run through one playback.
    const uint32_t start = recordEnd + test.mNbRecordClocks*clockPeriod;
    const uint32_t stop =start + test.mNbRecordClocks*clockPeriod;
    for(uint32_t i = start; i < stop; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, &outputStream);
    }
    TEST_ASSERT(4 == midiRecorder.mPhase);
    TEST_ASSERT(7 == midiRecorder.mNbEvents);
    TEST_ASSERT(47 == midiRecorder.mMidiClockCount);

    sortStream(&outputStream);
    TEST_ASSERT(6 == outputStream.mNbSortedEvents);
}

void testConsectutiveOnOffEvents()
{
    //7 notes that are played simultaneously after quantisation.
    //We will have to drop one of them.
    struct MidiRecorderTestData test;
    initialiseTest(&test);
    struct MidiRecorder midiRecorder;
    initialiseMidiRecorder(&test, &midiRecorder);

    const uint32_t midiStartTick = test.mClockStartTick;
    const uint32_t clockPeriod = test.mClockPulseTickPeriod;
    const uint32_t nextClockAfterStart = clockPeriod*((midiStartTick + clockPeriod)/clockPeriod);
    const uint32_t recordStart = nextClockAfterStart + test.mNbCountInClocks*clockPeriod;
    const uint32_t recordEnd = recordStart + test.mNbRecordClocks*clockPeriod;
    //const uint32_t playbackEnd = recordEnd + test.mNbRecordClocks*clockPeriod;

    struct MidiNoteInputEvent e0 = {18, 50, recordStart + 20*clockPeriod + 10, recordStart + 21*clockPeriod - 10, 0};
    struct MidiNoteInputEvent e1 = {19, 51, recordStart + 21*clockPeriod + 10, recordStart + 22*clockPeriod - 10, 1};
    struct MidiNoteInputEvent e2 = {20, 51, recordStart + 22*clockPeriod + 10, recordStart + 23*clockPeriod - 10, 2};

    test.mNoteEvents[0] = e0;
    test.mNoteEvents[1] = e1;
    test.mNoteEvents[2] = e2;
    test.mNbNoteEvents = 3;

    //Record all the events.
    for(uint32_t i = 0; i < recordEnd; i++)
    {
        tickMidiRecorderTest(i, &test, &midiRecorder, NULL);
    }
    TEST_ASSERT(3 == midiRecorder.mPhase);
    TEST_ASSERT(3 == midiRecorder.mNbEvents);

    {
        test.mQuantisation = 1;
        struct MidiRecorderOutputStream outputStream;
        initialiseStream(&outputStream);

        //Run through one playback.
        const uint32_t start = recordEnd + test.mNbRecordClocks*clockPeriod;
        const uint32_t stop =start + test.mNbRecordClocks*clockPeriod;
        for(uint32_t i = start; i < stop; i++)
        {
            tickMidiRecorderTest(i, &test, &midiRecorder, &outputStream);
        }
        TEST_ASSERT(4 == midiRecorder.mPhase);
        TEST_ASSERT(3 == midiRecorder.mNbEvents);
        TEST_ASSERT(47 == midiRecorder.mMidiClockCount);

        sortStream(&outputStream);
        TEST_ASSERT(3 == outputStream.mNbSortedEvents);
        TEST_ASSERT(MR_GET_NOTE_NR(outputStream.mSortedEvents[0])==18);
        TEST_ASSERT(MR_GET_NOTE_NR(outputStream.mSortedEvents[1])==19);
        TEST_ASSERT(MR_GET_NOTE_NR(outputStream.mSortedEvents[2])==20);
        TEST_ASSERT(MR_GET_NOTE_ON_CLOCK(outputStream.mSortedEvents[0])==20);
        TEST_ASSERT(MR_GET_NOTE_ON_CLOCK(outputStream.mSortedEvents[1])==21);
        TEST_ASSERT(MR_GET_NOTE_ON_CLOCK(outputStream.mSortedEvents[2])==22);
        TEST_ASSERT(MR_GET_NOTE_OFF_CLOCK(outputStream.mSortedEvents[0])==21);
        TEST_ASSERT(MR_GET_NOTE_OFF_CLOCK(outputStream.mSortedEvents[1])==22);
        TEST_ASSERT(MR_GET_NOTE_OFF_CLOCK(outputStream.mSortedEvents[2])==23);
        TEST_ASSERT(3 == outputStream.mNb[0]);
    }

    {
        test.mQuantisation = 8;
        struct MidiRecorderOutputStream outputStream;
        initialiseStream(&outputStream);

        //Run through one playback.
        const uint32_t start = recordEnd + test.mNbRecordClocks*clockPeriod;
        const uint32_t stop =start + test.mNbRecordClocks*clockPeriod;
        for(uint32_t i = start; i < stop; i++)
        {
            tickMidiRecorderTest(i, &test, &midiRecorder, &outputStream);
        }
        TEST_ASSERT(4 == midiRecorder.mPhase);
        TEST_ASSERT(3 == midiRecorder.mNbEvents);
        TEST_ASSERT(47 == midiRecorder.mMidiClockCount);

        sortStream(&outputStream);
        TEST_ASSERT(3 == outputStream.mNbSortedEvents);
        TEST_ASSERT(MR_GET_NOTE_ON_CLOCK(outputStream.mSortedEvents[0])==24);
        TEST_ASSERT(MR_GET_NOTE_ON_CLOCK(outputStream.mSortedEvents[1])==24);
        TEST_ASSERT(MR_GET_NOTE_ON_CLOCK(outputStream.mSortedEvents[2])==24);
        TEST_ASSERT(MR_GET_NOTE_OFF_CLOCK(outputStream.mSortedEvents[0])==25);
        TEST_ASSERT(MR_GET_NOTE_OFF_CLOCK(outputStream.mSortedEvents[1])==25);
        TEST_ASSERT(MR_GET_NOTE_OFF_CLOCK(outputStream.mSortedEvents[2])==25);
        TEST_ASSERT(1 == outputStream.mNb[0]);
        TEST_ASSERT(1 == outputStream.mNb[1]);
        TEST_ASSERT(1 == outputStream.mNb[2]);
    }
}

void testMidiRecorder()
{
    printf("MidiRecorder is %d bytes \n", sizeof(struct MidiRecorder));

    testMacros();

    //Test phase transitions based on
    //clock triggers and clock count thresholds.
    testPhaseTranstionStoppedToWaiting();
    testPhaseTranstionWaitingToCountIn();
    testPhaseTranstionRecordCountInToRecord();
    testPhaseTransitionRecordToPlayback();
    testPhaseTransitionPlaybackToPlayback();

    //Phases stopped, waiting, countin and record all
    //route input to output. Test this works.
    testInputRoutesToOutputPhaseStopped();
    testInputRoutesToOutputPhaseWaiting();
    testInputRoutesToOutputPhaseCountin();
    testInputRoutesToOutputPhaseRecord();

    //Test that we record notes to the correct clock quantisation.
    testMidiClockQuantisation();

    //Test we can record two overlapping but consecutive 3-note chords.
    testOverlappingConsecutiveChords();

    //Test we don't crash if we record more than 32 events in a loop.
    testEventLimit();

    //Test we don't crash if we issue more than N note-ons and note-offs per frame.
    testNoteOnLimitPerClock();
    testNoteOffLimitPerClock();

    //Test hanging ons are cleared on next loop in playback mode.
    testHangingNoteOnsInPlaybackMode();
    //Test hanging ons are cleared in stopped mode.
    testHangingNoteOnsInStoppedMode();

    //Test quantisation on playback.
    testPlaybackQuantisationA();
    testPlaybackQuantisationB();

    //Test when quantisation creates more simultaneous ons than allowed and notes should be dropped.
    testOutputSaturationFromQuantisationA();
    testOutputSaturationFromQuantisationB();
    testOutputSaturationFromQuantisationC();

    //Test we can trigger consecutive midi clocks.
    // ie on and off in clock N, on and off in clock N + 1
    testConsectutiveOnOffEvents();

    //Test note-ons and note-offs in last midi clock of loop ie
    //Test that note-ons quantised to end of loop are lost.
}


