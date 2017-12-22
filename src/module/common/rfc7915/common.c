#include "rfc7915/common.h"

#include <linux/icmp.h>

#include "config.h"
#include "ipv6-hdr-iterator.h"
#include "module-stats.h"
#include "packet.h"
#include "rfc7915/4to6.h"
#include "rfc7915/6to4.h"

struct backup_skb {
	unsigned int pulled;
	struct {
		int l3;
		int l4;
	} offset;
	void *payload;
	l4_protocol l4_proto;
	struct tuple tuple;
};

static int handle_unknown_l4(struct xlation *state)
{
	return copy_payload(state);
}

static struct translation_steps steps[][L4_PROTO_COUNT] = {
	{ /* IPv6 */
		{
			.skb_create_fn = ttp64_create_skb,
			.l3_hdr_fn = ttp64_ipv4,
			.l3_payload_fn = ttp64_tcp,
		},
		{
			.skb_create_fn = ttp64_create_skb,
			.l3_hdr_fn = ttp64_ipv4,
			.l3_payload_fn = ttp64_udp,
		},
		{
			.skb_create_fn = ttp64_create_skb,
			.l3_hdr_fn = ttp64_ipv4,
			.l3_payload_fn = ttp64_icmp,
		},
		{
			.skb_create_fn = ttp64_create_skb,
			.l3_hdr_fn = ttp64_ipv4,
			.l3_payload_fn = handle_unknown_l4,
		}
	},
	{ /* IPv4 */
		{
			.skb_create_fn = ttp46_create_skb,
			.l3_hdr_fn = ttp46_ipv6,
			.l3_payload_fn = ttp46_tcp,
		},
		{
			.skb_create_fn = ttp46_create_skb,
			.l3_hdr_fn = ttp46_ipv6,
			.l3_payload_fn = ttp46_udp,
		},
		{
			.skb_create_fn = ttp46_create_skb,
			.l3_hdr_fn = ttp46_ipv6,
			.l3_payload_fn = ttp46_icmp,
		},
		{
			.skb_create_fn = ttp46_create_skb,
			.l3_hdr_fn = ttp46_ipv6,
			.l3_payload_fn = handle_unknown_l4,
		}
	}
};

int copy_payload(struct xlation *state)
{
	int error;

	error = skb_copy_bits(state->in.skb, pkt_payload_offset(&state->in),
			pkt_payload(&state->out),
			/*
			 * Note: There's an important reason why the payload
			 * length must be extracted from the outgoing packet:
			 * the outgoing packet might be truncated. See
			 * ttp46_create_skb() and ttp64_create_skb().
			 */
			pkt_payload_len_frag(&state->out));
	if (error) {
		log_debug("The payload copy threw errcode %d.", error);
		return breakdown(state, JOOL_MIB_SKB_COPY_BITS, error);
	}

	return 0;
}

bool will_need_frag_hdr(const struct iphdr *hdr)
{
	return is_mf_set_ipv4(hdr) || get_fragment_offset_ipv4(hdr);
}

static int report_bug247(struct packet *pkt, __u8 proto)
{
	struct sk_buff *skb = pkt->skb;
	struct skb_shared_info *shinfo = skb_shinfo(skb);
	unsigned int i;

	pr_err("----- JOOL OUTPUT -----\n");
	pr_err("Bug #247 happened!\n");

	/* FIXME */
	pr_err("xlator: %u " JOOL_VERSION_STR, /* xlat_is_siit() */ 1);
	pr_err("Page size: %lu\n", PAGE_SIZE);
	pr_err("Page shift: %u\n", PAGE_SHIFT);
	pr_err("protocols: %u %u %u\n", pkt->l3_proto, pkt->l4_proto, proto);

	snapshot_report(&pkt->debug.shot1, "initial");
	snapshot_report(&pkt->debug.shot2, "mid");

	pr_err("current len: %u\n", skb->len);
	pr_err("current data_len: %u\n", skb->data_len);
	pr_err("current nr_frags: %u\n", shinfo->nr_frags);
	for (i = 0; i < shinfo->nr_frags; i++) {
		pr_err("    current frag %u: %u\n", i,
				skb_frag_size(&shinfo->frags[i]));
	}

	pr_err("Dropping packet.\n");
	pr_err("-----------------------\n");
	return -EINVAL;
}

static int move_pointers_in(struct xlation *state, __u8 protocol,
		unsigned int l3hdr_len)
{
	struct packet *pkt = &state->in;
	unsigned int l4hdr_len;

	if (unlikely(pkt->skb->len < pkt->skb->data_len)) {
		return breakdown(state, JOOL_MIB_ISSUE247,
				report_bug247(pkt, protocol));
	}

	skb_pull(pkt->skb, pkt_hdrs_len(pkt));
	skb_reset_network_header(pkt->skb);
	skb_set_transport_header(pkt->skb, l3hdr_len);

	switch (protocol) {
	case IPPROTO_TCP:
		pkt->l4_proto = L4PROTO_TCP;
		l4hdr_len = tcp_hdr_len(pkt_tcp_hdr(pkt));
		break;
	case IPPROTO_UDP:
		pkt->l4_proto = L4PROTO_UDP;
		l4hdr_len = sizeof(struct udphdr);
		break;
	case IPPROTO_ICMP:
	case NEXTHDR_ICMP:
		pkt->l4_proto = L4PROTO_ICMP;
		l4hdr_len = sizeof(struct icmphdr);
		break;
	default:
		pkt->l4_proto = L4PROTO_OTHER;
		l4hdr_len = 0;
		break;
	}
	pkt->is_inner = true;
	pkt->payload = skb_transport_header(pkt->skb) + l4hdr_len;

	return 0;
}

