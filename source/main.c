/* This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

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

int main(int argc, char* argv[]) {
    consoleInit(NULL);
    printf("SwitchTime v0.1.0\n\n");

    if (!setsysInternetTimeSyncIsOn()) {
        printf(
            "Internet time sync is not enabled. Please enable it in System Settings.\n\n"
            "Press + to quit...\n");

        while (appletMainLoop()) {
            hidScanInput();
            u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

            if (kDown & KEY_PLUS) {
                consoleExit(NULL);
                return 0;  // return to hbmenu
            }

            consoleUpdate(NULL);
        }
    }

    time_t timeToSet = time(NULL);
    struct tm* p_tm_timeToSet = localtime(&timeToSet);

    // Main loop
    while (appletMainLoop()) {
        printf(
            "\n\n\n"
            "Press: UP/DOWN to change day | LEFT/RIGHT to change hour\n"
            "       A to confirm time     | Y to reset to current time (ntp.org time server)\n"
            "                             | + to quit\n\n\n");

        while (appletMainLoop()) {
            char timeStr[25];
            strftime(timeStr, sizeof timeStr, "%c", p_tm_timeToSet);
            printf("\rTime to set: %s", timeStr);

            hidScanInput();
            u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

            if (kDown & KEY_PLUS) {
                consoleExit(NULL);
                return 0;  // return to hbmenu
            }

            if (kDown & KEY_LEFT) {
                p_tm_timeToSet->tm_mday--;
            } else if (kDown & KEY_RIGHT) {
                p_tm_timeToSet->tm_mday++;
            } else if (kDown & KEY_DOWN) {
                p_tm_timeToSet->tm_hour--;
            } else if (kDown & KEY_UP) {
                p_tm_timeToSet->tm_hour++;
            } else if (kDown & KEY_A) {
                printf("\n\n\n");
                setNetworkSystemClock(timeToSet);
                break;
            } else if (kDown & KEY_Y) {
                printf("\n\n\n");
                ntpGetTime(&timeToSet);
                p_tm_timeToSet = localtime(&timeToSet);
                setNetworkSystemClock(timeToSet);
                break;
            }

            timeToSet = mktime(p_tm_timeToSet);

            consoleUpdate(NULL);
        }        
    }
}
