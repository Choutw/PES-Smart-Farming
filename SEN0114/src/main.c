/*
 * Copyright (c) 2020 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

/*
	# the sensor value description
	# 0  ~300     dry soil
	# 300~700     humid soil
	# 700~950     in water
*/

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};


void main(void)
{
	int err;
	uint16_t buf;

	struct adc_sequence sequence = {

		.buffer = &buf,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(buf),

	};

	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {

		if (!device_is_ready(adc_channels[i].dev)) {

			printk("ADC controller device not ready\n");
			return;

		}

		err = adc_channel_setup_dt(&adc_channels[i]);

		if (err < 0) {

			printk("Could not setup channel #%d (%d)\n", i, err);
			return;

		}
	}

	while (1) {

		printk("ADC reading:\n");

		for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
			
			int32_t val_mv;
			int32_t sen_value;

			printk("INFO: - %s, channel %d\n",
			       adc_channels[i].dev->name,
			       adc_channels[i].channel_id);

			(void)adc_sequence_init_dt(&adc_channels[i], &sequence);

			err = adc_read(adc_channels[i].dev, &sequence);

			if (err < 0) {

				printk("Could not read (%d)\n", err);
				continue;

			}

			/*
			 * If using differential mode, the 16 bit value
			 * in the ADC sample buffer should be a signed 2's
			 * complement value.
			 */

			if (adc_channels[i].channel_cfg.differential) {
				val_mv = (int32_t)((int16_t)buf);
			} else {
				val_mv = (int32_t)buf;
			}
			
			// transform value
			sen_value = (val_mv/3.3);

			// printk("%"PRId32, val_mv);

			err = adc_raw_to_millivolts_dt(&adc_channels[i],
						       &val_mv);

			/* conversion to mV may not be supported, skip if not */
			if (err < 0) {

				printk(" (value in mV not available)\n");

			} else {

				printk("Raw data: %"PRId32" (mV)\n", val_mv);
				printk("SEN0114 value: %"PRId32"\n", sen_value);

			}

			printk("------------------------------------\n");

		}

		k_sleep(K_MSEC(1000));
	}
}