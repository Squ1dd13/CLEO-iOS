#include "Custom/Android.hpp"
#include "Game/Memory.hpp"
#include "Game/Touch.hpp"
#include <unordered_map>

template <typename T>
T *getArgumentsArray() {
    return Memory::slid<T *>(0x1007ad690);
}

static std::unordered_map<uint16, Android::Implementation> implementations {
    {0xDD0,  Android::getLabelAddress},
    {0xDD1,  Android::getFunctionAddressByName},
    {0xDD2,  Android::contextCallFunction},
    {0xDD3,  Android::contextSetReg},
    {0xDD4,  Android::contextGetReg},
    {0xDD6,  Android::getGameVersion},
    {0xDD7,  Android::getImageBase},
    {0xDD8,  Android::readMemory},
    {0xDD9,  Android::writeMemory},
    {0xDDC,  Android::setMutexVar},
    {0xDDD,  Android::getMutexVar},

    {0xDE0,  Android::getTouchPointState},

    {0xE1,   Android::checkButtonPressed},
    {0x80E1, Android::checkButtonNotPressed},
};

#define InstructionStub(func) \
    void func(GameScript *script) { Debug::logf("Warning: %s is a stub. Expect a crash...", __func__); }

InstructionStub(Android::getLabelAddress);
InstructionStub(Android::getFunctionAddressByName);
InstructionStub(Android::contextCallFunction);
InstructionStub(Android::contextSetReg);
InstructionStub(Android::contextGetReg);
InstructionStub(Android::getGameVersion);
InstructionStub(Android::getImageBase);
InstructionStub(Android::readMemory);
InstructionStub(Android::writeMemory);

static std::unordered_map<uint32, uint32> mutexVars {};

void Android::setMutexVar(GameScript *script) {
    script->readArguments(2);

    uint32 *args = getArgumentsArray<uint32>();

//    Debug::logf("setMutexVar(value: %d, to: %d)", args[0], args[1]);
    mutexVars[args[0]] = args[1];
}

InstructionStub(Android::getMutexVar);

bool processZoneQuery(GameScript *script, int pointIndex = 1) {
    // Touch zone check.
    script->readArguments(2);

    // 0x1007ad690 is the argument list address.
    int touchZone = Memory::slid<int *>(0x1007ad690)[pointIndex];

    if(0 < touchZone && touchZone < 10) {
//        Debug::logf("checking touch zone %d", touchZone);

        bool zoneStatus = Touch::touchAreaPressed(touchZone);
        if(zoneStatus) {
//            Debug::logf("zone is pressed");
        }

        return zoneStatus;
    }

    Debug::logf("ignoring invalid touch zone %d", touchZone);
    return false;
}

void Android::checkButtonPressed(GameScript *script) {
    script->handleFlag(processZoneQuery(script));
}

void Android::checkButtonNotPressed(GameScript *script) {
    script->handleFlag(!processZoneQuery(script));
}

void Android::getTouchPointState(GameScript *script) {
    int *destination = (int *)(script->readVariable());
    *destination = processZoneQuery(script, 0);
}

Android::Implementation Android::getImplementation(uint16 opcode) {
    auto it = implementations.find(opcode);
    if(it != implementations.end()) {
        return it->second;
    }

    return nullptr;
}