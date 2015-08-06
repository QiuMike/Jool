#include "nat64/mod/stateless/eam.h"
#include "nat64/mod/common/rtrie.h"
#include "nat64/mod/common/types.h"

/**
 * @author Daniel Hdz Felix
 * @author Alberto Leiva
 */

#define ADDR6_BITS 128
#define ADDR4_BITS 32

struct eam_table {
	struct rtrie_node *tree6;
	struct rtrie_node *tree4;
	u64 count;
};

static struct eam_table eamt;

/**
 * validate_prefixes - check @prefix6 and @prefix4 can be joined together to
 * form a (standalone) legal EAM entry.
 */
static int validate_prefixes(struct ipv6_prefix *prefix6,
		struct ipv4_prefix *prefix4)
{
	int error;

	error = prefix6_validate(prefix6);
	if (error)
		return error;

	error = prefix4_validate(prefix4);
	if (error)
		return error;

	if ((ADDR4_BITS - prefix4->len) > (ADDR6_BITS - prefix6->len)) {
		log_err("The IPv4 suffix length must be smaller or equal than "
				"the IPv6 suffix length.");
		return -EINVAL;
	}

	return 0;
}

static bool eamt_entry_equals(const struct eamt_entry *eam1,
		const struct eamt_entry *eam2)
{
	if (!prefix6_equals(&eam1->prefix6, &eam2->prefix6))
		return false;
	if (!prefix4_equals(&eam1->prefix4, &eam2->prefix4))
		return false;
	return true;
}

static void __revert_add6(struct ipv6_prefix *prefix6)
{
	struct rtrie_key key;
	int error;

	key.bytes = (__u8 *) &prefix6->address;
	key.len = prefix6->len;
	error = rtrie_rm(&eamt.tree6, &key);

	WARN(error, "Got error code %d while trying to remove an EAMT entry I "
			"just added.", error);
}

static int eamt_add6(struct eamt_entry *eam)
{
	int error;

	error = rtrie_add(&eamt.tree6, eam, sizeof(*eam),
			offsetof(typeof(*eam), prefix6.address),
			eam->prefix6.len);
	rtrie_print("IPv6 trie after add", eamt.tree6);

	return error;
}

static int eamt_add4(struct eamt_entry *eam)
{
	int error;

	error = rtrie_add(&eamt.tree4, eam, sizeof(*eam),
			offsetof(typeof(*eam), prefix4.address),
			eam->prefix4.len);
	rtrie_print("IPv4 trie after add", eamt.tree4);

	return error;
}

int eamt_add(struct ipv6_prefix *prefix6, struct ipv4_prefix *prefix4)
{
	struct eamt_entry new;
	int error;

	error = validate_prefixes(prefix6, prefix4);
	if (error)
		return error;

	new.prefix6 = *prefix6;
	new.prefix4 = *prefix4;

	error = eamt_add6(&new);
	if (error)
		return error;
	error = eamt_add4(&new);
	if (error) {
		__revert_add6(prefix6);
		return error;
	}

	eamt.count++;
	return 0;
}

static struct eamt_entry *get_exact6(struct ipv6_prefix *prefix)
{
	struct rtrie_key key;
	struct eamt_entry *result;

	key.bytes = (__u8 *) &prefix->address;
	key.len = prefix->len;

	result = rtrie_get(eamt.tree6, &key);
	if (!result)
		return NULL;

	return (result->prefix6.len == prefix->len) ? result : NULL;
}

static struct eamt_entry *get_exact4(struct ipv4_prefix *prefix)
{
	struct rtrie_key key;
	struct eamt_entry *result;

	key.bytes = (__u8 *) &prefix->address;
	key.len = prefix->len;

	result = rtrie_get(eamt.tree4, &key);
	if (!result)
		return NULL;

	return (result->prefix4.len == prefix->len) ? result : NULL;
}

static int __rm(struct ipv6_prefix *prefix6, struct ipv4_prefix *prefix4)
{
	struct ipv4_prefix tmp;
	struct rtrie_key key6;
	struct rtrie_key key4;
	int error;

	/*
	 * If @prefix4 happens to point to the IPv4 prefix from the IPv6 trie's
	 * EAMT entry, the first rtrie_rm() will turn @prefix4 into a bogus
	 * pointer.
	 * Therefore, work on a copy.
	 * (I don't think this can actually happen, but you know.)
	 */
	memcpy(&tmp, prefix4, sizeof(*prefix4));

	key6.bytes = (__u8 *) &prefix6->address;
	key6.len = prefix6->len;
	key4.bytes = (__u8 *) &tmp.address;
	key4.len = tmp.len;

	error = rtrie_rm(&eamt.tree6, &key6);
	if (error)
		goto corrupted;
	error = rtrie_rm(&eamt.tree4, &key4);
	if (error)
		goto corrupted;

	eamt.count--;
	rtrie_print("IPv6 trie after remove", eamt.tree6);
	rtrie_print("IPv4 trie after remove", eamt.tree4);
	return 0;

corrupted:
	WARN(true, "EAMT entry was extracted from the table, "
			"but it no longer seems to be there. "
			"Errcode: %d", error);
	return error;
}

