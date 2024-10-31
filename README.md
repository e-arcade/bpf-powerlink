# Overview

Ethernet Powerlink is a real-time protocol for standard Ethernet.

Ethernet Powerlink expands Ethernet with a mixed polling and timeslicing mechanism. This provides:
-   Guaranteed transfer of time-critical data in very short isochronic cycles with configurable response time
-   Time-synchronisation of all nodes in the network with very high precision of sub-microseconds
-   Transmission of less time-critical data in a reserved asynchronous channel

```
              8           14              11        42- 1500    4      bytes count
	+----------+--------------+---------------+----------+-----+
	| preamble | ethernet hdr | powerlink hdr |   data   | crc |   fields
        +----------+--------------+---------------+----------+-----+
                                 /                 \
                                /                   \
                               /                     \
                              +------+-----+-----+----+
                              | type | dst | src | .. |
                              +------+-----+-----+----+
                                 1      1     1     8
```

# Purpose Of This eBPF Module
Incrementing sixth byte of data field

#### Packet Restrictions:
-   Powerlink protocol packet should be ingress
-   Powerlink protocol packet contains 'type' field equals 0x4 and 'src' field equals 0x81
-   Every twentieth packet is modified

# Source Code Structure

-   `/build` - directory encapsulating all generated artifacts, shall be .gitignored
-   `/downloads`
    -   `/packages` - directory for third party packages needed for the build
-   `/src` - all sources
-   `Makefile` - makefile arranged so that `make` in project root builds all artifacts

# Cloning

```
git clone --recurse-submodules git@github.com:e-arcade/bpf-powerlink.git

OR

git clone --recurse-submodules https://github.com/e-arcade/bpf-powerlink.git
```

# Build

```
cd ./bpf-powerlink && make
```

# Run

```
sudo ./handle-powerlink <ifname>

OR

1) sudo ip link set dev <ifname> xdpgeneric object build/handle-powerlink.bpf.o sec xdp
2) sudo ip link set dev <ifname> xdpgeneric off
```

