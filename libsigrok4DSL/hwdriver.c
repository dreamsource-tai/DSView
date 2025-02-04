/*
 * This file is part of the libsigrok project.
 *
 * Copyright (C) 2013 Bert Vermeulen <bert@biot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "libsigrok.h"
#include "libsigrok-internal.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <glib.h>
#include "config.h" /* Needed for HAVE_LIBUSB_1_0 and others. */

#include "hardware/DSL/dsl.h"

/* Message logging helpers with subsystem-specific prefix string. */
#define LOG_PREFIX "hwdriver: "
#define sr_log(l, s, args...) sr_log(l, LOG_PREFIX s, ## args)
#define sr_spew(s, args...) sr_spew(LOG_PREFIX s, ## args)
#define sr_dbg(s, args...) sr_dbg(LOG_PREFIX s, ## args)
#define sr_info(s, args...) sr_info(LOG_PREFIX s, ## args)
#define sr_warn(s, args...) sr_warn(LOG_PREFIX s, ## args)
#define sr_err(s, args...) sr_err(LOG_PREFIX s, ## args)

/**
 * @file
 *
 * Hardware driver handling in libsigrok.
 */

/**
 * @defgroup grp_driver Hardware drivers
 *
 * Hardware driver handling in libsigrok.
 *
 * @{
 */
static struct sr_config_info sr_config_info_data[] = {
    {SR_CONF_CONN, SR_T_CHAR, "conn",
        "Connection", "Connection", "连接", NULL},
	{SR_CONF_SERIALCOMM, SR_T_CHAR, "serialcomm",
        "Serial communication", "Serial communication", "串口通讯", NULL},
	{SR_CONF_SAMPLERATE, SR_T_UINT64, "samplerate",
        "Sample rate", "Sample rate", "采样率", NULL},
    {SR_CONF_LIMIT_SAMPLES, SR_T_UINT64, "samplecount",
        "Sample count", "Sample count", "采样深度", NULL},
    {SR_CONF_ACTUAL_SAMPLES, SR_T_UINT64, "samplecount",
        "Sample count", "Sample count", "实际采样数", NULL},
    {SR_CONF_CLOCK_TYPE, SR_T_BOOL, "clocktype",
        "Using External Clock", "Using External Clock", "使用外部输入时钟采样", NULL},
    {SR_CONF_CLOCK_EDGE, SR_T_BOOL, "clockedge",
        "Using Clock Negedge", "Using Clock Negedge", "使用时钟下降沿采样", NULL},
    {SR_CONF_CAPTURE_RATIO, SR_T_UINT64, "captureratio",
        "Pre-trigger capture ratio", "Pre-trigger capture ratio", "触发前采样比例", NULL},
    {SR_CONF_PATTERN_MODE, SR_T_CHAR, "pattern",
        "Pattern mode", "Pattern mode", "信号模式", NULL},
	{SR_CONF_RLE, SR_T_BOOL, "rle",
        "Run Length Encoding", "Run Length Encoding", "RLE编码", NULL},
    {SR_CONF_WAIT_UPLOAD, SR_T_BOOL, "buf_upload",
        "Wait Buffer Upload", "Wait Buffer Upload", "上传已采集数据", NULL},
    {SR_CONF_TRIGGER_SLOPE, SR_T_UINT8, "triggerslope",
        "Trigger slope", "Trigger slope", "触发沿", NULL},
    {SR_CONF_TRIGGER_SOURCE, SR_T_UINT8, "triggersource",
        "Trigger source", "Trigger source", "触发源", NULL},
    {SR_CONF_TRIGGER_CHANNEL, SR_T_UINT8, "triggerchannel",
        "Trigger channel", "Trigger channel", "触发通道", NULL},
    {SR_CONF_HORIZ_TRIGGERPOS, SR_T_UINT8, "horiz_triggerpos",
        "Horizontal trigger position", "Horizontal trigger position", "触发位置", NULL},
    {SR_CONF_TRIGGER_HOLDOFF, SR_T_UINT64, "triggerholdoff",
        "Trigger hold off", "Trigger hold off", "触发释抑时间", NULL},
    {SR_CONF_TRIGGER_MARGIN, SR_T_UINT8, "triggermargin",
    "Trigger margin", "Trigger margin", "触发灵敏度", NULL},
    {SR_CONF_BUFFERSIZE, SR_T_UINT64, "buffersize",
        "Buffer size", "Buffer size", "缓存大小", NULL},
    {SR_CONF_TIMEBASE, SR_T_UINT64, "timebase",
        "Time base", "Time base", "时基", NULL},
    {SR_CONF_MAX_HEIGHT, SR_T_CHAR, "height",
        "Max Height", "Max Height", "最大高度", NULL},
    {SR_CONF_MAX_HEIGHT_VALUE, SR_T_UINT8, "height",
        "Max Height", "Max Height", "最大高度值", NULL},
	{SR_CONF_FILTER, SR_T_CHAR, "filter",
        "Filter Targets", "Filter Targets", "滤波器设置", NULL},
	{SR_CONF_DATALOG, SR_T_BOOL, "datalog",
        "Datalog", "Datalog", "数据记录", NULL},
    {SR_CONF_OPERATION_MODE, SR_T_CHAR, "operation",
        "Operation Mode", "Operation Mode", "运行模式", NULL},
    {SR_CONF_BUFFER_OPTIONS, SR_T_CHAR, "stopoptions",
        "Stop Options", "Stop Options", "停止选项", NULL},
    {SR_CONF_CHANNEL_MODE, SR_T_CHAR, "channel",
        "Channel Mode", "Channel Mode", "通道模式", NULL},
    {SR_CONF_THRESHOLD, SR_T_CHAR, "threshold",
        "Threshold Level", "Threshold Level", "阈值电压", NULL},
    {SR_CONF_VTH, SR_T_FLOAT, "threshold",
        "Threshold Level", "Threshold Level", "阈值电压", NULL},
    {SR_CONF_RLE_SUPPORT, SR_T_BOOL, "rle",
        "Enable RLE Compress", "Enable RLE Compress", "RLE硬件压缩", NULL},
    {SR_CONF_BANDWIDTH_LIMIT, SR_T_CHAR, "bandwidth",
        "Bandwidth Limit", "Bandwidth Limit", "带宽限制", NULL},

