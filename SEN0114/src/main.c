#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#include "inc/AnalogIn.h"
#include <zephyr/drivers/adc.h>
#include <string.h>
#include <stdio.h>

// Simple analog input method
// this just reads a sample then waits then returns it

// ADC Sampling Settings
// doc says that impedance of 800K == 40usec sample time

//   # the sensor value description
//   # 0  ~300     dry soil
//   # 300~700     humid soil
//   # 700~950     in water

#define ADC_RESOLUTION		14
#define ADC_GAIN			ADC_GAIN_1_6
#define ADC_REFERENCE		ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME	ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40)
#define BUFFER_SIZE			6

static bool _IsInitialized = false;
static uint8_t _LastChannel = 250;
static int16_t m_sample_buffer[BUFFER_SIZE];

// the channel configuration with channel not yet filled in
static struct adc_channel_cfg m_1st_channel_cfg = {
	.gain             = ADC_GAIN,
	.reference        = ADC_REFERENCE,
	.acquisition_time = ADC_ACQUISITION_TIME,
	.channel_id       = 0, // gets set during init
	.differential	  = 0,
#if CONFIG_ADC_CONFIGURABLE_INPUTS
	.input_positive   = 0, // gets set during init
#endif
};



// initialize the adc channel
static const struct device* init_adc(int channel)
{
	int ret;
	
	// Define adc device 
	const struct device *adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));

    if (adc_dev == NULL || !device_is_ready(adc_dev)) {
		printk("INFO: ADC device is not found.\n");
	}
	else{
		printk("INFO: ADC device connected.\n");
	}

	if(_LastChannel != channel)
	{
		_IsInitialized = false;
		_LastChannel = channel;
	}

	if ( adc_dev != NULL && !_IsInitialized)
	{
		// strangely channel_id gets the channel id and input_positive gets id+1
		m_1st_channel_cfg.channel_id = channel;
#if CONFIG_ADC_CONFIGURABLE_INPUTS
        m_1st_channel_cfg.input_positive = channel+1,
#endif
		ret = adc_channel_setup(adc_dev, &m_1st_channel_cfg);
		if(ret != 0)
		{
			//LOG_INF("Setting up of the first channel failed with code %d", ret);
			adc_dev = NULL;
		}
		else
		{
			_IsInitialized = true;	// we don't have any other analog users
		}
	}
	
	memset(m_sample_buffer, 0, sizeof(m_sample_buffer));
	return adc_dev;
}

// ------------------------------------------------
// read one channel of adc
// ------------------------------------------------
static int16_t readOneChannel(int channel)
{
	const struct adc_sequence sequence = {
		.options     = NULL,				// extra samples and callback
		.channels    = BIT(channel),		// bit mask of channels to read
		.buffer      = m_sample_buffer,		// where to put samples read
		.buffer_size = sizeof(m_sample_buffer),
		.resolution  = ADC_RESOLUTION,		// desired resolution
		.oversampling = 0,					// don't oversample
		.calibrate = 0						// don't calibrate
	};

	int ret;
	int16_t sample_value = BAD_ANALOG_READ;
	const struct device *adc_dev = init_adc(channel);
	if (adc_dev)
	{
		ret = adc_read(adc_dev, &sequence);
		if(ret == 0)
		{
			sample_value = m_sample_buffer[0];
		}
	}

	return sample_value;
}

// ------------------------------------------------
// high level read adc channel and convert to float voltage
// ------------------------------------------------
float AnalogRead(int channel)
{

	int16_t sv = readOneChannel(channel);
	if(sv == BAD_ANALOG_READ)
	{
		return sv;
	}

	// Convert the result to voltage
	// Result = [V(p) - V(n)] * GAIN/REFERENCE / 2^(RESOLUTION)
																				  
	int multip = 256;
	// find 2**adc_resolution
	switch(ADC_RESOLUTION)
				
	{
		default :
		case 8 :
			multip = 256;
			break;
		case 10 :
			multip = 1024;
			break;
		case 12 :
			multip = 4096;
			break;
		case 14 :
			multip = 16384;
			break;
	}
	
	float fout = (sv * 3.3 / multip);
	return fout;
}

int main(){

    float m_value;

	while (1)
	{
        k_msleep(3000);
		m_value = AnalogRead(0);
		printf("m value : %.8f\n", m_value);
	}

	return 0;
}
