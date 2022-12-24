#define _CRT_SECURE_NO_WARNINGS

#include "easy_hwid.h"

#include <windows.h>
#include <intrin.h>
#include <iostream>
#include <ntddscsi.h>

typedef struct _IdentifyMiniportResponse {
	uint16_t pad_0;
	uint16_t pad_1;
	uint16_t pad_2;
	uint16_t pad_3;
	uint16_t pad_4;
	uint16_t pad_5;
	uint16_t pad_6;
	uint16_t pad_7[3];
	char serial[20];
} IdentifyMiniportResponse;

#define  FILE_DEVICE_SCSI              0x0000001b
#define  IOCTL_SCSI_MINIPORT_IDENTIFY  ((FILE_DEVICE_SCSI << 16) + 0x0501)

void StrTrim(std::string& s) {
	int i = 0, j = s.size() - 1;
	while (s[i] == ' ' || s[i] == '\t') i++;
	while (s[j] == ' ' || s[j] == '\t') j--;
	s = s.substr(i, s.size() - i - j);
}

namespace easy_hwid {
	// taken from https://github.com/HeathHowren/HWID-Info-Grabber
	// i'm not sure that it's original source of this code
	std::string CPUHash() {

		int cpuInfo[4] = { 0 }; //EAX, EBX, ECX, EDX
		__cpuid(cpuInfo, 0);
		char16_t hash = 0;
		char16_t* pointer = (char16_t*)(&cpuInfo[0]);
		for (char32_t i = 0; i < 8; i++)
			hash += pointer[i];

		return std::to_string(hash);
	}
	
	// author is unknown
	std::string VideoAdapter() {
		DISPLAY_DEVICEA dd;
		dd.cb = sizeof(DISPLAY_DEVICEA);
		EnumDisplayDevicesA(NULL, 0, &dd, EDD_GET_DEVICE_INTERFACE_NAME);

		std::string res = dd.DeviceString;
		return res;
	}

	// taken from https://www.unknowncheats.me/forum/2781103-post2.html
	std::string PhysicalDrive() {
		std::string m_sResult;

		HANDLE m_hFile = CreateFileW(L"\\\\.\\PhysicalDrive0", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
			return { };

		std::unique_ptr< std::remove_pointer <HANDLE >::type, void(*)(HANDLE) > m_hDevice
		{
			m_hFile, [](HANDLE handle)
			{
				CloseHandle(handle);
			}
		};

		STORAGE_PROPERTY_QUERY m_PropertyQuery;
		m_PropertyQuery.PropertyId = StorageDeviceProperty;
		m_PropertyQuery.QueryType = PropertyStandardQuery;

		STORAGE_DESCRIPTOR_HEADER m_DescHeader;
		DWORD m_dwBytesReturned = 0;
		if (!DeviceIoControl(m_hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &m_PropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
			&m_DescHeader, sizeof(STORAGE_DESCRIPTOR_HEADER), &m_dwBytesReturned, NULL))
			return { };

		const DWORD m_dwOutBufferSize = m_DescHeader.Size;
		std::unique_ptr< BYTE[] > m_pOutBuffer{ new BYTE[m_dwOutBufferSize] { } };

		if (!DeviceIoControl(m_hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &m_PropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
			m_pOutBuffer.get(), m_dwOutBufferSize, &m_dwBytesReturned, NULL))
			return { };

		STORAGE_DEVICE_DESCRIPTOR * m_pDeviceDescriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(m_pOutBuffer.get());
		const DWORD m_dwSerialNumberOffset = m_pDeviceDescriptor->SerialNumberOffset;
		if (m_dwSerialNumberOffset == 0)
			return { };

		m_sResult = reinterpret_cast<const char*>(m_pOutBuffer.get() + m_dwSerialNumberOffset);

		StrTrim(m_sResult);
		return m_sResult;
	}
}