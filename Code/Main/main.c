#include "Test/MidiClockEmulatorTest.h"
#include "Test/MidiRecorderTest.h"
#include "Test/MidiNoteEmulatorTest.h"
#include "Test/MidiNoteRouterTest.h"

int main()
{
    testMidiClockEmulator();

    testMidiRecorder();

    testMidiNoteEmulator();

    testMidiNoteRouter();

    return 0;
}
