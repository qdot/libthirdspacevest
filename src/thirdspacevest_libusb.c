/*
 * Third Space Vest Driver - libusb comms
 *
 * Copyright (c) 2010 Kyle Machulis/Nonpolynomial Labs <kyle@nonpolynomial.com>
 *
 * More info on Nonpolynomial Labs @ http://www.nonpolynomial.com
 *
 * Source code available at http://www.github.com/qdot/libthirdspacevest/
 *
 * This library is covered by the BSD License
 * Read LICENSE_BSD.txt for details.
 */


#include "thirdspacevest/thirdspacevest.h"
#include <stdlib.h>

#define THIRDSPACEVEST_USB_INTERFACE	0

thirdspacevest_device* thirdspacevest_create()
{
	thirdspacevest_device* s = (thirdspacevest_device*)malloc(sizeof(thirdspacevest_device));
	s->_is_open = 0;
	s->_is_inited = 0;
	if(libusb_init(&s->_context) < 0)
	{
		return NULL;
	}
	s->_is_inited = 1;
	return s;
}

int thirdspacevest_get_count(thirdspacevest_device* s)
{
	struct libusb_device **devs;
	struct libusb_device *found = NULL;
	struct libusb_device *dev;
	size_t i = 0;
	int count = 0;

	if (!s->_is_inited)
	{
		return E_NPUTIL_NOT_INITED;
	}

	if (libusb_get_device_list(s->_context, &devs) < 0)
	{
		return E_NPUTIL_DRIVER_ERROR;
	}

	while ((dev = devs[i++]) != NULL)
	{
		struct libusb_device_descriptor desc;
		int dev_error_code;
		dev_error_code = libusb_get_device_descriptor(dev, &desc);
		if (dev_error_code < 0)
		{
			break;
		}
		if (desc.idVendor == THIRDSPACEVEST_VID && desc.idProduct == THIRDSPACEVEST_PID)
		{
			++count;
		}
	}

	libusb_free_device_list(devs, 1);
	return count;
}

int thirdspacevest_open(thirdspacevest_device* s, unsigned int device_index)
{
	int ret;
	struct libusb_device **devs;
	struct libusb_device *found = NULL;
	struct libusb_device *dev;
	size_t i = 0;
	int count = 0;
	int device_error_code = 0;

	if (!s->_is_inited)
	{
		return E_NPUTIL_NOT_INITED;
	}

	if ((device_error_code = libusb_get_device_list(s->_context, &devs)) < 0)
	{
		return E_NPUTIL_DRIVER_ERROR;
	}

	while ((dev = devs[i++]) != NULL)
	{
		struct libusb_device_descriptor desc;
		device_error_code = libusb_get_device_descriptor(dev, &desc);
		if (device_error_code < 0)
		{
			libusb_free_device_list(devs, 1);
			return E_NPUTIL_NOT_INITED;
		}
		if (desc.idVendor == THIRDSPACEVEST_VID && desc.idProduct == THIRDSPACEVEST_PID)
		{
			if(count == device_index)
			{
				found = dev;
				break;
			}
			++count;
		}
	}

	if (found)
	{
		device_error_code = libusb_open(found, &s->_device);
		if (device_error_code < 0)
		{
			libusb_free_device_list(devs, 1);
			return E_NPUTIL_NOT_INITED;
		}
	}
	else
	{
		return E_NPUTIL_NOT_INITED;
	}
	s->_is_open = 1;

	if(libusb_kernel_driver_active(s->_device, 0))
	{
		libusb_detach_kernel_driver(s->_device, 0);
	}
	ret = libusb_claim_interface(s->_device, 0);

	return ret;
}

int thirdspacevest_close(thirdspacevest_device* s)
{
	if(!s->_is_open)
	{
		return E_NPUTIL_NOT_OPENED;
	}
	if (libusb_release_interface(s->_device, 0) < 0)
	{
		return E_NPUTIL_NOT_INITED;
	}
	libusb_close(s->_device);
	s->_is_open = 0;
	return 0;
}

void thirdspacevest_delete(thirdspacevest_device* dev)
{
	free(dev);
}

int thirdspacevest_read_data(thirdspacevest_device* dev, uint8_t* input_report)
{
	int trans;
	int ret = libusb_bulk_transfer(dev->_device, THIRDSPACEVEST_IN_ENDPT, input_report, 10, &trans, 100);
	return ret;
}

int thirdspacevest_write_data(thirdspacevest_device* dev, uint8_t* output_report)
{
	int trans;
	return libusb_bulk_transfer(dev->_device, THIRDSPACEVEST_OUT_ENDPT, output_report, 10, &trans, 100);
}

