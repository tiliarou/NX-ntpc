/* This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this software, either in source code form or
as a compiled binary, for any purpose, commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this software dedicate any and all copyright
interest in the software to the public domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an overt act of relinquishment in perpetuity
of all present and future rights to this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include "ntp.h"

bool setsysInternetTimeSyncIsOn() {
    Result rs = setsysInitialize();
    if (R_FAILED(rs)) {
        printf("setsysInitialize failed, %x\n", rs);
        return false;
    }

    bool internetTimeSyncIsOn;
    rs = setsysGetFlag(60, &internetTimeSyncIsOn);
    setsysExit();
    if (R_FAILED(rs)) {
        printf("Unable to detect if Internet time sync is enabled, %x\n", rs);
        return false;
    }

    return internetTimeSyncIsOn;
}

bool setNetworkSystemClock(time_t time) {
    Result rs = timeSetCurrentTime(TimeType_NetworkSystemClock, (uint64_t)time);
    if (R_FAILED(rs)) {
        printf("timeSetCurrentTime failed with %x\n", rs);
        return false;
    }
    printf("Successfully set NetworkSystemClock.\n");
    return true;
}

int consoleExitWithMsg(char* msg) {
    printf("%s\n\nPress + to quit...", msg);

    while (appletMainLoop()) {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS) {
            consoleExit(NULL);
            return 0;  // return to hbmenu
        }

        consoleUpdate(NULL);
    }
    consoleExit(NULL);
    return 0;
}

int main(int argc, char* argv[]) {
    consoleInit(NULL);
    printf("SwitchTime v0.1.0\n\n");

    if (!setsysInternetTimeSyncIsOn()) {
        return consoleExitWithMsg("Internet time sync is not enabled. Please enable it in System Settings.");
    }

    // Main loop
    while (appletMainLoop()) {
        printf(
            "\n\n\n"
            "Press: UP/DOWN to change hour | LEFT/RIGHT to change day\n"
            "       A to confirm time      | Y to reset to current time (ntp.org time server)\n"
            "                              | + to quit\n\n\n");

        int dayChange = 0, hourChange = 0;
        while (appletMainLoop()) {
            hidScanInput();
            u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

            if (kDown & KEY_PLUS) {
                consoleExit(NULL);
                return 0;  // return to hbmenu
            }

            time_t currentTime;
            Result rs = timeGetCurrentTime(TimeType_NetworkSystemClock, (u64*)&currentTime);
            if (R_FAILED(rs)) {
                printf("timeGetCurrentTime failed with %x", rs);
                return consoleExitWithMsg("");
            }

            struct tm* p_tm_timeToSet = localtime(&currentTime);
            p_tm_timeToSet->tm_mday += dayChange;
            p_tm_timeToSet->tm_hour += hourChange;
            time_t timeToSet = mktime(p_tm_timeToSet);

            if (kDown & KEY_A) {
                printf("\n\n\n");
                setNetworkSystemClock(timeToSet);
                break;
            }

            if (kDown & KEY_Y) {
                printf("\n\n\n");
                rs = ntpGetTime(&timeToSet);
                if (R_SUCCEEDED(rs)) {
                    setNetworkSystemClock(timeToSet);
                }
                break;
            }

            if (kDown & KEY_LEFT) {
                dayChange--;
            } else if (kDown & KEY_RIGHT) {
                dayChange++;
            } else if (kDown & KEY_DOWN) {
                hourChange--;
            } else if (kDown & KEY_UP) {
                hourChange++;
            }

            char timeToSetStr[25];
            strftime(timeToSetStr, sizeof timeToSetStr, "%c", p_tm_timeToSet);
            printf("\rTime to set: %s", timeToSetStr);
            consoleUpdate(NULL);
        }
    }
}
