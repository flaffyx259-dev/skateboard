#include "includes.h"
#include "crash.h"
#include <thread>
#include <chrono>
#include <tchar.h>
#include <algorithm>
#include <iterator>
#include <iostream>

int __stdcall DllMain(HMODULE self, ulong_t reason, void* reserved) {

    if (reason == DLL_PROCESS_ATTACH) {
        // install the crash handler first thing, so even a fault during init
        // gets reported with a full report instead of a silent death.
        CrashLog::Init(XOR("dbg"));

        g_cl.m_user = "skateboard";
        HANDLE thread = CreateThread(nullptr, 0, Client::init, nullptr, 0, nullptr);
        if (!thread)
            return 0;
        return 1;
    }
}