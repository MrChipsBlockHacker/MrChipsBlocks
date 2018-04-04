#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "Blocks/MidiClockEmulator.h"
#include "MidiClockEmulatorTest.h"
#include "CommonTest.h"

void testBPM(const float beatsPerMinute, const float testTimeInSeconds, const float width)
{
    //Compute the number of clocks expected in N seconds at a specified bpm.
    const float clocksPerSecond = 24*beatsPerMinute/60.0f;
    const uint32_t nbClocksInTestTime = (uint32_t)(testTimeInSeconds*clocksPerSecond);
    const uint32_t testTimeInTicks = SMP_RATE*testTimeInSeconds;

    struct MidiClockEmulator midiClockEmulator;
    midiClockEmulator.mPhase = 0;
    midiClockEmulator.mBPM = toFX(beatsPerMinute, 10);
    midiClockEmulator.mClockPulseWidth = toFX(width, 10);

    //Tick until we get the first clock pulse.
    uint32_t firstOnTime = 0xffffffff;
    uint8_t wasNegative = 1;
    uint32_t count = 0;
    while(0xffffffff == firstOnTime)
    {
        tickMidiClockEmulator(&midiClockEmulator);

        if(midiClockEmulator.out0 > 0)
        {
            if(wasNegative)
            {
                wasNegative = 0;

                if(0xffffffff == firstOnTime)
                {
                    firstOnTime = count;
                }
            }
        }
        else
        {
            wasNegative = 1;
        }

        count++;
    }

    //Tick from time of first clock until end of test.
    uint32_t numOffTicks = 0;
    uint32_t numOnTicks = 0;
    uint32_t numClocks = 1;
    for(uint32_t i = firstOnTime; i < firstOnTime + testTimeInTicks; i++)
    {
        tickMidiClockEmulator(&midiClockEmulator);

        if(midiClockEmulator.out0 > 0)
        {
            if(wasNegative)
            {
                wasNegative = 0;
                numClocks++;
            }
            numOnTicks++;
        }
        else
        {
            wasNegative = 1;
            numOffTicks++;
        }
    }

    const float ratio = (float)numOnTicks/(float)(numOnTicks + numOffTicks);
    TEST_ASSERT(fabsf(ratio - width) < 0.001f);

    TEST_ASSERT(numClocks == nbClocksInTestTime);
}

void testMidiClockEmulator()
{
    printf("MidiClockEmulator is %d bytes \n", sizeof(struct MidiClockEmulator));

    testBPM(120.0f, 2.0f, 0.1f);
    testBPM(180.0f, 2.0f, 0.075f);
    testBPM(240.0f, 2.0f, 0.05f);
}
