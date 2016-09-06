#include "nat64/mod/stateless/blacklist4.h"

#include <linux/rculist.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>

#include "nat64/common/str_utils.h"
#include "nat64/mod/common/namespace.h"
#include "nat64/mod/common/rcu.h"
#include "nat64/mod/stateless/pool.h"

static struct list_head __rcu *pool;

int blacklist_init(char *pref_strs[], int pref_count)
{
	return pool_init(&pool, pref_strs, pref_count);
}

void blacklist_destroy(void)
{
	pool_destroy(&pool);
}

int blacklist_add(struct ipv4_prefix *prefix)
{
	return pool_add(pool, prefix);
}

int blacklist_rm(struct ipv4_prefix *prefix)
{
	return pool_rm(pool, prefix);
}

int blacklist_flush(void)
{
	return pool_flush(&pool);
}

/**
 * Is @addr present in one of @ns's interfaces?
 * Will also return true of @addr is the broadcast address of one of @ns's
 * interfaces.
 */
bool interface_contains(struct in_addr *addr)
{
	struct net_device *dev;
	struct in_device *in_dev;
	struct in_ifaddr *ifa;
	struct in_addr ifaddr;

	rcu_read_lock();
	for_each_netdev_rcu(joolns_get(), dev) {
		in_dev = rcu_dereference(dev->ip_ptr);
		ifa = in_dev->ifa_list;
		while (ifa) {
			ifaddr.s_addr = ifa->ifa_local;
			if (ipv4_addr_cmp(&ifaddr, addr) == 0)
				goto found;

			ifaddr.s_addr = ifa->ifa_local | ~ifa->ifa_mask;
			if (ipv4_addr_cmp(&ifaddr, addr) == 0)
				goto found;

			ifa = ifa->ifa_next;
		}
	}
	rcu_read_unlock();

	return false;

found:
	rcu_read_unlock();
	return true;
}

bool blacklist_contains(struct in_addr *addr)
{
	return pool_contains(pool, addr);
}

int blacklist_for_each(int (*func)(struct ipv4_prefix *, void *), void *arg,
		struct ipv4_prefix *offset)
{
	return pool_foreach(pool, func, arg, offset);
}

int blacklist_count(__u64 *result)
{
	return pool_count(pool, result);
}

bool blacklist_is_empty(void)
{
	return pool_is_empty(pool);
}
