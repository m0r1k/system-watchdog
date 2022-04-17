#include <linux/kernel.h>

#include "system_sysfs.h"

// TODO XXX FIXME OMG it seems to me they are idiots..
// how can I store the private data (engine) here?
// that can be accessed from kobj_attribute :(((
// so temporary HARDCODE :(
static system_watchdog_t *s_engine;

static ssize_t __used mgmt_if_icmp_timeout_sec_wr(
    struct kobject          *a_kobj,
    struct kobj_attribute   *a_attr,
    const char              *a_buf,
    size_t                  a_count)
{
    uint32_t val = 0;
    int32_t  res;
    if (a_count){
        res = kstrtouint(a_buf, 10, &val);
    }
    s_engine->mgmt_if_icmp_timeout_sec      = (uint16_t)val;
    s_engine->mgmt_if_icmp_timeout_left_sec = (uint16_t)val;
    return a_count;
}

static ssize_t __used mgmt_if_down_timeout_sec_wr(
    struct kobject          *a_kobj,
    struct kobj_attribute   *a_attr,
    const char              *a_buf,
    size_t                  a_count)
{
    uint32_t val = 0;
    int32_t  res;
    if (a_count){
        res = kstrtouint(a_buf, 10, &val);
    }
    s_engine->mgmt_if_down_timeout_sec      = (uint16_t)val;
    s_engine->mgmt_if_down_timeout_left_sec = (uint16_t)val;
    return a_count;
}

static ssize_t __used mgmt_if_idle_timeout_sec_wr(
    struct kobject          *a_kobj,
    struct kobj_attribute   *a_attr,
    const char              *a_buf,
    size_t                  a_count)
{
    uint32_t val = 0;
    int32_t  res;
    if (a_count){
        res = kstrtouint(a_buf, 10, &val);
    }
    s_engine->mgmt_if_idle_timeout_sec      = (uint16_t)val;
    s_engine->mgmt_if_idle_timeout_left_sec = (uint16_t)val;
    return a_count;
}

// rigister read/write handlers

static struct kobj_attribute mgmt_if_icmp_timeout_sec_attr = __ATTR(
    mgmt_if_icmp_timeout_sec, 0220, NULL, mgmt_if_icmp_timeout_sec_wr
);

static struct kobj_attribute mgmt_if_down_timeout_sec_attr = __ATTR(
    mgmt_if_down_timeout_sec, 0220, NULL, mgmt_if_down_timeout_sec_wr
);

static struct kobj_attribute mgmt_if_idle_timeout_sec_attr = __ATTR(
    mgmt_if_idle_timeout_sec, 0220, NULL, mgmt_if_idle_timeout_sec_wr
);

// put attributes to the attribute group
static struct attribute *register_attrs[] = {
    &mgmt_if_icmp_timeout_sec_attr.attr,
    &mgmt_if_down_timeout_sec_attr.attr,
    &mgmt_if_idle_timeout_sec_attr.attr,
    NULL,   // NULL terminate the list
};

static struct attribute_group  reg_attr_group = {
    .attrs = register_attrs
};

int32_t init_sysfs(
    system_watchdog_t *a_engine)
{
    int32_t res, err = -1;

    // TODO XXX FIXME OMG it seems to me they are idiots..
    // how can I store the private data (engine) here?
    // that can be accessed from kobj_attribute :(((
    // so temporary HARDCODE :(
    s_engine = a_engine;

    // create root object
    a_engine->root_kobject = kobject_create_and_add(
        "system_watchdog",
        kernel_kobj
    );
    if (!a_engine->root_kobject){
        err = -ENOMEM;
        goto fail;
    }

    // create attributes (files)
    res = sysfs_create_group(
        a_engine->root_kobject,
        &reg_attr_group
    );
    if (res){
        err = -ENOMEM;
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    if (a_engine->root_kobject){
        kobject_put(a_engine->root_kobject);
        a_engine->root_kobject = NULL;
    }
    goto out;
}

void cleanup_sysfs(
    system_watchdog_t *a_engine)
{
    if (a_engine->root_kobject){
        kobject_put(a_engine->root_kobject);
        a_engine->root_kobject = NULL;
    }
}

