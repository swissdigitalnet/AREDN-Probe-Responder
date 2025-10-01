/*
 * AREDN Probe Responder
 *
 * Lightweight UDP echo service for AREDN mesh network monitoring.
 * Listens on UDP port 40050 and echoes packets back to sender.
 *
 * Compatible with AREDN-Phonebook network monitoring probes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>

#define PROBE_PORT 40050
#define BUFFER_SIZE 1024

volatile sig_atomic_t keep_running = 1;

void signal_handler(int signum) {
    if (signum == SIGTERM || signum == SIGINT) {
        keep_running = 0;
    }
}

int main(void) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    ssize_t recv_len, sent_len;

    // Set up signal handlers
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    // Open syslog
    openlog("probe-responder", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Starting AREDN probe responder on port %d", PROBE_PORT);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        syslog(LOG_ERR, "Failed to create socket: %s", strerror(errno));
        closelog();
        return EXIT_FAILURE;
    }

    // Set socket to reuse address
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        syslog(LOG_WARNING, "Failed to set SO_REUSEADDR: %s", strerror(errno));
    }

    // Bind to port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PROBE_PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        syslog(LOG_ERR, "Failed to bind to port %d: %s", PROBE_PORT, strerror(errno));
        close(sockfd);
        closelog();
        return EXIT_FAILURE;
    }

    syslog(LOG_INFO, "Probe responder listening on UDP port %d", PROBE_PORT);

    // Main echo loop
    while (keep_running) {
        client_len = sizeof(client_addr);

        // Receive packet
        recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                           (struct sockaddr *)&client_addr, &client_len);

        if (recv_len < 0) {
            if (errno == EINTR) continue; // Interrupted by signal
            syslog(LOG_ERR, "recvfrom error: %s", strerror(errno));
            continue;
        }

        // Echo packet back to sender
        sent_len = sendto(sockfd, buffer, recv_len, 0,
                         (struct sockaddr *)&client_addr, client_len);

        if (sent_len < 0) {
            syslog(LOG_WARNING, "sendto error: %s", strerror(errno));
        }
    }

    // Cleanup
    syslog(LOG_INFO, "Shutting down probe responder");
    close(sockfd);
    closelog();

    return EXIT_SUCCESS;
}
