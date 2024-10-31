# Root Makefile
# `make` here builds all artifacts and runs all tests
#
#
# `make handle-powerlink` build process example:
#   - build libbpf library                [./build]
#   - build bpftool utility               [./build] 
#   - generate vmlinux.h header           [./build]
#   - generate handle-powerlink.bpf.o     [./build]
#   - generate handle-powerlink.skel.h    [./build]
#   - build handle-powerlink.o            [./build]
#   - build handle-powerlink executable   [.]
#
# 
# load eBPF program receipt:
#   - using userspace executable (currently doesn't work):
#
#         sudo ./executable <ifname>
#
#     for example, if you want to attach 'handle-powerlink'
#     eBPF program to enp2s0f3 interface, do this:
#
#         sudo ./handle-powerlink enp2s0f3
#
#   - do not use executable and use iproute2 instead:
#     	  
#         (load)   sudo ip link set dev enp2s0f3 xdpgeneric object build/handle-powerlink.bpf.o sec xdp
#         (unload) sudo ip link set dev enp2s0f3 xdpgeneric off
#

CC=clang
CFLAGS=-g -O2 -c -target bpf

PROGS := handle-powerlink

SRC := src
OUTPUT := build

LIBBPF_SRC := $(abspath downloads/packages/libbpf/src)
LIBBPF_OBJ := $(abspath $(OUTPUT)/libbpf.a)

BPFTOOL_SRC := $(abspath downloads/packages/bpftool/src)
BPFTOOL_OUTPUT ?= $(abspath $(OUTPUT)/bpftool)
BPFTOOL ?= $(BPFTOOL_OUTPUT)/bootstrap/bpftool

LIBBPF_STATIC := $(abspath $(OUTPUT)/libbpf/libbpf.a)

INCLUDES := -I$(OUTPUT) -I$(OUTPUT)/root/usr/include

VMLINUX := /sys/kernel/btf/vmlinux

ifeq ($(V),1)
        Q =
        msg =
else
        Q = @
        msg = @printf '  %-8s %s%s\n'                                   \
                      "$(1)"                                            \
                      "$(patsubst $(abspath $(OUTPUT))/%,%,$(2))"       \
                      "$(if $(3), $(3))";
        MAKEFLAGS += --no-print-directory
endif

.PHONY: all
all: $(PROGS)

.PHONY: clean
clean:
	$(call msg,CLEAN,./$(OUTPUT))
	$(call msg,CLEAN,$(PROGS))
	$(Q)rm -rf $(OUTPUT) $(PROGS)

$(OUTPUT) $(OUTPUT)/libbpf $(BPFTOOL_OUTPUT):
	$(call msg,MKDIR,$@)
	$(Q)mkdir -p $@

# build libbpf
$(LIBBPF_OBJ): $(wildcard $(LIBBPF_SRC)/*.[ch] $(LIBBPF_SRC)/Makefile) | $(OUTPUT)/libbpf
	$(call msg,LIB,$@)
	$(Q)$(MAKE) -C $(LIBBPF_SRC) BUILD_STATIC_ONLY=1     		\
		    OBJDIR=$(dir $@)libbpf DESTDIR=$(dir $@)root	\
		    -s install

# build bpftool
$(BPFTOOL): | $(BPFTOOL_OUTPUT)
	$(call msg,BPFTOOL,$@)
	$(Q)$(MAKE) OUTPUT=$(BPFTOOL_OUTPUT)/ -C $(BPFTOOL_SRC) bootstrap

# build eBPF program
$(OUTPUT)/%.bpf.o: $(SRC)/%.bpf.c $(LIBBPF_OBJ) $(wildcard %.h) $(VMLINUX) | $(OUTPUT) $(BPFTOOL)
	$(call msg,BPF,$@)
	$(Q)$(BPFTOOL) btf dump file $(VMLINUX) format c > $(OUTPUT)/vmlinux.h
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(filter %.c,$^) -o $(patsubst %.bpf.o,%.tmp.bpf.o,$@)
	$(Q)$(BPFTOOL) gen object $@ $(patsubst %.bpf.o,%.tmp.bpf.o,$@)

# generate eBPF skeleton
$(OUTPUT)/%.skel.h: $(OUTPUT)/%.bpf.o | $(OUTPUT) $(BPFTOOL)
	$(call msg,GEN-SKEL,$@)
	$(Q)$(BPFTOOL) gen skeleton $< > $@

# build user-space code
$(patsubst %,$(OUTPUT)/%.o,$(PROGS)): %.o: %.skel.h

# build application objects
$(OUTPUT)/%.o: $(SRC)/%.c | $(OUTPUT)
	$(call msg,CC,$@)
	$(Q)$(CC) -g -O2 $(INCLUDES) -c $(filter %.c,$^) -o $@

# build application binaries
$(PROGS): %: $(OUTPUT)/%.o | $(OUTPUT)
	$(call msg,BINARY,$@)
	$(Q)$(CC) -g -O2 $^ -o $@ $(LIBBPF_STATIC) -lelf -lz

# delete failed targets
.DELETE_ON_ERROR:

# keep intermediate (.skel.h, .bpf.o, etc) targets
.SECONDARY:
