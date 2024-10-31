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

