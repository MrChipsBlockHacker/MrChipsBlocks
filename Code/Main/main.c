#include "Test/MidiClockEmulatorTest.h"
#include "Test/MidiRecorderTest.h"
#include "Test/MidiNoteEmulatorTest.h"

int main()
{
    testMidiClockEmulator();

    testMidiRecorder();

    testMidiNoteEmulator();

    return 0;
}
