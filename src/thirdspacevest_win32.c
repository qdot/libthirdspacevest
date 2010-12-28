/*
 * Third Space Vest Driver - Windows WDK comms
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

#include <api/setupapi.h>
#include <api/hidsdi.h>
#include <stdlib.h>
#include <stdio.h>

//Application global variables
DWORD								ActualBytesRead;
DWORD								BytesRead;
HIDP_CAPS							Capabilities;
DWORD								cbBytesRead;
PSP_DEVICE_INTERFACE_DETAIL_DATA	detailData;
DWORD								dwError;
char								FeatureReport[256];
HANDLE								hEventObject;
HANDLE								hDevInfo;
GUID								HidGuid;
OVERLAPPED							HIDOverlapped;
char								InputReport[256];
ULONG								Length;
LPOVERLAPPED						lpOverLap;
BOOL								MyDeviceDetected = FALSE;
TCHAR*								MyDevicePathName;
DWORD								NumberOfBytesRead;
char								OutputReport[256];
DWORD								ReportType;
ULONG								Required;
TCHAR*								ValueToDisplay;

void GetDeviceCapabilities(HANDLE DeviceHandle)
{
	//Get the Capabilities structure for the 

	PHIDP_PREPARSED_DATA	PreparsedData;

	/*
	  API function: HidD_GetPreparsedData
	  Returns: a pointer to a buffer containing the information about the device's capabilities.
	  Requires: A handle returned by CreateFile.
	  There's no need to access the buffer directly,
	  but HidP_GetCaps and other API functions require a pointer to the buffer.
	*/

	HidD_GetPreparsedData
		(DeviceHandle,
		 &PreparsedData);

	/*
	  API function: HidP_GetCaps
	  Learn the device's capabilities.
	  For standard devices such as joysticks, you can find out the specific
	  capabilities of the 
	  For a custom device, the software will probably know what the device is capable of,
	  and the call only verifies the information.
	  Requires: the pointer to the buffer returned by HidD_GetPreparsedData.
	  Returns: a Capabilities structure containing the information.
	*/

	HidP_GetCaps
		(PreparsedData,
		 &Capabilities);

	HidD_FreePreparsedData(PreparsedData);
}

