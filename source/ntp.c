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

For more information, please refer to <https://unlicense.org>

Additionally, the ntp_packet struct uses code licensed under the BSD 3-clause. See LICENSE-THIRD-PARTY for more
information. */

#include "ntp.h"

bool nifmInternetIsConnected() {
    Result rs = nifmInitialize();
    if (R_FAILED(rs)) {
        printf("Failed to get Internet connection status. Error code %x\n", rs);
        return false;
    }

    NifmInternetConnectionStatus nifmICS;
    rs = nifmGetInternetConnectionStatus(NULL, NULL, &nifmICS);
    nifmExit();

    if (R_FAILED(rs)) {
        printf("nifmGetInternetConnectionStatus failed with error code %x\n", rs);
    }

    if (nifmICS != NifmInternetConnectionStatus_Connected) {
        return false;
    }

    return true;
}

Result ntpGetTime(time_t *p_resultTime) {
    if (!nifmInternetIsConnected()) {
        printf("You're not connected to the Internet. Please run this application again after connecting.\n");
        return -1;
    }

    Result rs = socketInitializeDefault();
    if (R_FAILED(rs)) {
        printf("Failed to init socket services, error code %x\n", rs);
        return rs;
    }
    printf("Socket services initialized\n");

    int sockfd = -1;
    const char *server_name = "0.pool.ntp.org";
    const uint16_t port = 123;

    ntp_packet packet;
    memset(&packet, 0, sizeof(ntp_packet));

    packet.li_vn_mode = (0 << 6) | (4 << 3) | 3;  // LI 0 | Client version 4 | Mode 3

    packet.txTm_s = htonl(NTP_TIMESTAMP_DELTA + time(NULL));  // Current networktime on the console

    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        printf("Failed to open socket with error code %x\n", errno);
        goto failed;
    }

    printf("Opened socket\n");
    printf("Attempting to connect to %s\n", server_name);
    errno = 0;
    if ((server = gethostbyname(server_name)) == NULL) {
        printf("Gethostbyname failed: %x\n", errno);
        goto failed;
    }

    memset(&serv_addr, 0, sizeof(struct sockaddr_in));

    serv_addr.sin_family = AF_INET;

    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr_list[0], 4);

    serv_addr.sin_port = htons(port);

    errno = 0;
    int res = 0;
    if ((res = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("Connect failed: %x %x\n", res, errno);
        goto failed;
    }

    printf("Connected to 0.pool.ntp.org with result: %x %x\nSending time request...\n", res, errno);

    errno = 0;
    if ((res = send(sockfd, (char *)&packet, sizeof(ntp_packet), 0)) < 0) {
        printf("Error writing to socket: %x %x\n", res, errno);
        goto failed;
    }

    printf("Sent time request with result: %x %x, waiting for response...\n", res, errno);

    errno = 0;
    if ((res = recv(sockfd, (char *)&packet, sizeof(ntp_packet), 0)) < sizeof(ntp_packet)) {
        printf("Error reading from socket: %x %x\n", res, errno);
        goto failed;
    }

    packet.txTm_s = ntohl(packet.txTm_s);

    *p_resultTime = (time_t)(packet.txTm_s - NTP_TIMESTAMP_DELTA);

    printf("Time received from server: %s\n", ctime(p_resultTime));

    socketExit();
    return 0;

failed:
    socketExit();
    return -1;
}
