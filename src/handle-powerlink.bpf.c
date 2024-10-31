#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

#define POWERLINK_TYPE 0x4
#define POWERLINK_SRC  0x81

struct packet {
	__u32 num;
	void *off;
};

struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
   	__type(key, __u32);
    	__type(value, sizeof(struct packet));
	__uint(max_entries, 1);
} packets_map SEC(".maps");

/* powerlink protocol header */
struct pwrhdr {
	__u8 type;
	__u8 dst;
	__u8 src;
	__u8 padding[8];
};

/* check if the packet number is a multiple of the 'freq',
 * also update packet stats using maps packet counter
 */
static bool check_packet_by_sampling_freq(__u32 freq)
{
	__u32 key = 0;
	bool ret = 0;

	struct packet *value = bpf_map_lookup_elem(&packets_map, &key);
	if (value) {
		ret = (value->num % freq == 0);
		value->num++;
	} else {
		bpf_printk("bpf_map_lookup_elem() returns NULL, resetting packet stats");

		/* reset packet stats */
		struct packet new_value = {0};
		new_value.num = 1;
		bpf_map_update_elem(&packets_map, &key, &new_value, BPF_ANY);
	}

	return ret;
}

/* increment necessary byte in 'data' field (debug realization) */
static int increment_byte(struct packet *pkt, void *data_end, int byteno)
{
	pkt->off += byteno;

	/* ensure that the desired byte are within bounds */
	if (pkt->off >= data_end) {
		return -1;
	}

	unsigned char byte = *((unsigned char *)pkt->off);

	bpf_printk("sixth data byte before = %u", byte);
	byte++;
	bpf_printk("sixth data byte after = %d", byte);

	return 0;
}

/* check powerlink header for the required fields */
static bool check_required(struct pwrhdr *pwr, void *data_end, struct packet *pkt)
{	
	/* ensure powerlink header is within bounds */
	if ((void *)(pwr + 1) > data_end) {
		return false;
	}
	pkt->off += sizeof(*pwr);

	bpf_printk("powerlink header exists and within bounds");

	/* check powerlink header for the required fields */
    	if (pwr->type != POWERLINK_TYPE || pwr->src != POWERLINK_SRC) {
		bpf_printk("powerlink header hasn't required fields!");
        	return false;
	}

	bpf_printk("powerlink header has required fields!");
	bpf_printk("type = 0x%x, src = 0x%x", pwr->type, pwr->src);

	return true;
}

/* main eBPF program entry point */
SEC("xdp")
int xdp_handle_powerlink(struct xdp_md *ctx)
{
	int ret = XDP_DROP;

	void *data = (void *)(long)ctx->data;
	void *data_end = (void *)(long)ctx->data_end;
	struct ethhdr *eth = data;

	/* keep track of the current packet stats */
	struct packet pkt = {0};
	pkt.off = data;

	u64 eth_off = sizeof(*eth);
	/* ensure ethernet header is within bounds */
	if (data + eth_off > data_end) {
		bpf_printk("");
		return ret;
	}
	pkt.off += eth_off;

	struct pwrhdr *pwr = (struct pwrhdr *)(data + eth_off);
	if (!check_required(pwr, data_end, &pkt)) {
		bpf_printk("");
		return ret;
	}

	/* increment data byte in every twentieth powerlink packet */
        if (check_packet_by_sampling_freq(20)) {
		bpf_printk("packet is multiple of 20, changing byte..");

		/* increment data byte number 6 */
		if (increment_byte(&pkt, data_end, 6) < 0) {
			bpf_printk("");
			return ret;
		}
	}

	bpf_printk("");
	return XDP_PASS;
}

char __license[] SEC("license") = "GPL";
