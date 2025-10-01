# AREDN Probe Responder

Lightweight UDP echo service for AREDN mesh network monitoring.

## Overview

AREDN Probe Responder is a minimal service that listens on UDP port 40050 and echoes packets back to the sender. This enables network monitoring agents (like [AREDN-Phonebook](https://github.com/swissdigitalnet/AREDN-Phonebook)) to measure RTT, jitter, and packet loss to nodes that don't run the full phonebook service.

## Features

- 🪶 **Lightweight**: ~50 lines of C code, minimal memory footprint
- 📡 **Standards-compliant**: Compatible with AREDN-Phonebook network monitoring
- 🔒 **Reliable**: Uses syslog for monitoring, procd for service management
- 🚀 **Zero configuration**: Works out of the box after installation

## Use Cases

Install this package on AREDN nodes that:
- Don't need SIP phonebook functionality
- Serve as monitoring targets for network health checks
- Need to respond to mesh quality probes from monitoring agents

**Example:** Tunnel servers, backbone nodes, repeaters, or any infrastructure nodes that need to be monitored but don't host SIP phones.

## Installation

### Download Pre-built Package

Download the appropriate `.ipk` for your platform from [Releases](https://github.com/swissdigitalnet/AREDN-Probe-Responder/releases):

- **x86_64**: Virtual machines, x86 hardware
- **ath79**: Mikrotik hAP, TP-Link devices
- **ipq40xx**: GL.iNet devices, modern routers

### Install on AREDN Node

```bash
# Upload .ipk to node via SCP or wget
scp aredn-probe-responder_*.ipk root@node.local.mesh:/tmp/

# SSH to node
ssh root@node.local.mesh

# Install package
opkg install /tmp/aredn-probe-responder_*.ipk

# Enable and start service
/etc/init.d/probe-responder enable
/etc/init.d/probe-responder start
```

### Verify Operation

```bash
# Check service is running
/etc/init.d/probe-responder status

# Check port is listening
netstat -uln | grep 40050

# View logs
logread | grep probe-responder

# Test locally
echo "test" | nc -u -w1 127.0.0.1 40050
```

## How It Works

1. Service listens on UDP port 40050
2. Receives probe packets from monitoring agents
3. Echoes packets back to sender with original payload
4. Monitoring agent calculates RTT, jitter, and packet loss

## Compatibility

- **Compatible with:** AREDN-Phonebook v2.0+, any RFC3550-compliant network monitor
- **OpenWRT:** 23.05+
- **AREDN Firmware:** 3.24.6+

## Building from Source

Requires OpenWRT SDK:

```bash
# Clone repository
git clone https://github.com/swissdigitalnet/AREDN-Probe-Responder.git

# Copy to OpenWRT SDK
cp -r AREDN-Probe-Responder <SDK_PATH>/package/

# Build
cd <SDK_PATH>
make package/aredn-probe-responder/compile V=s
```

## Troubleshooting

**Service not starting:**
```bash
logread | grep probe-responder
# Check for port conflicts on 40050
```

**No responses to probes:**
```bash
# Verify firewall allows UDP 40050
iptables -L INPUT -v -n | grep 40050

# Test manually
echo "probe" | nc -u -w1 <node-ip> 40050
```

## Monitoring Integration

To monitor this node from AREDN-Phonebook:

1. Install `aredn-probe-responder` on target node
2. On monitoring node with AREDN-Phonebook, probes will automatically discover neighbors via OLSR/Babel
3. View metrics: `curl http://<monitoring-node>/cgi-bin/network`

## Uninstall

```bash
/etc/init.d/probe-responder stop
/etc/init.d/probe-responder disable
opkg remove aredn-probe-responder
```

## License

GPLv3

## Related Projects

- [AREDN-Phonebook](https://github.com/swissdigitalnet/AREDN-Phonebook) - Full SIP server with integrated monitoring
- [AREDN](https://www.arednmesh.org/) - Amateur Radio Emergency Data Network

## Support

- **Issues**: https://github.com/swissdigitalnet/AREDN-Probe-Responder/issues
- **AREDN Forums**: https://www.arednmesh.org/forum
