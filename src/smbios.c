/**
  Copyright (c) 2026, Z1KOx. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause
**/

#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/Smbios.h>
#include <IndustryStandard/SmBios.h>

#include "smbios.h"
#include "utils.h"

static EFI_GUID var_guid = { 0x3f1c9a52, 0x6b7d, 0x4e08,{ 0x9a, 0xd1, 0x52, 0xc4, 0x7e, 0x80, 0xb3, 0x16 } };

typedef struct 
{
    CONST CHAR8 *path;
    UINT8        type;
    UINT8        offset;
    UINT8        instance;
} FIELD;

#define FIELD_ENTRY(Path, Type, Structure, Member) { Path, Type, OFFSET_OF(Structure, Member), 0 }

#define FIELD_ENTRY_INSTANCE(Path, Type, Structure, Member, Instance) { Path, Type, OFFSET_OF(Structure, Member), Instance }

#define MEMORY_FIELDS(Slot, Instance) \
    FIELD_ENTRY_INSTANCE("smbios.memory[" #Slot "].device_locator", SMBIOS_TYPE_MEMORY_DEVICE, SMBIOS_TABLE_TYPE17, DeviceLocator, Instance), \
    FIELD_ENTRY_INSTANCE("smbios.memory[" #Slot "].bank_locator",   SMBIOS_TYPE_MEMORY_DEVICE, SMBIOS_TABLE_TYPE17, BankLocator,   Instance), \
    FIELD_ENTRY_INSTANCE("smbios.memory[" #Slot "].manufacturer",   SMBIOS_TYPE_MEMORY_DEVICE, SMBIOS_TABLE_TYPE17, Manufacturer,  Instance), \
    FIELD_ENTRY_INSTANCE("smbios.memory[" #Slot "].serial",         SMBIOS_TYPE_MEMORY_DEVICE, SMBIOS_TABLE_TYPE17, SerialNumber,  Instance), \
    FIELD_ENTRY_INSTANCE("smbios.memory[" #Slot "].asset_tag",      SMBIOS_TYPE_MEMORY_DEVICE, SMBIOS_TABLE_TYPE17, AssetTag,      Instance), \
    FIELD_ENTRY_INSTANCE("smbios.memory[" #Slot "].part_number",    SMBIOS_TYPE_MEMORY_DEVICE, SMBIOS_TABLE_TYPE17, PartNumber,    Instance)

static CONST FIELD fields[] = 
{
    /* Type 0: BIOS Information */
    FIELD_ENTRY("smbios.bios.vendor",      SMBIOS_TYPE_BIOS_INFORMATION, SMBIOS_TABLE_TYPE0, Vendor),
    FIELD_ENTRY("smbios.bios.version",     SMBIOS_TYPE_BIOS_INFORMATION, SMBIOS_TABLE_TYPE0, BiosVersion),
    FIELD_ENTRY("smbios.bios.release_date",SMBIOS_TYPE_BIOS_INFORMATION, SMBIOS_TABLE_TYPE0, BiosReleaseDate),

    /* Type 1: System Information */
    FIELD_ENTRY("smbios.system.manufacturer", SMBIOS_TYPE_SYSTEM_INFORMATION, SMBIOS_TABLE_TYPE1, Manufacturer),
    FIELD_ENTRY("smbios.system.product",      SMBIOS_TYPE_SYSTEM_INFORMATION, SMBIOS_TABLE_TYPE1, ProductName),
    FIELD_ENTRY("smbios.system.version",      SMBIOS_TYPE_SYSTEM_INFORMATION, SMBIOS_TABLE_TYPE1, Version),
    FIELD_ENTRY("smbios.system.serial",       SMBIOS_TYPE_SYSTEM_INFORMATION, SMBIOS_TABLE_TYPE1, SerialNumber),
    FIELD_ENTRY("smbios.system.sku",          SMBIOS_TYPE_SYSTEM_INFORMATION, SMBIOS_TABLE_TYPE1, SKUNumber),
    FIELD_ENTRY("smbios.system.family",       SMBIOS_TYPE_SYSTEM_INFORMATION, SMBIOS_TABLE_TYPE1, Family),

    /* Type 2: Baseboard Information */
    FIELD_ENTRY("smbios.baseboard.manufacturer", SMBIOS_TYPE_BASEBOARD_INFORMATION, SMBIOS_TABLE_TYPE2, Manufacturer),
    FIELD_ENTRY("smbios.baseboard.product",      SMBIOS_TYPE_BASEBOARD_INFORMATION, SMBIOS_TABLE_TYPE2, ProductName),
    FIELD_ENTRY("smbios.baseboard.version",      SMBIOS_TYPE_BASEBOARD_INFORMATION, SMBIOS_TABLE_TYPE2, Version),
    FIELD_ENTRY("smbios.baseboard.serial",       SMBIOS_TYPE_BASEBOARD_INFORMATION, SMBIOS_TABLE_TYPE2, SerialNumber),
    FIELD_ENTRY("smbios.baseboard.asset_tag",    SMBIOS_TYPE_BASEBOARD_INFORMATION, SMBIOS_TABLE_TYPE2, AssetTag),
    FIELD_ENTRY("smbios.baseboard.location",     SMBIOS_TYPE_BASEBOARD_INFORMATION, SMBIOS_TABLE_TYPE2, LocationInChassis),

    /* Type 3: Chassis Information */
    FIELD_ENTRY("smbios.chassis.manufacturer", SMBIOS_TYPE_SYSTEM_ENCLOSURE, SMBIOS_TABLE_TYPE3, Manufacturer),
    FIELD_ENTRY("smbios.chassis.version",      SMBIOS_TYPE_SYSTEM_ENCLOSURE, SMBIOS_TABLE_TYPE3, Version),
    FIELD_ENTRY("smbios.chassis.serial",       SMBIOS_TYPE_SYSTEM_ENCLOSURE, SMBIOS_TABLE_TYPE3, SerialNumber),
    FIELD_ENTRY("smbios.chassis.asset_tag",    SMBIOS_TYPE_SYSTEM_ENCLOSURE, SMBIOS_TABLE_TYPE3, AssetTag),

    /* Type 4: Processor Information (first processor record) */
    FIELD_ENTRY("smbios.processor.socket",       SMBIOS_TYPE_PROCESSOR_INFORMATION, SMBIOS_TABLE_TYPE4, Socket),
    FIELD_ENTRY("smbios.processor.manufacturer",SMBIOS_TYPE_PROCESSOR_INFORMATION, SMBIOS_TABLE_TYPE4, ProcessorManufacturer),
    FIELD_ENTRY("smbios.processor.version",     SMBIOS_TYPE_PROCESSOR_INFORMATION, SMBIOS_TABLE_TYPE4, ProcessorVersion),
    FIELD_ENTRY("smbios.processor.serial",      SMBIOS_TYPE_PROCESSOR_INFORMATION, SMBIOS_TABLE_TYPE4, SerialNumber),
    FIELD_ENTRY("smbios.processor.asset_tag",   SMBIOS_TYPE_PROCESSOR_INFORMATION, SMBIOS_TABLE_TYPE4, AssetTag),
    FIELD_ENTRY("smbios.processor.part_number", SMBIOS_TYPE_PROCESSOR_INFORMATION, SMBIOS_TABLE_TYPE4, PartNumber),

    /* Type 17: Memory Device Information (two records in this QEMU setup) */
    MEMORY_FIELDS(0, 0),
    MEMORY_FIELDS(1, 1),
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
        print_str(L"SKU", SKUNumber);
        print_str(L"Family", Family);
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
        print_str(L"Asset Tag", AssetTag);
        print_str(L"Location", LocationInChassis);
        break;
    }
    case SMBIOS_TYPE_SYSTEM_ENCLOSURE:
    {
        SMBIOS_TABLE_TYPE3 *t = (SMBIOS_TABLE_TYPE3*)record;
        UINT8 type = t->Type & 0x7F;

        Print(L"\n--- SYSTEM ENCLOSURE INFORMATION (TYPE 3) ---\n");
        print_str(L"Manufacturer", Manufacturer);
        Print(L"Chassis Type: %s (0x%02x)\n", smbios_chassis_name(type), type);
        print_str(L"Serial Number", SerialNumber);
        print_str(L"Version", Version);
        print_str(L"Asset Tag", AssetTag);
        break;
    }
    case SMBIOS_TYPE_PROCESSOR_INFORMATION:
    {
        SMBIOS_TABLE_TYPE4 *t = (SMBIOS_TABLE_TYPE4*)record;
        UINT16 cores = t->CoreCount, threads = t->ThreadCount;

        if (cores == 0xFF && t->Hdr.Length >= 0x30) { cores = t->CoreCount2; }
        if (threads == 0xFF && t->Hdr.Length >= 0x30) { threads = t->ThreadCount2; }

        Print(L"\n--- PROCESSOR INFORMATION (TYPE 4) ---\n");
        print_str(L"Socket", Socket);
        print_str(L"Manufacturer", ProcessorManufacturer);
        print_str(L"Version", ProcessorVersion);
        print_str(L"Serial Number", SerialNumber);
        print_str(L"Asset Tag", AssetTag);
        print_str(L"Part Number", PartNumber);
        Print(L"Core Count: %u\nThread Count: %u\n", cores, threads);
        break;
    }
    case SMBIOS_TYPE_MEMORY_DEVICE:
    {
        SMBIOS_TABLE_TYPE17 *t = (SMBIOS_TABLE_TYPE17*)record;

        Print(L"\n--- MEMORY DEVICE INFORMATION (TYPE 17) ---\n");
        print_str(L"Device Locator", DeviceLocator);
        print_str(L"Bank Locator", BankLocator);
        print_str(L"Manufacturer", Manufacturer);
        print_str(L"Serial Number", SerialNumber);
        print_str(L"Asset Tag", AssetTag);
        print_str(L"Part Number", PartNumber);
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

static
EFI_STATUS
update_field(
    EFI_SMBIOS_PROTOCOL *smbios,
    CONST FIELD         *field,
    CHAR8               *value
)
{
    EFI_SMBIOS_TABLE_HEADER *record = NULL;
    EFI_SMBIOS_HANDLE handle = SMBIOS_HANDLE_PI_RESERVED;
    EFI_SMBIOS_TYPE type = field->type;
    EFI_STATUS status;

    for (UINTN instance = 0; instance <= field->instance; instance++)
    {
        status = smbios->GetNext(
            smbios, &handle, &type, &record, NULL
        );

        if (EFI_ERROR(status)) {
            return status;
        }
    }

    if (field->offset >= record->Length) {
        return EFI_UNSUPPORTED;
    }

    UINTN index = *((UINT8*)record + field->offset);

    if (index == 0) {
        return EFI_NOT_FOUND;
    }

    return smbios->UpdateString(smbios, &handle, &index, value);
}

EFI_STATUS
smbios_set(
    CONST CHAR8 *path,
    CHAR8       *value
)
{
    EFI_SMBIOS_PROTOCOL *smbios;
    CHAR16 name[64];
    EFI_STATUS status = gBS->LocateProtocol(&gEfiSmbiosProtocolGuid, NULL, (VOID**)&smbios);

    if (EFI_ERROR(status)) {
        return status;
    }

    for (UINTN i = 0; i < ARRAY_SIZE(fields); i++)
    {
        if (AsciiStrCmp(path, fields[i].path) != 0) {
            continue;
        }

        status = update_field(smbios, &fields[i], value);

        if (EFI_ERROR(status)) {
            return status;
        }

        UnicodeSPrint(name, sizeof(name), L"%a", path);

        return gRT->SetVariable(name, &var_guid,
            EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
            AsciiStrSize(value), value);
    }

    return EFI_NOT_FOUND;
}

EFI_STATUS
smbios_reset(
    CONST CHAR8 *path
)
{
    CHAR16 name[64];
    EFI_STATUS status;

    for (UINTN i = 0; i < ARRAY_SIZE(fields); i++)
    {
        if (AsciiStrCmp(path, fields[i].path) == 0) 
        {
            UnicodeSPrint(name, sizeof(name), L"%a", path);

            status = gRT->SetVariable(
                name,
                &var_guid,
                0,
                0,
                NULL
            );

            if (status == EFI_NOT_FOUND) {
                return EFI_SUCCESS;
            }

            return status;
        }
    }

    return EFI_NOT_FOUND;
}

EFI_STATUS
smbios_apply(
    VOID
)
{
    EFI_SMBIOS_PROTOCOL *smbios;
    CHAR16 name[64];
    CHAR8 value[256];
    EFI_STATUS status;
    EFI_STATUS first_error = EFI_SUCCESS;

    status = gBS->LocateProtocol(
        &gEfiSmbiosProtocolGuid,
        NULL,
        (VOID**)&smbios
    );

    if (EFI_ERROR(status)) {
        return status;
    }

    for (UINTN i = 0; i < ARRAY_SIZE(fields); i++)
    {
        UINTN size = sizeof(value);

        SetMem(value, sizeof(value), 0);
        UnicodeSPrint(name, sizeof(name), L"%a", fields[i].path);

        status = gRT->GetVariable(
            name, &var_guid, NULL, &size, value
        );

        if (status == EFI_NOT_FOUND) {
            continue;
        }

        if (EFI_ERROR(status)) 
        {
            Print(L"Load %a: %r\n", fields[i].path, status);

            if (!EFI_ERROR(first_error)) {
                first_error = status;
            }

            continue;
        }

        if (size == 0 || size > sizeof(value) || value[size - 1] != '\0') {
            status = EFI_COMPROMISED_DATA;
        }
        else {
            status = update_field(smbios, &fields[i], value);
        }

        Print(L"Apply %a: %r\n", fields[i].path, status);

        if (EFI_ERROR(status) && !EFI_ERROR(first_error)) {
            first_error = status;
        }
    }

    return first_error;
}