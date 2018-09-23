#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include <switch.h>

 #define NTP_TIMESTAMP_DELTA 2208988800ull

void waitInput(void)
{
        while(appletMainLoop())
        {
            hidScanInput();
            if(hidKeysDown(CONTROLLER_P1_AUTO))
                break;
            gfxFlushBuffers();
            gfxSwapBuffers();
        }
}
    
typedef struct
{
  unsigned mode: 3;        // 3 = client, others are values we don't care about
  unsigned vn: 3;          // Protocol version. Currently 4.
  unsigned li: 2;          // leap second adjustment.
  
  uint8_t stratum;         // Stratum level of the local clock.
  uint8_t poll;            // Maximum interval between successive messages.
  uint8_t precision;       // Precision of the local clock.

  uint32_t rootDelay;      // Total round trip delay time.
  uint32_t rootDispersion; // Max error aloud from primary clock source.
  uint32_t refId;          // Reference clock identifier.

  uint32_t refTm_s;        // Reference time-stamp seconds.
  uint32_t refTm_f;        // Reference time-stamp fraction of a second.

  uint32_t origTm_s;       // Originate time-stamp seconds.
  uint32_t origTm_f;       // Originate time-stamp fraction of a second.

  uint32_t rxTm_s;         // Received time-stamp seconds.
  uint32_t rxTm_f;         // Received time-stamp fraction of a second.

  uint32_t txTm_s;         // Transmit time-stamp seconds.
  uint32_t txTm_f;         // Transmit time-stamp fraction of a second.
} ntp_packet;

int main(int argc, char **argv)
{
    const char *server_name = "0.pool.ntp.org";
    const uint16_t port = 123;
    
    gfxInitDefault();
    consoleInit(NULL);

    Result rs = timeInitialize();
    if(R_FAILED(rs))
    {
        printf("Failed to init time services\n");
        waitInput();
        goto done;
    }
    
    printf("Time services initialized\n");
    rs = socketInitializeDefault();
    if(R_FAILED(rs))
    {
        printf("Failed to init socket services\n");
        waitInput();
        goto done;
    }
    
    printf("Socket services initialized\n");
    ntp_packet packet;
    memset(&packet, 0, sizeof(ntp_packet));
    packet.li = 0;
    packet.vn = 4; // NTP version 4
    packet.mode = 3; // Client mode
    
    packet.txTm_s = htonl(NTP_TIMESTAMP_DELTA + time(NULL)); // Current networktime on the console
    
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sockfd < 0)
    {
        printf("Failed to open socket\n");
        waitInput();
        goto done;
    }
    
    printf("Opened socket\n");
    
    printf("Attempting to connect to %s\n", server_name);
        
    if((server = gethostbyname(server_name)) == NULL)
    {
        printf("Gethostbyname failed: %x\n", errno);
        waitInput();
        goto done;
    }
    
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    
    serv_addr.sin_family = AF_INET;
    
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr_list[0], 4);
    
    serv_addr.sin_port = htons(port);
    
    errno = 0;
    int res = 0;
    if((res = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("Connect failed: %x %x\n", res, errno);
        waitInput();
        goto done;        
    }
    
    printf("Connected to 0.pool.ntp.org with result: %x %x\nSending time request...\n", res, errno);
    
    errno = 0;
    if((res = send(sockfd, (char *)&packet, sizeof(ntp_packet), 0)) < 0)
    {
        printf("Error writing to socket: %x %x\n", res, errno);
        waitInput();
        goto done;         
    }
    
    printf("Sent time request with result: %x %x, waiting for response...\n", res, errno);
    
    errno = 0; 
    if((res = recv(sockfd, (char *)&packet, sizeof(ntp_packet), 0)) < sizeof(ntp_packet))
    {
        printf("Error reading from socket: %x %x\n", res, errno);
        waitInput();
        goto done;         
    }
    
    packet.txTm_s = ntohl(packet.txTm_s);

    time_t tim = (time_t) (packet.txTm_s - NTP_TIMESTAMP_DELTA);
    
    printf("Time received from server: %s\n", ctime((const time_t *)&tim));
 
#if 0 
    for(int i = 0; i < 48; ++i)
    {
        printf("%x ", reinterpret[i]);
    }
    
    printf("\n");
#endif

    rs = timeSetCurrentTime(TimeType_NetworkSystemClock, (uint64_t)tim);
    if(R_FAILED(rs))
    {
        printf("Failed to set NetworkSystemClock, %x\n", rs);
        waitInput();
    }
    else
        printf("Successfully set NetworkSystemClock\n");
    
done:
    close(sockfd);
    printf("Press PLUS to quit.\n");

    while(appletMainLoop())
    {
        hidScanInput();

        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS) break;

        gfxFlushBuffers();
        gfxSwapBuffers();
    }
    
    socketExit();
    timeExit();
    gfxExit();
    return 0;
}
