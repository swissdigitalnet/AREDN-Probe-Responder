# AREDN Probe Responder — Functional Specification

**Version:** 1.0.0
**Date:** 2025-10-01
**Status:** Production Ready

---

## 1. Overview

### 1.1 Purpose

AREDN Probe Responder is a lightweight UDP echo service designed for AREDN mesh network monitoring. It enables network monitoring agents to measure round-trip time (RTT), jitter, and packet loss to nodes that don't run a full monitoring stack.

### 1.2 Scope

This specification covers **only the probe responder agent** - a standalone service that:
- Listens on UDP port 40050
- Echoes received packets back to sender
- Logs activity to syslog
- Runs as a system service via procd

Backend collection, analysis, dashboards, and alerting are handled by separate projects (e.g., AREDN-Phonebook with mesh monitoring).

---

## 2. Requirements

### 2.1 Functional Requirements

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-1 | Listen on UDP port 40050 | MUST |
| FR-2 | Echo all received packets back to sender | MUST |
| FR-3 | Handle packets up to 1024 bytes | MUST |
| FR-4 | Log service start/stop/errors to syslog | MUST |
| FR-5 | Gracefully handle SIGTERM/SIGINT | MUST |
| FR-6 | Start automatically on boot via init.d | SHOULD |

### 2.2 Non-Functional Requirements

| ID | Requirement | Priority |
|----|-------------|----------|
| NFR-1 | Minimal memory footprint (<1MB RAM) | MUST |
| NFR-2 | Minimal CPU usage (<0.1% idle) | MUST |
| NFR-3 | No dependencies beyond libc | MUST |
| NFR-4 | Compatible with OpenWRT 23.05+ | MUST |
| NFR-5 | Support x86_64, ath79, ipq40xx platforms | MUST |

---

## 3. Architecture

### 3.1 System Architecture

```
┌─────────────────────────────────────────┐
│   AREDN Router (VM-TUNNELSERVER)        │
├─────────────────────────────────────────┤
│                                          │
│  ┌────────────────────────────────┐    │
│  │  probe-responder (daemon)      │    │
│  │  - Binds to UDP 40050          │    │
│  │  - Echoes packets              │    │
│  │  - Logs to syslog              │    │
│  └────────────────────────────────┘    │
│              ↕                          │
│         UDP:40050                       │
│              ↕                          │
└─────────────────────────────────────────┘
              ↕
       AREDN Mesh Network
              ↕
┌─────────────────────────────────────────┐
│   Monitoring Node (VM-1 with Phonebook) │
├─────────────────────────────────────────┤
│  - Sends probe packets                  │
│  - Calculates RTT, jitter, loss         │
│  - Displays metrics via CGI             │
└─────────────────────────────────────────┘
```

### 3.2 Protocol

**Probe Request/Response:**
- Protocol: UDP
- Port: 40050
- Payload: Any data (up to 1024 bytes)
- Behavior: Echo exact payload back to source IP:port

**Example:**
```
Client sends:    "probe_12345\n" to 10.167.247.74:40050
Server echoes:   "probe_12345\n" back to client_ip:client_port
Client measures: RTT = (receive_time - send_time)
```

---

## 4. Implementation

### 4.1 Source Code

**File:** `src/probe_responder.c` (94 lines)

**Key Functions:**
- `main()` - Initialize socket, bind, enter echo loop
- `signal_handler()` - Handle SIGTERM/SIGINT for graceful shutdown

**Dependencies:**
- Standard C library (libc)
- POSIX sockets
- syslog

### 4.2 Build System

**OpenWRT Package:**
- Name: `aredn-probe-responder`
- Version: 1.0.0
- Binary: `/usr/bin/probe-responder`
- Init script: `/etc/init.d/probe-responder`

### 4.3 Service Management

**Init Script:** `/etc/init.d/probe-responder`
- Type: procd
- Start priority: 95 (late start)
- Stop priority: 10 (early stop)
- Auto-restart: Yes (on crash)

---

## 5. Usage

### 5.1 Installation

```bash
# Install package
opkg install aredn-probe-responder_*.ipk

# Enable service
/etc/init.d/probe-responder enable

# Start service
/etc/init.d/probe-responder start
```

### 5.2 Verification

```bash
# Check service status
/etc/init.d/probe-responder status

# Verify port is listening
netstat -uln | grep 40050

# View logs
logread | grep probe-responder

# Test locally
echo "test" | nc -u -w1 127.0.0.1 40050
```

### 5.3 Monitoring Integration

Once installed, the node will automatically respond to probes from:
- AREDN-Phonebook v2.0+ with network monitoring enabled
- Any RFC3550-compliant network monitoring tool

---

## 6. Security Considerations

### 6.1 Attack Surface

- **Minimal:** Single UDP port, no authentication
- **Risk:** Potential DoS via packet flooding
- **Mitigation:** Runs on trusted AREDN mesh networks only

### 6.2 Recommendations

- Deploy only on trusted mesh networks
- Consider firewall rules to limit sources
- Monitor syslog for unusual activity

---

## 7. Testing

### 7.1 Unit Tests

Manual testing via:
```bash
# Local echo test
echo "ping" | nc -u -w1 127.0.0.1 40050

# Remote echo test
echo "ping" | nc -u -w1 10.167.247.74 40050

# Stress test (100 packets)
for i in {1..100}; do echo "probe_$i" | nc -u -w1 10.167.247.74 40050; done
```

### 7.2 Integration Tests

With AREDN-Phonebook monitoring:
```bash
# On monitoring node
curl http://localhost/cgi-bin/network

# Expected output should include target node with:
# - rtt_ms_avg > 0
# - jitter_ms >= 0
# - loss_pct < 100
```

---

## 8. Performance Metrics

### 8.1 Resource Usage

- **Binary size:** ~15KB
- **RAM usage:** <500KB
- **CPU usage:** <0.1% (idle), ~1% (under load)

### 8.2 Capacity

- **Throughput:** 1000+ probes/second
- **Latency:** <1ms echo time (local network)

---

## 9. Maintenance

### 9.1 Logging

All events logged to syslog with facility `LOG_DAEMON`:
- Service start/stop
- Bind failures
- Socket errors

### 9.2 Troubleshooting

**Service won't start:**
```bash
logread | grep probe-responder
# Check for "Failed to bind" - port conflict
```

**No probe responses:**
```bash
# Check firewall
iptables -L INPUT -v -n | grep 40050

# Verify service running
ps | grep probe-responder
```

---

## 10. Future Enhancements

Potential future features (not in v1.0):
- Configuration file for custom port
- Packet statistics (counter endpoint)
- Rate limiting
- IPv6 support

---

## 11. References

- **AREDN-Phonebook:** https://github.com/swissdigitalnet/AREDN-Phonebook
- **RFC3550:** RTP: A Transport Protocol for Real-Time Applications
- **OpenWRT SDK:** https://openwrt.org/docs/guide-developer/toolchain/using_the_sdk

---

## Appendix A: Compatibility Matrix

| Platform | Architecture | Status | Tested |
|----------|-------------|--------|--------|
| x86_64 VM | x86/64 | ✅ Supported | Yes |
| Mikrotik hAP | ath79/generic | ✅ Supported | Yes |
| GL.iNet | ipq40xx/generic | ✅ Supported | No |
| TP-Link | ath79/generic | ✅ Supported | No |

---

**Document Status:** Complete
**Implementation Status:** Production Ready
**Last Updated:** 2025-10-01
