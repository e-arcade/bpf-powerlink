#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <net/if.h>

#include "handle-powerlink.skel.h" // generated skeleton header

int main(int argc, char **argv)
{
	struct handle_powerlink_bpf *obj;
	int ifindex, err;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <ifname>\n", argv[0]);
		return 1;
	}

	const char *ifname = argv[1];
	ifindex = if_nametoindex(ifname);
	if (ifindex == 0) {
		fprintf(stderr, "invalid interface name: %s\n", ifname);
		return 1;
	}
	
	/* open eBPF application */
	obj = handle_powerlink_bpf__open();
	if (!obj) {
		fprintf(stderr, "failed to open BPF skeleton\n");
        	return 1;
	}
	
	/* load and verify eBPF program */
	err = handle_powerlink_bpf__load(obj);
	if (err) {
		fprintf(stderr, "failed to load and verify BPF object: %d\n", err);
        	goto cleanup;
    	}

	/* attach XDP program */
    	err = handle_powerlink_bpf__attach(obj);
    	if (err)
    	{
        	fprintf(stderr, "failed to attach BPF object: %d\n", err);
        	goto cleanup;
    	}
	
	/* attach bpf program to specific interface */
	obj->links.xdp_handle_powerlink = bpf_program__attach_xdp(obj->progs.xdp_handle_powerlink, ifindex);
	if (!obj->links.xdp_handle_powerlink) {
		err = -errno;
        	fprintf(stderr, "failed to attach XDP program: %s\n", strerror(errno));
        	goto cleanup;
    	}

	printf("successfully attached XDP program to interface %s\n", ifname);

	for (;;) pause();

cleanup:
	handle_powerlink_bpf__destroy(obj);
	return -err;
}
