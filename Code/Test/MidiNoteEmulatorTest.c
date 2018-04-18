#include "CommonTest.h"
#include "Blocks/MidiNoteEmulator.h"
#include "MidiNoteEmulatorTest.h"

void testMidiNoteEmulator()
{
    printf("MidiNoteEmulator is %d bytes \n", sizeof(struct MidiNoteEmulator));

    //No tests yet but at least test it compiles
    //and runs 1 tick without crashing.
    struct MidiNoteEmulator midiNoteEmulator;
    memset(&midiNoteEmulator, 0, sizeof(struct MidiNoteEmulator));
    int32_t buffer[2];
    midiNoteEmulator.in0 = buffer + 0;
    midiNoteEmulator.in1 = buffer + 1;
    tickMidiNoteEmulator(&midiNoteEmulator);
}