    {SR_CONF_PROBE_COUPLING, SR_T_CHAR, "coupling",
        "Coupling", "Coupling", "耦合", NULL},
    {SR_CONF_PROBE_VDIV, SR_T_RATIONAL_VOLT, "vdiv",
        "Volts/div", "Volts/div", "电压/格", NULL},
    {SR_CONF_PROBE_FACTOR, SR_T_UINT64, "factor",
        "Probe Factor", "Probe Factor", "探头衰减", NULL},
    {SR_CONF_PROBE_MAP_DEFAULT, SR_T_BOOL, "mdefault",
        "Map Default", "Map Default", "默认电压", NULL},
    {SR_CONF_PROBE_MAP_UNIT, SR_T_CHAR, "munit",
        "Map Unit", "Map Unit", "对应单位", NULL},
    {SR_CONF_PROBE_MAP_MIN, SR_T_FLOAT, "MMIN",
        "Map Min", "Map Min", "对应最小值", NULL},
    {SR_CONF_PROBE_MAP_MAX, SR_T_FLOAT, "MMAX",
        "Map Max", "Map Max", "对应最大值", NULL},
    {0, 0, NULL, NULL, NULL, NULL, NULL},
};

/** @cond PRIVATE */
#ifdef HAVE_LA_DEMO
extern SR_PRIV struct sr_dev_driver demo_driver_info;
#endif
#ifdef HAVE_DSL_DEVICE
extern SR_PRIV struct sr_dev_driver DSLogic_driver_info;
extern SR_PRIV struct sr_dev_driver DSCope_driver_info;
#endif
/** @endcond */