int eamt_remove(struct ipv6_prefix *prefix6, struct ipv4_prefix *prefix4)
{
	struct eamt_entry *eam6;
	struct eamt_entry *eam4;

	if (WARN(!prefix6 && !prefix4, "Prefixes can't both be NULL"))
		return -EINVAL;

	if (!prefix4) {
		eam6 = get_exact6(prefix6);
		return eam6 ? __rm(prefix6, &eam6->prefix4) : -ESRCH;
	}

	if (!prefix6) {
		eam4 = get_exact4(prefix4);
		return eam4 ? __rm(&eam4->prefix6, prefix4) : -ESRCH;
	}

	eam6 = get_exact6(prefix6);
	if (!eam6)
		return -ESRCH;
	eam4 = get_exact4(prefix4);
	if (!eam4)
		return -ESRCH;

	return eamt_entry_equals(eam6, eam4) ? __rm(prefix6, prefix4) : -ESRCH;
}

static struct eamt_entry *find_addr6(struct in6_addr *addr)
{
	struct rtrie_key key;

	key.bytes = (__u8 *) addr;
	key.len = 8 * sizeof(*addr);

	return rtrie_get(eamt.tree6, &key);
}

static struct eamt_entry *find_addr4(struct in_addr *addr)
{
	struct rtrie_key key;

	key.bytes = (__u8 *) addr;
	key.len = 8 * sizeof(*addr);

	return rtrie_get(eamt.tree4, &key);
}

bool eamt_contains6(struct in6_addr *addr)
{
	return !!find_addr6(addr);
}

bool eamt_contains4(__u32 addr)
{
	struct in_addr tmp = { .s_addr = addr };
	return !!find_addr4(&tmp);
}

int eamt_xlat_6to4(struct in6_addr *addr6, struct in_addr *result)
{
	struct eamt_entry *eam;
	struct ipv4_prefix prefix4;
	struct ipv6_prefix prefix6;
	unsigned int i;

	/* Find the entry. */
	eam = find_addr6(addr6);
	if (!eam)
		return -ESRCH;
	prefix4 = eam->prefix4;
	prefix6 = eam->prefix6;

//	spin_unlock_bh(&eamt_lock);

	/* Translate the address. */
	for (i = 0; i < ADDR4_BITS - prefix4.len; i++) {
		unsigned int offset4 = prefix4.len + i;
		unsigned int offset6 = prefix6.len + i;
		addr4_set_bit(&prefix4.address, offset4,
				addr6_get_bit(addr6, offset6));
	}

	/* I'm assuming the prefix address is already zero-trimmed. */
	*result = prefix4.address;
	return 0;
}

int eamt_xlat_4to6(struct in_addr *addr4, struct in6_addr *result)
{
	struct ipv4_prefix prefix4;
	struct ipv6_prefix prefix6;
	struct eamt_entry *eam;
	unsigned int suffix4_len;
	unsigned int i;

	if (!addr4) {
		log_err("The IPv4 Address 'addr' can't be NULL");
		return -EINVAL;
	}

	/* Find the entry. */
	eam = find_addr4(addr4);
	if (!eam)
		return -ESRCH;
	prefix4 = eam->prefix4;
	prefix6 = eam->prefix6;

//	spin_unlock_bh(&eamt_lock);

	/* Translate the address. */
	suffix4_len = ADDR4_BITS - prefix4.len;

	for (i = 0; i < suffix4_len; i++) {
		unsigned int offset4 = prefix4.len + i;
		unsigned int offset6 = prefix6.len + i;
		addr6_set_bit(&prefix6.address, offset6,
				addr4_get_bit(addr4, offset4));
	}

	/* I'm assuming the prefix address is already zero-trimmed. */
	*result = prefix6.address;
	return 0;
}

int eamt_count(__u64 *count)
{
	*count = eamt.count;
	return 0;
}

bool eamt_is_empty(void)
{
	__u64 count;
	eamt_count(&count);
	return !count;
}

struct foreach_args {
	int (*cb)(struct eamt_entry *, void *);
	void *arg;
};

static int foreach_cb(void *eam, void *arg)
{
	struct foreach_args *args = arg;
	return args->cb(eam, args->arg);
}

int eamt_foreach(int (*cb)(struct eamt_entry *, void *), void *arg,
		struct ipv4_prefix *offset)
{
	struct foreach_args args = { .cb = cb, .arg = arg };
	struct rtrie_key offset_key;
	struct rtrie_key *offset_key_ptr = NULL;

	args.cb = cb;
	args.arg = arg;

	if (offset) {
		offset_key.bytes = (__u8 *) &offset->address;
		offset_key.len = offset->len;
		offset_key_ptr = &offset_key;
	}

	return rtrie_foreach(eamt.tree6, foreach_cb, &args, offset_key_ptr);
}

void eamt_flush(void)
{
	rtrie_flush(&eamt.tree6);
	rtrie_flush(&eamt.tree4);
	eamt.count = 0;
}

int eamt_init(void)
{
	eamt.tree4 = NULL;
	eamt.tree6 = NULL;
	eamt.count = 0;
	return 0;
}

void eamt_destroy(void)
{
	log_debug("Emptying the Address Mapping table...");
	eamt_flush();
}
