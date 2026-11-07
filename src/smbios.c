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

static
VOID
smbios_print_type0_bios(
    EFI_SMBIOS_TABLE_HEADER *record
)
{
    SMBIOS_TABLE_TYPE0 *type0 = (SMBIOS_TABLE_TYPE0*)record;
    CHAR8 *string_area = (CHAR8*)((UINT8*)record + type0->Hdr.Length);

    Print(L"--- BIOS INFORMATION (TYPE 0) ---\n");

    smbios_print_string(L"Vendor", string_area, type0->Vendor);
    smbios_print_string(L"BIOS Version", string_area, type0->BiosVersion);
    smbios_print_string(L"Release Date", string_area, type0->BiosReleaseDate);
}

static
VOID
smbios_print_type1_system(
    EFI_SMBIOS_TABLE_HEADER *record
)
{
    SMBIOS_TABLE_TYPE1 *type1 = (SMBIOS_TABLE_TYPE1*)record;
    CHAR8 *string_area = (CHAR8*)((UINT8*)record + type1->Hdr.Length);

    Print(L"\n--- SYSTEM INFORMATION (TYPE 1) ---\n");

    smbios_print_string(L"Manufacturer", string_area, type1->Manufacturer);
    smbios_print_string(L"Product Name", string_area, type1->ProductName);
    smbios_print_string(L"Version", string_area, type1->Version);
    smbios_print_string(L"Serial Number", string_area, type1->SerialNumber);

    Print(L"UUID: %g\n", &type1->Uuid);
}

static
VOID
smbios_print_type2_baseboard(
    EFI_SMBIOS_TABLE_HEADER *record
)
{
    SMBIOS_TABLE_TYPE2 *type2 = (SMBIOS_TABLE_TYPE2*)record;
    CHAR8 *string_area = (CHAR8*)((UINT8*)record + type2->Hdr.Length);

    Print(L"\n--- BASEBOARD INFORMATION (TYPE 2) ---\n");

    smbios_print_string(L"Mainboard Manufacturer", string_area, type2->Manufacturer);
    smbios_print_string(L"Product Name", string_area, type2->ProductName);
    smbios_print_string(L"Version", string_area, type2->Version);
    smbios_print_string(L"Serial Number", string_area, type2->SerialNumber);
}

static
CONST CHAR16*
chassis_type_to_string(
    UINT8 type
)
{
    switch (type & 0x7F)
    {
        case 0x01: return L"Other";
        case 0x02: return L"Unknown";
        case 0x03: return L"Desktop";
        case 0x04: return L"Low Profile Desktop";
        case 0x06: return L"Mini Tower";
        case 0x07: return L"Tower";
        case 0x08: return L"Portable";
        case 0x09: return L"Laptop";
        case 0x0A: return L"Notebook";
        case 0x0D: return L"All in One";
        case 0x1F: return L"Convertible";
        default:   return L"Unknown";
    }
}

static
VOID
smbios_print_type3_mechanical(
    EFI_SMBIOS_TABLE_HEADER *record
)
{
    SMBIOS_TABLE_TYPE3* type3 = (SMBIOS_TABLE_TYPE3*)record;
    CHAR8 *string_area = (CHAR8*)((UINT8*)record + type3->Hdr.Length);

    Print(L"\n--- MECHANICAL ENCLOSURE INFORMATION (TYPE 3) ---\n");

    smbios_print_string(L"Manufacturer", string_area, type3->Manufacturer);
    Print(L"Chassis Type: %s (0x%02x)\n", chassis_type_to_string(type3->Type), type3->Type & 0x7F);
    smbios_print_string(L"Serial Number", string_area, type3->SerialNumber);
    smbios_print_string(L"Version", string_area, type3->Version);
}

static
VOID
smbios_print_type4_processor(
    EFI_SMBIOS_TABLE_HEADER *record
)
{
    SMBIOS_TABLE_TYPE4* type4 = (SMBIOS_TABLE_TYPE4*)record;
    CHAR8 *string_area = (CHAR8*)((UINT8*)record + type4->Hdr.Length);

    Print(L"\n--- PROCESSOR INFORMATION (TYPE 4) ---\n");

    smbios_print_string(L"Manufacturer", string_area, type4->ProcessorManufacturer);
    smbios_print_string(L"Version", string_area, type4->ProcessorVersion);

    UINT16 cores = type4->CoreCount;
    UINT16 threads = type4->ThreadCount;

    if (type4->Hdr.Length >= 0x30)
    {
        if (cores == 0xFF)   { cores = type4->CoreCount2; }
        if (threads == 0xFF) { threads = type4->ThreadCount2; }
    }

    Print(L"Core Count: %u\n", cores);
    Print(L"Thread Count: %u\n", threads);
}

EFI_STATUS
smbios_print_info(
    VOID
)
{
    EFI_SMBIOS_PROTOCOL *smbios;
    EFI_STATUS status = gBS->LocateProtocol(
        &gEfiSmbiosProtocolGuid,
        NULL,
        (VOID**)&smbios
    );

    if (EFI_ERROR(status)) 
    {
        DEBUG((
            DEBUG_ERROR,
            "Couldn't locate SMBIOS protocol: %r\n",
            status
            ));

        return status;
    }

    EFI_SMBIOS_HANDLE handle = SMBIOS_HANDLE_PI_RESERVED;
    EFI_SMBIOS_TABLE_HEADER* record;

    while (TRUE) 
    {
        status = smbios->GetNext(
            smbios,
            &handle,
            NULL,
            &record,
            NULL
        );

        if (status == EFI_NOT_FOUND) {
            break;
        }

        if (EFI_ERROR(status)) {
            return status;
        }

        switch (record->Type) 
        {
        case SMBIOS_TYPE_BIOS_INFORMATION:
            smbios_print_type0_bios(record);
            break;

        case SMBIOS_TYPE_SYSTEM_INFORMATION:
            smbios_print_type1_system(record);
            break;

        case SMBIOS_TYPE_BASEBOARD_INFORMATION:
            smbios_print_type2_baseboard(record);
            break;
        case SMBIOS_TYPE_SYSTEM_ENCLOSURE:
            smbios_print_type3_mechanical(record);
            break;
        case SMBIOS_TYPE_PROCESSOR_INFORMATION:
            smbios_print_type4_processor(record);
            break;

        default:
            break;
        }
    }

    return EFI_SUCCESS;
}