# Source Code Structure

-   `/build` - directory encapsulating all generated artifacts, shall be .gitignored
-   `/downloads`
    -   `/packages` - directory for third party packages needed for the build
-   `/src` - all sources
-   `Makefile` - makefile arranged so that `make` in project root builds all artifacts

# Build

```
git clone --recurse-submodules git@github.com:edwin/bpf-powerlink.git
cd ./bpf-powerlink && make
```

# Run

```
1) sudo ./handle-powerlink <ifname>

OR

2) sudo ip link set dev <ifname> xdpgeneric object build/handle-powerlink.bpf.o sec xdp
   sudo ip link set dev <ifname> xdpgeneric off
```