static struct sr_dev_driver *drivers_list[] = {
#ifdef HAVE_LA_DEMO
	&demo_driver_info,
#endif
#ifdef HAVE_DSL_DEVICE
    &DSLogic_driver_info,
    &DSCope_driver_info,
#endif
	NULL,
};

/**
 * Return the list of supported hardware drivers.
 *
 * @return Pointer to the NULL-terminated list of hardware driver pointers.
 */
SR_API struct sr_dev_driver **sr_driver_list(void)
{

	return drivers_list;
}

/**
 * Initialize a hardware driver.
 *
 * This usually involves memory allocations and variable initializations
 * within the driver, but _not_ scanning for attached devices.
 * The API call sr_driver_scan() is used for that.
 *
 * @param ctx A libsigrok context object allocated by a previous call to
 *            sr_init(). Must not be NULL.
 * @param driver The driver to initialize. This must be a pointer to one of
 *               the entries returned by sr_driver_list(). Must not be NULL.
 *
 * @return SR_OK upon success, SR_ERR_ARG upon invalid parameters,
 *         SR_ERR_BUG upon internal errors, or another negative error code
 *         upon other errors.
 */
SR_API int sr_driver_init(struct sr_context *ctx, struct sr_dev_driver *driver)
{
	int ret;

	if (!ctx) {
		sr_err("Invalid libsigrok context, can't initialize.");
		return SR_ERR_ARG;
	}

	if (!driver) {
		sr_err("Invalid driver, can't initialize.");
		return SR_ERR_ARG;
	}

	sr_spew("Initializing driver '%s'.", driver->name);
	if ((ret = driver->init(ctx)) < 0)
		sr_err("Failed to initialize the driver: %d.", ret);

	return ret;
}

/**
 * Tell a hardware driver to scan for devices.
 *
 * In addition to the detection, the devices that are found are also
 * initialized automatically. On some devices, this involves a firmware upload,
 * or other such measures.
 *
 * The order in which the system is scanned for devices is not specified. The
 * caller should not assume or rely on any specific order.
 *
 * Before calling sr_driver_scan(), the user must have previously initialized
 * the driver by calling sr_driver_init().
 *
 * @param driver The driver that should scan. This must be a pointer to one of
 *               the entries returned by sr_driver_list(). Must not be NULL.
 * @param options A list of 'struct sr_hwopt' options to pass to the driver's
 *                scanner. Can be NULL/empty.
 *
 * @return A GSList * of 'struct sr_dev_inst', or NULL if no devices were
 *         found (or errors were encountered). This list must be freed by the
 *         caller using g_slist_free(), but without freeing the data pointed
 *         to in the list.
 */
SR_API GSList *sr_driver_scan(struct sr_dev_driver *driver, GSList *options)
{
	GSList *l;

	if (!driver) {
		sr_err("Invalid driver, can't scan for devices.");
		return NULL;
	}

	if (!driver->priv) {
		sr_err("Driver not initialized, can't scan for devices.");
		return NULL;
	}

	l = driver->scan(options);

	sr_spew("Scan of '%s' found %d devices.", driver->name,
		g_slist_length(l));

	return l;
}

/** @private */
SR_PRIV void sr_hw_cleanup_all(void)
{
	int i;
	struct sr_dev_driver **drivers;

	drivers = sr_driver_list();
	for (i = 0; drivers[i]; i++) {
		if (drivers[i]->cleanup)
			drivers[i]->cleanup();
	}
}

/** A floating reference can be passed in for data. */
SR_API struct sr_config *sr_config_new(int key, GVariant *data)
{
	struct sr_config *src;

	if (!(src = g_try_malloc(sizeof(struct sr_config))))
		return NULL;
	src->key = key;
	src->data = g_variant_ref_sink(data);

	return src;
}

SR_API void sr_config_free(struct sr_config *src)
{

	if (!src || !src->data) {
		sr_err("%s: invalid data!", __func__);
		return;
	}

	g_variant_unref(src->data);
	g_free(src);

}

