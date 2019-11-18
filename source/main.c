#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include "ntp.h"

int main(int argc, char* argv[]) {
    consoleInit(NULL);

    printf("Hello World!\n");

    // Main loop
    while (appletMainLoop()) {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS)
            break;  // break in order to return to hbmenu

        // test ntpGetTime
        if (kDown & KEY_A) {
            time_t resultTime;
            ntpGetTime(&resultTime);
        }

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