static void move_pointers_out(struct packet *in, struct packet *out,
		unsigned int l3hdr_len)
{
	skb_pull(out->skb, pkt_hdrs_len(out));
	skb_reset_network_header(out->skb);
	skb_set_transport_header(out->skb, l3hdr_len);

	out->l4_proto = pkt_l4_proto(in);
	out->is_inner = true;
	out->payload = skb_transport_header(out->skb) + pkt_l4hdr_len(in);
}

static int move_pointers4(struct xlation *state)
{
	struct iphdr *hdr4;
	unsigned int l3hdr_len;
	int error;

	hdr4 = pkt_payload(&state->in);
	error = move_pointers_in(state, hdr4->protocol, 4 * hdr4->ihl);
	if (error)
		return error;

	l3hdr_len = sizeof(struct ipv6hdr);
	if (will_need_frag_hdr(hdr4))
		l3hdr_len += sizeof(struct frag_hdr);
	move_pointers_out(&state->in, &state->out, l3hdr_len);
	return 0;
}

static int move_pointers6(struct xlation *state)
{
	struct ipv6hdr *hdr6 = pkt_payload(&state->in);
	struct hdr_iterator iterator = HDR_ITERATOR_INIT(hdr6);
	int error;

	hdr_iterator_last(&iterator);

	error = move_pointers_in(state, iterator.hdr_type,
			iterator.data - (void *)hdr6);
	if (error)
		return error;

	move_pointers_out(&state->in, &state->out, sizeof(struct iphdr));
	return 0;
}

static void backup(struct xlation *state, struct packet *pkt,
		struct backup_skb *bkp)
{
	bkp->pulled = pkt_hdrs_len(pkt);
	bkp->offset.l3 = skb_network_offset(pkt->skb);
	bkp->offset.l4 = skb_transport_offset(pkt->skb);
	bkp->payload = pkt_payload(pkt);
	bkp->l4_proto = pkt_l4_proto(pkt);
	if (state->jool.type == XLATOR_NAT64)
		bkp->tuple = pkt->tuple;
}

static void restore(struct xlation *state, struct packet *pkt,
		struct backup_skb *bkp)
{
	skb_push(pkt->skb, bkp->pulled);
	skb_set_network_header(pkt->skb, bkp->offset.l3);
	skb_set_transport_header(pkt->skb, bkp->offset.l4);
	pkt->payload = bkp->payload;
	pkt->l4_proto = bkp->l4_proto;
	pkt->is_inner = 0;
	if (state->jool.type == XLATOR_NAT64)
		pkt->tuple = bkp->tuple;
}

int ttpcomm_translate_inner_packet(struct xlation *state)
{
	struct packet *in = &state->in;
	struct packet *out = &state->out;
	struct backup_skb bkp_in, bkp_out;
	struct translation_steps *current_steps;
	int error;

	backup(state, in, &bkp_in);
	backup(state, out, &bkp_out);

	switch (pkt_l3_proto(in)) {
	case L3PROTO_IPV4:
		error = move_pointers4(state);
		break;
	case L3PROTO_IPV6:
		error = move_pointers6(state);
		break;
	default:
		return einval(state, JOOL_MIB_UNKNOWN_L3);
	}
	if (error)
		return error;

	if (state->jool.type == XLATOR_NAT64) {
		in->tuple.src = bkp_in.tuple.dst;
		in->tuple.dst = bkp_in.tuple.src;
		out->tuple.src = bkp_out.tuple.dst;
		out->tuple.dst = bkp_out.tuple.src;
	}

	current_steps = &steps[pkt_l3_proto(in)][pkt_l4_proto(in)];
	error = current_steps->l3_hdr_fn(state);
	if (error)
		return error;
	error = current_steps->l3_payload_fn(state);
	if (error)
		return error;

	restore(state, in, &bkp_in);
	restore(state, out, &bkp_out);

	return 0;
}

struct translation_steps *ttpcomm_get_steps(struct packet *in)
{
	return &steps[pkt_l3_proto(in)][pkt_l4_proto(in)];
}

/**
 * partialize_skb - set up @out_skb so the layer 4 checksum will be computed
 * from almost-scratch by the OS or by the NIC later.
 * @csum_offset: The checksum field's offset within its header.
 *
 * When the incoming skb's ip_summed field is NONE, UNNECESSARY or COMPLETE,
 * the checksum is defined, in the sense that its correctness consistently
 * dictates whether the packet is corrupted or not. In these cases, Jool is
 * supposed to update the checksum with the translation changes (pseudoheader
 * and transport header) and forget about it. The incoming packet's corruption
 * will still be reflected in the outgoing packet's checksum.
 *
 * On the other hand, when the incoming skb's ip_summed field is PARTIAL,
 * the existing checksum only covers the pseudoheader (which Jool replaces).
 * In these cases, fully updating the checksum is wrong because it doesn't
 * already cover the transport header, and fully computing it again is wasted
 * time because this work can be deferred to the NIC (which'll likely do it
 * faster).
 *
 * The correct thing to do is convert the partial (pseudoheader-only) checksum
 * into a translated-partial (pseudoheader-only) checksum, and set up some skb
 * fields so the NIC can do its thing.
 *
 * This function handles the skb fields setting part.
 */
void partialize_skb(struct sk_buff *out_skb, unsigned int csum_offset)
{
	out_skb->ip_summed = CHECKSUM_PARTIAL;
	out_skb->csum_start = skb_transport_header(out_skb) - out_skb->head;
	out_skb->csum_offset = csum_offset;
}