#include "CommonTest.h"
#include "Blocks/MidiNoteRouter.h"
#include "MidiNoteRouterTest.h"

void testMidiNoteRouter()
{
    printf("MidiNoteRouter is %d bytes \n", sizeof(struct MidiNoteRouter));

    struct MidiNoteRouter midiNoteRouter;
    midiNoteRouter.mNoteNumbers[0] = 60 << 10;
    midiNoteRouter.mNoteNumbers[1] = 61 << 10;
    midiNoteRouter.mNoteNumbers[2] = 62 << 10;
    midiNoteRouter.mNoteNumbers[3] = 63 << 10;
    midiNoteRouter.mNoteNumbers[4] = 64 << 10;
    midiNoteRouter.mNoteNumbers[5] = 65 << 10;

    int32_t buffer[18];
    memset(buffer, 0, sizeof(buffer));
    midiNoteRouter.in0 = buffer + 0;
    midiNoteRouter.in1 = buffer + 1;
    midiNoteRouter.in2 = buffer + 2;
    midiNoteRouter.in3 = buffer + 3;
    midiNoteRouter.in4 = buffer + 4;
    midiNoteRouter.in5 = buffer + 5;
    midiNoteRouter.in6 = buffer + 6;
    midiNoteRouter.in7 = buffer + 7;
    midiNoteRouter.in8 = buffer + 8;
    midiNoteRouter.in9 = buffer + 9;
    midiNoteRouter.in10 = buffer + 10;
    midiNoteRouter.in11 = buffer + 11;
    midiNoteRouter.in12 = buffer + 12;
    midiNoteRouter.in13 = buffer + 13;
    midiNoteRouter.in14 = buffer + 14;
    midiNoteRouter.in15 = buffer + 15;
    midiNoteRouter.in16 = buffer + 16;
    midiNoteRouter.in17 = buffer + 17;

    buffer[0] = 1 << 10;
    buffer[1] = 61 << 10;
    buffer[2] = 100 << 10;

    tickMidiNoteRouter(&midiNoteRouter);
}