/**
 * Returns information about the given driver or device instance.
 *
 * @param driver The sr_dev_driver struct to query.
 * @param key The configuration key (SR_CONF_*).
 * @param data Pointer to a GVariant where the value will be stored. Must
 *             not be NULL. The caller is given ownership of the GVariant
 *             and must thus decrease the refcount after use. However if
 *             this function returns an error code, the field should be
 *             considered unused, and should not be unreferenced.
 * @param sdi (optional) If the key is specific to a device, this must
 *            contain a pointer to the struct sr_dev_inst to be checked.
 *            Otherwise it must be NULL.
 *
 * @return SR_OK upon success or SR_ERR in case of error. Note SR_ERR_ARG
 *         may be returned by the driver indicating it doesn't know that key,
 *         but this is not to be flagged as an error by the caller; merely
 *         as an indication that it's not applicable.
 */
SR_API int sr_config_get(const struct sr_dev_driver *driver,
                         const struct sr_dev_inst *sdi,
                         const struct sr_channel *ch,
                         const struct sr_channel_group *cg,
                         int key, GVariant **data)
{
	int ret;

	if (!driver || !data)
		return SR_ERR;

	if (!driver->config_get)
		return SR_ERR_ARG;

    if ((ret = driver->config_get(key, data, sdi, ch, cg)) == SR_OK) {
		/* Got a floating reference from the driver. Sink it here,
		 * caller will need to unref when done with it. */
		g_variant_ref_sink(*data);
	}

	return ret;
}

/**
 * Set a configuration key in a device instance.
 *
 * @param sdi The device instance.
 * @param key The configuration key (SR_CONF_*).
 * @param data The new value for the key, as a GVariant with GVariantType
 *        appropriate to that key. A floating reference can be passed
 *        in; its refcount will be sunk and unreferenced after use.
 *
 * @return SR_OK upon success or SR_ERR in case of error. Note SR_ERR_ARG
 *         may be returned by the driver indicating it doesn't know that key,
 *         but this is not to be flagged as an error by the caller; merely
 *         as an indication that it's not applicable.
 */
SR_API int sr_config_set(struct sr_dev_inst *sdi,
                         struct sr_channel *ch,
                         struct sr_channel_group *cg,
                         int key, GVariant *data)
{
	int ret;

	g_variant_ref_sink(data);

	if (!sdi || !sdi->driver || !data)
		ret = SR_ERR;
	else if (!sdi->driver->config_set)
		ret = SR_ERR_ARG;
	else
        ret = sdi->driver->config_set(key, data, sdi, ch, cg);

	g_variant_unref(data);

	return ret;
}

/**
 * List all possible values for a configuration key.
 *
 * @param driver The sr_dev_driver struct to query.
 * @param key The configuration key (SR_CONF_*).
 * @param data A pointer to a GVariant where the list will be stored. The
 *             caller is given ownership of the GVariant and must thus
 *             unref the GVariant after use. However if this function
 *             returns an error code, the field should be considered
 *             unused, and should not be unreferenced.
 * @param sdi (optional) If the key is specific to a device, this must
 *            contain a pointer to the struct sr_dev_inst to be checked.
 *
 * @return SR_OK upon success or SR_ERR in case of error. Note SR_ERR_ARG
 *         may be returned by the driver indicating it doesn't know that key,
 *         but this is not to be flagged as an error by the caller; merely
 *         as an indication that it's not applicable.
 */
SR_API int sr_config_list(const struct sr_dev_driver *driver,
                          const struct sr_dev_inst *sdi,
                          const struct sr_channel_group *cg,
                          int key, GVariant **data)
{
	int ret;

	if (!driver || !data)
		ret = SR_ERR;
	else if (!driver->config_list)
		ret = SR_ERR_ARG;
    else if ((ret = driver->config_list(key, data, sdi, cg)) == SR_OK)
		g_variant_ref_sink(*data);

	return ret;
}

/**
 * Get information about a configuration key.
 *
 * @param key The configuration key.
 *
 * @return A pointer to a struct sr_config_info, or NULL if the key
 *         was not found.
 */