int thirdspacevest_open_win32(thirdspacevest_device* dev, unsigned int device_index, int get_count)
{
	//Use a series of API calls to find a HID with a specified Vendor IF and Product ID.

	HIDD_ATTRIBUTES						Attributes;
	SP_DEVICE_INTERFACE_DATA			devInfoData;
	BOOL								LastDevice = FALSE;
	int									MemberIndex = 0;
	LONG								Result;
	int									device_count = 0;

	Length = 0;
	detailData = NULL;
	dev->_dev = NULL;

	/*
	  API function: HidD_GetHidGuid
	  Get the GUID for all system HIDs.
	  Returns: the GUID in HidGuid.
	*/

	HidD_GetHidGuid(&HidGuid);

	/*
	  API function: SetupDiGetClassDevs
	  Returns: a handle to a device information set for all installed devices.
	  Requires: the GUID returned by GetHidGuid.
	*/

	hDevInfo=SetupDiGetClassDevs
		(&HidGuid,
		 NULL,
		 NULL,
		 DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);

	devInfoData.cbSize = sizeof(devInfoData);

	//Step through the available devices looking for the one we want.
	//Quit on detecting the desired device or checking all available devices without success.

	MemberIndex = 0;
	LastDevice = FALSE;

	do
	{
		/*
		  API function: SetupDiEnumDeviceInterfaces
		  On return, MyDeviceInterfaceData contains the handle to a
		  SP_DEVICE_INTERFACE_DATA structure for a detected 
		  Requires:
		  The DeviceInfoSet returned in SetupDiGetClassDevs.
		  The HidGuid returned in GetHidGuid.
		  An index to specify a 
		*/

		Result=SetupDiEnumDeviceInterfaces
			(hDevInfo,
			 0,
			 &HidGuid,
			 MemberIndex,
			 &devInfoData);

		if (Result != 0)
		{
			//A device has been detected, so get more information about it.

			/*
			  API function: SetupDiGetDeviceInterfaceDetail
			  Returns: an SP_DEVICE_INTERFACE_DETAIL_DATA structure
			  containing information about a 
			  To retrieve the information, call this function twice.
			  The first time returns the size of the structure in Length.
			  The second time returns a pointer to the data in DeviceInfoSet.
			  Requires:
			  A DeviceInfoSet returned by SetupDiGetClassDevs
			  The SP_DEVICE_INTERFACE_DATA structure returned by SetupDiEnumDeviceInterfaces.

			  The final parameter is an optional pointer to an SP_DEV_INFO_DATA structure.
			  This application doesn't retrieve or use the structure.
			  If retrieving the structure, set
			  MyDeviceInfoData.cbSize = length of MyDeviceInfoData.
			  and pass the structure's address.
			*/

			//Get the Length value.
			//The call will return with a "buffer too small" error which can be ignored.

			Result = SetupDiGetDeviceInterfaceDetail
				(hDevInfo,
				 &devInfoData,
				 NULL,
				 0,
				 &Length,
				 NULL);

			//Allocate memory for the hDevInfo structure, using the returned Length.

 			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);

			//Set cbSize in the detailData structure.

			detailData -> cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			//Call the function again, this time passing it the returned buffer size.

			Result = SetupDiGetDeviceInterfaceDetail
				(hDevInfo,
				 &devInfoData,
				 detailData,
				 Length,
				 &Required,
				 NULL);

			// Open a handle to the 
			// To enable retrieving information about a system mouse or keyboard,
			// don't request Read or Write access for this handle.

			/*
			  API function: CreateFile
			  Returns: a handle that enables reading and writing to the 
			  Requires:
			  The DevicePath in the detailData structure
			  returned by SetupDiGetDeviceInterfaceDetail.
			*/

			
			dev->_dev =CreateFile
				(detailData->DevicePath,
				 GENERIC_READ | GENERIC_WRITE,
				 FILE_SHARE_READ|FILE_SHARE_WRITE,
				 (LPSECURITY_ATTRIBUTES)NULL,
				 OPEN_EXISTING,
				 0,
				 NULL);

			/*
			  API function: HidD_GetAttributes
			  Requests information from the 
			  Requires: the handle returned by CreateFile.
			  Returns: a HIDD_ATTRIBUTES structure containing
			  the Vendor ID, Product ID, and Product Version Number.
			  Use this information to decide if the detected device is
			  the one we're looking for.
			*/

			//Set the Size to the number of bytes in the structure.

			Attributes.Size = sizeof(Attributes);

			Result = HidD_GetAttributes
				(dev->_dev,
				 &Attributes);

			//Is it the desired device?

			MyDeviceDetected = FALSE;

			if ((Attributes.VendorID == THIRDSPACEVEST_VID && Attributes.ProductID == THIRDSPACEVEST_PID))
			{
				if(get_count)
				{
					++device_count;
					CloseHandle(dev->_dev);
				}
				else
				{
					MyDeviceDetected = TRUE;
					MyDevicePathName = detailData->DevicePath;
					GetDeviceCapabilities(dev->_dev);
					break;
				}
			}
			else
			{
				CloseHandle(dev->_dev);
			}
			free(detailData);
		}  //if (Result != 0)

		else
		{
			LastDevice=TRUE;
		}
		//If we haven't found the device yet, and haven't tried every available device,
		//try the next one.
		MemberIndex = MemberIndex + 1;
	}
	while (!LastDevice);
	SetupDiDestroyDeviceInfoList(hDevInfo);
	if(get_count) return device_count;
	if(MyDeviceDetected) return 0;
	return -1;
}

THIRDSPACEVEST_DECLSPEC int thirdspacevest_get_count(thirdspacevest_device* dev)
{
	return thirdspacevest_open_win32(dev, 0, 1);
}

THIRDSPACEVEST_DECLSPEC int thirdspacevest_open(thirdspacevest_device* dev, unsigned int device_index)
{
	return thirdspacevest_open_win32(dev, device_index, 0);
}

THIRDSPACEVEST_DECLSPEC int thirdspacevest_close(thirdspacevest_device* dev)
{
	CloseHandle(dev->_dev);
	return 0;
}

THIRDSPACEVEST_DECLSPEC int thirdspacevest_read_data(thirdspacevest_device* dev, unsigned char *input_report)
{
	int Result;
	char read[64];
	Result = ReadFile
		(dev->_dev,
		 read,
		 Capabilities.InputReportByteLength,
		 &NumberOfBytesRead,
		 NULL);
	memcpy(input_report, read+1, 10);
	return Result;
}

THIRDSPACEVEST_DECLSPEC int thirdspacevest_write_data(thirdspacevest_device* dev, unsigned char *output_report)
{
	int Result;
	char command[64];
	command[0] = 0x0;
	memcpy((command+1), output_report, 10);
	Result = WriteFile
		(dev->_dev,
		 command,
		 Capabilities.OutputReportByteLength,
		 &NumberOfBytesRead,
		 (LPOVERLAPPED) &HIDOverlapped);
	return Result;
}

THIRDSPACEVEST_DECLSPEC thirdspacevest_device* thirdspacevest_create()
{
	thirdspacevest_device* s = (thirdspacevest_device*)malloc(sizeof(thirdspacevest_device));
	s->_is_open = 0;
	s->_is_inited = 1;	
	return s;
}

THIRDSPACEVEST_DECLSPEC void thirdspacevest_delete(thirdspacevest_device* dev)
{
	free(dev);
}
