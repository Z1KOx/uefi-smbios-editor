/**
  Copyright (c) 2026, Z1KOx. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause
**/

#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>

#include <Protocol/Smbios.h>
#include <IndustryStandard/SmBios.h>

#include "smbios.h"
#include "utils.h"

#define print_str(label, member) smbios_print_string(label, record, t->member)

static CONST CHAR16 *chassis_names[] = {
    L"Unknown", L"Other", L"Unknown", L"Desktop", L"Low Profile Desktop", L"Pizza Box", L"Mini Tower",
    L"Tower", L"Portable", L"Laptop", L"Notebook", L"Hand Held", L"Docking Station", L"All in One"
};

static
VOID
smbios_print_record(
    EFI_SMBIOS_TABLE_HEADER *record
)
{
    switch (record->Type)
    {
    case SMBIOS_TYPE_BIOS_INFORMATION: 
    {
        SMBIOS_TABLE_TYPE0 *t = (SMBIOS_TABLE_TYPE0*)record;

        Print(L"--- BIOS INFORMATION (TYPE 0) ---\n");
        print_str(L"Vendor", Vendor);
        print_str(L"BIOS Version", BiosVersion);
        print_str(L"Release Date", BiosReleaseDate);
        break;
    }
    case SMBIOS_TYPE_SYSTEM_INFORMATION:
    {
        SMBIOS_TABLE_TYPE1 *t = (SMBIOS_TABLE_TYPE1*)record;

        Print(L"\n--- SYSTEM INFORMATION (TYPE 1) ---\n");
        print_str(L"Manufacturer", Manufacturer);
        print_str(L"Product Name", ProductName);
        print_str(L"Version", Version);
        print_str(L"Serial Number", SerialNumber);
        Print(L"UUID: %g\n", &t->Uuid);
        break;
    }
    case SMBIOS_TYPE_BASEBOARD_INFORMATION:
    {
        SMBIOS_TABLE_TYPE2 *t = (SMBIOS_TABLE_TYPE2*)record;

        Print(L"\n--- BASEBOARD INFORMATION (TYPE 2) ---\n");
        print_str(L"Mainboard Manufacturer", Manufacturer);
        print_str(L"Product Name", ProductName);
        print_str(L"Version", Version);
        print_str(L"Serial Number", SerialNumber);
        break;
    }
    case SMBIOS_TYPE_SYSTEM_ENCLOSURE: 
    {
        SMBIOS_TABLE_TYPE3 *t = (SMBIOS_TABLE_TYPE3*)record;
        UINT8 type = t->Type & 0x7F;

        Print(L"\n--- SYSTEM ENCLOSURE INFORMATION (TYPE 3) ---\n");
        print_str(L"Manufacturer", Manufacturer);
        Print(L"Chassis Type: %s (0x%02x)\n", type < ARRAY_SIZE(chassis_names) ? chassis_names[type] : L"Unknown", type);
        print_str(L"Serial Number", SerialNumber);
        print_str(L"Version", Version);
        break;
    }
    case SMBIOS_TYPE_PROCESSOR_INFORMATION:
    {
        SMBIOS_TABLE_TYPE4 *t = (SMBIOS_TABLE_TYPE4*)record;
        UINT16 cores = t->CoreCount, threads = t->ThreadCount;
        
        if (cores == 0xFF && t->Hdr.Length >= 0x30) { cores = t->CoreCount2; }
        if (threads == 0xFF && t->Hdr.Length >= 0x30) { threads = t->ThreadCount2; }
        
        Print(L"\n--- PROCESSOR INFORMATION (TYPE 4) ---\n");
        print_str(L"Manufacturer", ProcessorManufacturer);
        print_str(L"Version", ProcessorVersion);
        Print(L"Core Count: %u\nThread Count: %u\n", cores, threads);
        break;
    }
    case SMBIOS_TYPE_MEMORY_DEVICE: 
    {
        SMBIOS_TABLE_TYPE17 *t = (SMBIOS_TABLE_TYPE17*)record;
       
        if (t->Size == 0) {
            break;
        }

        Print(L"\n--- MEMORY DEVICE INFORMATION (TYPE 17) ---\n");
        print_str(L"Manufacturer", Manufacturer);
        Print(L"Size: %u MB\n", t->Size == 0x7FFF ? t->ExtendedSize & 0x7FFFFFFF : t->Size);
        Print(L"Speed: %u MT/s\n", t->Speed);
        print_str(L"Part Number", PartNumber);
        break;
    }
    }
}

EFI_STATUS
smbios_print_info(
    VOID
)
{
    EFI_SMBIOS_PROTOCOL *smbios;
    EFI_STATUS status = gBS->LocateProtocol(&gEfiSmbiosProtocolGuid, NULL, (VOID**)&smbios);

    if (EFI_ERROR(status))
    {
        DEBUG((DEBUG_ERROR, "Couldn't locate SMBIOS protocol: %r\n", status));
        return status;
    }

    EFI_SMBIOS_TABLE_HEADER *record;
    EFI_SMBIOS_HANDLE handle = SMBIOS_HANDLE_PI_RESERVED;

    while (!EFI_ERROR(status = smbios->GetNext(smbios, &handle, NULL, &record, NULL))) {
        smbios_print_record(record);
    }

    return status == EFI_NOT_FOUND ? EFI_SUCCESS : status;
}