SR_API const struct sr_config_info *sr_config_info_get(int key)
{
	int i;

	for (i = 0; sr_config_info_data[i].key; i++) {
		if (sr_config_info_data[i].key == key)
			return &sr_config_info_data[i];
	}

	return NULL;
}

/**
 * Get status about an acquisition
 *
 * @param sdi The device instance.
 * @param status A pointer to a struct sr_capture_status.
 *
 * @return SR_OK upon success or SR_ERR in case of error. Note SR_ERR_ARG
 *         may be returned by the driver indicating it doesn't know that key,
 *         but this is not to be flagged as an error by the caller; merely
 *         as an indication that it's not applicable.
 */
SR_API int sr_status_get(const struct sr_dev_inst *sdi,
                         struct sr_status *status, gboolean prg)
{
    int ret;

    if (!sdi->driver)
        ret = SR_ERR;
    else if (!sdi->driver->dev_status_get)
        ret = SR_ERR_ARG;
    else
        ret = sdi->driver->dev_status_get(sdi, status, prg);

    return ret;
}

/**
 * Get status about an acquisition.
 *
 * @param optname The configuration key.
 *
 * @return A pointer to a struct sr_config_info, or NULL if the key
 *         was not found.
 */
SR_API const struct sr_config_info *sr_config_info_name_get(const char *optname)
{
    int i;

    for (i = 0; sr_config_info_data[i].key; i++) {
        if (!strcmp(sr_config_info_data[i].id, optname))
            return &sr_config_info_data[i];
    }

    return NULL;
}

/* Unnecessary level of indirection follows. */

/** @private */
SR_PRIV int sr_source_remove(int fd)
{
	return sr_session_source_remove(fd);
}

/** @private */
SR_PRIV int sr_source_add(int fd, int events, int timeout,
			  sr_receive_data_callback_t cb, void *cb_data)
{
    return sr_session_source_add(fd, events, timeout, cb, cb_data);
}

/** @} */

/*
test usb device api
*/
SR_API void sr_test_usb_api()
{
    libusb_context *ctx;
    struct libusb_device_descriptor des;
    int usb_speed;
    int ret;
    int i;
    int num_devs;
    libusb_device **devlist;
    int stdnum = 0;
    int j;
    int bfind = 0;
    int dlsnum = 0;
    struct libusb_device_handle *devhandle;

    printf("\n");

    ret = libusb_init(&ctx);
    if (ret) {
	printf("unable to initialize libusb: %i\n", ret);
    return;	 
   }
  
    num_devs = libusb_get_device_list(ctx, &devlist);
    printf("usb dev num:%d\n", num_devs);

    for (i=0; i<num_devs; i++){
        libusb_get_device_descriptor(devlist[i], &des);

        usb_speed = libusb_get_device_speed( devlist[i]);

        if ((usb_speed != LIBUSB_SPEED_HIGH) && (usb_speed != LIBUSB_SPEED_SUPER))
            continue;
        stdnum++;
        bfind = 0;

        for (j = 0; supported_DSLogic[j].vid; j++) {
            if (des.idVendor == supported_DSLogic[j].vid &&
                des.idProduct == supported_DSLogic[j].pid &&
                usb_speed == supported_DSLogic[j].usb_speed) {
                bfind = 1;
                break;
			}
		}

        if (bfind){
            dlsnum++;

            devhandle = NULL;
            ret = libusb_open(devlist[i], &devhandle);
            if (ret){
                printf("open device error!%s\n", libusb_error_name(ret));
            }
            else{
                //printf("dev open success\n");
                  ret = libusb_claim_interface(devhandle, USB_INTERFACE);
                  if (ret){
                         printf("Unable to claim interface: %s\n", libusb_error_name(ret));
                  }

                libusb_close(devhandle);
            }
        }
    }

    printf("std usb dev num:%d\n", stdnum);
    printf("dls dev num:%d\n", dlsnum);

    libusb_free_device_list(devlist, 0);

   libusb_exit(NULL);
}
