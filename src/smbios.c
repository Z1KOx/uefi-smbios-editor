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

typedef struct 
{
    CONST CHAR16 *label;
    UINT8        offset;
} STR_FIELD;

#define STR(label, type, member) { label, OFFSET_OF(type, member) }

static CONST STR_FIELD bios_fields[] = 
{
    STR(L"Vendor",       SMBIOS_TABLE_TYPE0, Vendor),
    STR(L"BIOS Version", SMBIOS_TABLE_TYPE0, BiosVersion),
    STR(L"Release Date", SMBIOS_TABLE_TYPE0, BiosReleaseDate),
};

static CONST STR_FIELD system_fields[] = 
{
    STR(L"Manufacturer",  SMBIOS_TABLE_TYPE1, Manufacturer),
    STR(L"Product Name",  SMBIOS_TABLE_TYPE1, ProductName),
    STR(L"Version",       SMBIOS_TABLE_TYPE1, Version),
    STR(L"Serial Number", SMBIOS_TABLE_TYPE1, SerialNumber),
};

static CONST STR_FIELD baseboard_fields[] =
{
    STR(L"Mainboard Manufacturer", SMBIOS_TABLE_TYPE2, Manufacturer),
    STR(L"Product Name",           SMBIOS_TABLE_TYPE2, ProductName),
    STR(L"Version",                SMBIOS_TABLE_TYPE2, Version),
    STR(L"Serial Number",          SMBIOS_TABLE_TYPE2, SerialNumber),
};

static CONST STR_FIELD enclosure_fields[] = 
{
    STR(L"Manufacturer",  SMBIOS_TABLE_TYPE3, Manufacturer),
    STR(L"Serial Number", SMBIOS_TABLE_TYPE3, SerialNumber),
    STR(L"Version",       SMBIOS_TABLE_TYPE3, Version),
};

static CONST STR_FIELD processor_fields[] =
{
    STR(L"Manufacturer", SMBIOS_TABLE_TYPE4, ProcessorManufacturer),
    STR(L"Version",      SMBIOS_TABLE_TYPE4, ProcessorVersion),
};

static CONST STR_FIELD memory_fields[] =
{
    STR(L"Manufacturer", SMBIOS_TABLE_TYPE17, Manufacturer),
    STR(L"Part Number",  SMBIOS_TABLE_TYPE17, PartNumber),
};

static CONST CHAR16* chassis_names[] = 
{
    L"Unknown", L"Other", L"Unknown", L"Desktop", L"Low Profile Desktop", L"Pizza Box", L"Mini Tower",
    L"Tower", L"Portable", L"Laptop", L"Notebook", L"Hand Held", L"Docking Station", L"All in One"
};

static
VOID
print_strings(
    CONST CHAR16            *title,
    EFI_SMBIOS_TABLE_HEADER *record,
    CONST STR_FIELD         *fields,
    UINTN                   count
)
{
    Print(L"\n--- %s ---\n", title);

    for (UINTN i = 0; i < count; i++) {
        smbios_print_string(fields[i].label, record, *((UINT8*)record + fields[i].offset));
    }
}

static
VOID
smbios_print_record(
    EFI_SMBIOS_TABLE_HEADER *record
)
{
    switch (record->Type)
    {
    case SMBIOS_TYPE_BIOS_INFORMATION:
        print_strings(L"BIOS INFORMATION (TYPE 0)", record, bios_fields, ARRAY_SIZE(bios_fields));
        break;

    case SMBIOS_TYPE_SYSTEM_INFORMATION:
    {
        SMBIOS_TABLE_TYPE1 *t = (SMBIOS_TABLE_TYPE1*)record;

        print_strings(L"SYSTEM INFORMATION (TYPE 1)", record, system_fields, ARRAY_SIZE(system_fields));
        Print(L"UUID: %g\n", &t->Uuid);
        break;
    }
    case SMBIOS_TYPE_BASEBOARD_INFORMATION:
        print_strings(L"BASEBOARD INFORMATION (TYPE 2)", record, baseboard_fields, ARRAY_SIZE(baseboard_fields));
        break;

    case SMBIOS_TYPE_SYSTEM_ENCLOSURE:
    {
        SMBIOS_TABLE_TYPE3 *t = (SMBIOS_TABLE_TYPE3*)record;
        UINT8 type = t->Type & 0x7F;

        print_strings(L"SYSTEM ENCLOSURE INFORMATION (TYPE 3)", record, enclosure_fields, ARRAY_SIZE(enclosure_fields));
        Print(L"Chassis Type: %s (0x%02x)\n", type < ARRAY_SIZE(chassis_names) ? chassis_names[type] : L"Unknown", type);
        break;
    }
    case SMBIOS_TYPE_PROCESSOR_INFORMATION:
    {
        SMBIOS_TABLE_TYPE4 *t = (SMBIOS_TABLE_TYPE4*)record;
        UINT16 cores = t->CoreCount, threads = t->ThreadCount;

        if (cores == 0xFF && t->Hdr.Length >= 0x30) { cores = t->CoreCount2; }
        if (threads == 0xFF && t->Hdr.Length >= 0x30) { threads = t->ThreadCount2; }

        print_strings(L"PROCESSOR INFORMATION (TYPE 4)", record, processor_fields, ARRAY_SIZE(processor_fields));
        Print(L"Core Count: %u\nThread Count: %u\n", cores, threads);
        break;
    }
    case SMBIOS_TYPE_MEMORY_DEVICE:
    {
        SMBIOS_TABLE_TYPE17 *t = (SMBIOS_TABLE_TYPE17*)record;

        if (t->Size == 0) {
            break;
        }

        print_strings(L"MEMORY DEVICE INFORMATION (TYPE 17)", record, memory_fields, ARRAY_SIZE(memory_fields));
        Print(L"Size: %u MB\n", t->Size == 0x7FFF ? t->ExtendedSize & 0x7FFFFFFF : t->Size);
        Print(L"Speed: %u MT/s\n", t->Speed);
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