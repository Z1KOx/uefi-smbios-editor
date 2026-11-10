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
    UINTN        instance;
} FIELD;

typedef struct
{
    CONST CHAR8 *name;
    UINT8        offset;
} MEMORY_FIELD;

#define FIELD_ENTRY(Path, Type, Structure, Member) { Path, Type, OFFSET_OF(Structure, Member), 0 }

static CONST FIELD fields[] =
{
    FIELD_ENTRY("smbios.bios.vendor",      SMBIOS_TYPE_BIOS_INFORMATION, SMBIOS_TABLE_TYPE0, Vendor),
    FIELD_ENTRY("smbios.bios.version",     SMBIOS_TYPE_BIOS_INFORMATION, SMBIOS_TABLE_TYPE0, BiosVersion),
    FIELD_ENTRY("smbios.bios.release_date",SMBIOS_TYPE_BIOS_INFORMATION, SMBIOS_TABLE_TYPE0, BiosReleaseDate),

    FIELD_ENTRY("smbios.system.manufacturer", SMBIOS_TYPE_SYSTEM_INFORMATION, SMBIOS_TABLE_TYPE1, Manufacturer),
    FIELD_ENTRY("smbios.system.product",      SMBIOS_TYPE_SYSTEM_INFORMATION, SMBIOS_TABLE_TYPE1, ProductName),
    FIELD_ENTRY("smbios.system.version",      SMBIOS_TYPE_SYSTEM_INFORMATION, SMBIOS_TABLE_TYPE1, Version),
    FIELD_ENTRY("smbios.system.serial",       SMBIOS_TYPE_SYSTEM_INFORMATION, SMBIOS_TABLE_TYPE1, SerialNumber),
    FIELD_ENTRY("smbios.system.sku",          SMBIOS_TYPE_SYSTEM_INFORMATION, SMBIOS_TABLE_TYPE1, SKUNumber),
    FIELD_ENTRY("smbios.system.family",       SMBIOS_TYPE_SYSTEM_INFORMATION, SMBIOS_TABLE_TYPE1, Family),

    FIELD_ENTRY("smbios.baseboard.manufacturer", SMBIOS_TYPE_BASEBOARD_INFORMATION, SMBIOS_TABLE_TYPE2, Manufacturer),
    FIELD_ENTRY("smbios.baseboard.product",      SMBIOS_TYPE_BASEBOARD_INFORMATION, SMBIOS_TABLE_TYPE2, ProductName),
    FIELD_ENTRY("smbios.baseboard.version",      SMBIOS_TYPE_BASEBOARD_INFORMATION, SMBIOS_TABLE_TYPE2, Version),
    FIELD_ENTRY("smbios.baseboard.serial",       SMBIOS_TYPE_BASEBOARD_INFORMATION, SMBIOS_TABLE_TYPE2, SerialNumber),
    FIELD_ENTRY("smbios.baseboard.asset_tag",    SMBIOS_TYPE_BASEBOARD_INFORMATION, SMBIOS_TABLE_TYPE2, AssetTag),
    FIELD_ENTRY("smbios.baseboard.location",     SMBIOS_TYPE_BASEBOARD_INFORMATION, SMBIOS_TABLE_TYPE2, LocationInChassis),

    FIELD_ENTRY("smbios.chassis.manufacturer", SMBIOS_TYPE_SYSTEM_ENCLOSURE, SMBIOS_TABLE_TYPE3, Manufacturer),
    FIELD_ENTRY("smbios.chassis.version",      SMBIOS_TYPE_SYSTEM_ENCLOSURE, SMBIOS_TABLE_TYPE3, Version),
    FIELD_ENTRY("smbios.chassis.serial",       SMBIOS_TYPE_SYSTEM_ENCLOSURE, SMBIOS_TABLE_TYPE3, SerialNumber),
    FIELD_ENTRY("smbios.chassis.asset_tag",    SMBIOS_TYPE_SYSTEM_ENCLOSURE, SMBIOS_TABLE_TYPE3, AssetTag),

    FIELD_ENTRY("smbios.processor.socket",       SMBIOS_TYPE_PROCESSOR_INFORMATION, SMBIOS_TABLE_TYPE4, Socket),
    FIELD_ENTRY("smbios.processor.manufacturer",SMBIOS_TYPE_PROCESSOR_INFORMATION, SMBIOS_TABLE_TYPE4, ProcessorManufacturer),
    FIELD_ENTRY("smbios.processor.version",     SMBIOS_TYPE_PROCESSOR_INFORMATION, SMBIOS_TABLE_TYPE4, ProcessorVersion),
    FIELD_ENTRY("smbios.processor.serial",      SMBIOS_TYPE_PROCESSOR_INFORMATION, SMBIOS_TABLE_TYPE4, SerialNumber),
    FIELD_ENTRY("smbios.processor.asset_tag",   SMBIOS_TYPE_PROCESSOR_INFORMATION, SMBIOS_TABLE_TYPE4, AssetTag),
    FIELD_ENTRY("smbios.processor.part_number", SMBIOS_TYPE_PROCESSOR_INFORMATION, SMBIOS_TABLE_TYPE4, PartNumber),

};

static CONST MEMORY_FIELD memory_fields[] =
{
    { "device_locator", OFFSET_OF(SMBIOS_TABLE_TYPE17, DeviceLocator) },
    { "bank_locator",   OFFSET_OF(SMBIOS_TABLE_TYPE17, BankLocator) },
    { "manufacturer",   OFFSET_OF(SMBIOS_TABLE_TYPE17, Manufacturer) },
    { "serial",         OFFSET_OF(SMBIOS_TABLE_TYPE17, SerialNumber) },
    { "asset_tag",      OFFSET_OF(SMBIOS_TABLE_TYPE17, AssetTag) },
    { "part_number",    OFFSET_OF(SMBIOS_TABLE_TYPE17, PartNumber) }
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

        Print(L"--- SYSTEM INFORMATION (TYPE 1) ---\n");
        print_str(L"Manufacturer", Manufacturer);
        print_str(L"Product Name", ProductName);
        print_str(L"Version", Version);
        print_str(L"Serial Number", SerialNumber);
        print_str(L"SKU", SKUNumber);
        print_str(L"Family", Family);
        if (record->Length >= OFFSET_OF(SMBIOS_TABLE_TYPE1, Uuid) + sizeof(t->Uuid)) {
            Print(L"UUID: %g\n", &t->Uuid);
        }
        break;
    }
    case SMBIOS_TYPE_BASEBOARD_INFORMATION:
    {
        SMBIOS_TABLE_TYPE2 *t = (SMBIOS_TABLE_TYPE2*)record;

        Print(L"--- BASEBOARD INFORMATION (TYPE 2) ---\n");
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
        SMBIOS_TABLE_TYPE3* t = (SMBIOS_TABLE_TYPE3*)record;

        Print(L"--- SYSTEM ENCLOSURE INFORMATION (TYPE 3) ---\n");

        print_str(L"Manufacturer", Manufacturer);

        // Chassis Type
        if (record->Length >= OFFSET_OF(SMBIOS_TABLE_TYPE3, Type) + sizeof(t->Type))
        {
            UINT8 type = t->Type & 0x7F;
            Print(L"Chassis Type: %s (0x%02x)\n", smbios_chassis_name(type), type);
        }

        print_str(L"Serial Number", SerialNumber);
        print_str(L"Version", Version);
        print_str(L"Asset Tag", AssetTag);

        break;
    }
    case SMBIOS_TYPE_PROCESSOR_INFORMATION:
    {
        SMBIOS_TABLE_TYPE4 *t = (SMBIOS_TABLE_TYPE4*)record;

        Print(L"--- PROCESSOR INFORMATION (TYPE 4) ---\n");

        print_str(L"Socket", Socket);
        print_str(L"Manufacturer", ProcessorManufacturer);
        print_str(L"Version", ProcessorVersion);
        print_str(L"Serial Number", SerialNumber);
        print_str(L"Asset Tag", AssetTag);
        print_str(L"Part Number", PartNumber);

        // Core Count
        if (record->Length >= OFFSET_OF(SMBIOS_TABLE_TYPE4, CoreCount) + sizeof(t->CoreCount))
        {
            UINT16 cores = t->CoreCount;

            if (cores == 0xFF && record->Length >= OFFSET_OF(SMBIOS_TABLE_TYPE4, CoreCount2) + sizeof(t->CoreCount2)) {
                cores = t->CoreCount2;
            }

            Print(L"Core Count: %u\n", cores);
        }

        // Thread Count
        if (record->Length >= OFFSET_OF(SMBIOS_TABLE_TYPE4, ThreadCount) + sizeof(t->ThreadCount))
        {
            UINT16 threads = t->ThreadCount;

            if (threads == 0xFF && record->Length >= OFFSET_OF(SMBIOS_TABLE_TYPE4, ThreadCount2) + sizeof(t->ThreadCount2)) {
                threads = t->ThreadCount2;
            }

            Print(L"Thread Count: %u\n", threads);
        }

        break;
    }
    case SMBIOS_TYPE_MEMORY_DEVICE:
    {
        SMBIOS_TABLE_TYPE17 *t = (SMBIOS_TABLE_TYPE17*)record;

        Print(L"MEMORY DEVICE INFORMATION (TYPE 17) ---\n");
        print_str(L"Device Locator", DeviceLocator);
        print_str(L"Bank Locator", BankLocator);
        print_str(L"Manufacturer", Manufacturer);
        print_str(L"Serial Number", SerialNumber);
        print_str(L"Asset Tag", AssetTag);
        print_str(L"Part Number", PartNumber);

        // Check if Size exists
        if (record->Length >= OFFSET_OF(SMBIOS_TABLE_TYPE17, Size) + sizeof(t->Size))
        {
            if (t->Size == 0) {
                Print(L"Size: Not installed\n");
            }
            else if (t->Size == 0xFFFF) {
                Print(L"Size: Unknown\n");
            }
            else if (t->Size == 0x7FFF)
            {
                if (record->Length >= OFFSET_OF(SMBIOS_TABLE_TYPE17, ExtendedSize) + sizeof(t->ExtendedSize)) {
                    Print(L"Size: %u MB\n", t->ExtendedSize & 0x7FFFFFFF);
                }
            }
            else if (t->Size & 0x8000) {
                Print(L"Size: %u KB\n", t->Size & 0x7FFF);
            }
            else {
                Print(L"Size: %u MB\n", t->Size);
            }
        }

        // Check if Speed exists
        if (record->Length >= OFFSET_OF(SMBIOS_TABLE_TYPE17, Speed) + sizeof(t->Speed))
        {
            if (t->Speed == 0) {
                Print(L"Speed: Unknown\n");
            }
            else {
                Print(L"Speed: %u MT/s\n", t->Speed);
            }
        }

        break;
    }
    }
}

EFI_STATUS
smbios_print_info(
    VOID
)
{
    EFI_SMBIOS_PROTOCOL     *smbios;
    EFI_SMBIOS_TABLE_HEADER *record;
    EFI_SMBIOS_HANDLE        handle = SMBIOS_HANDLE_PI_RESERVED;
    EFI_INPUT_KEY            key;
    EFI_STATUS               status;
    UINTN                    event_index;
    UINTN                    memory_index = 0;

    status = gBS->LocateProtocol(
        &gEfiSmbiosProtocolGuid,
        NULL,
        (VOID**)&smbios
    );

    if (EFI_ERROR(status)) {
        return status;
    }

    while (!EFI_ERROR(status = smbios->GetNext(smbios, &handle, NULL, &record, NULL)))
    {
        switch (record->Type)
        {
            case SMBIOS_TYPE_BIOS_INFORMATION:
            case SMBIOS_TYPE_SYSTEM_INFORMATION:
            case SMBIOS_TYPE_BASEBOARD_INFORMATION:
            case SMBIOS_TYPE_SYSTEM_ENCLOSURE:
            case SMBIOS_TYPE_PROCESSOR_INFORMATION:
            case SMBIOS_TYPE_MEMORY_DEVICE:
                break;

            default:
                continue;
        }

        gST->ConOut->ClearScreen(gST->ConOut);
        Print(L"SMBIOS Version: %u.%u\n\n", (UINTN)smbios->MajorVersion, (UINTN)smbios->MinorVersion);

        if (record->Type == SMBIOS_TYPE_MEMORY_DEVICE) {
            Print(L"--- [%u] ", memory_index++);
        }

        smbios_print_record(record);

        status = gBS->WaitForEvent(
            1,
            &gST->ConIn->WaitForKey,
            &event_index
        );

        if (EFI_ERROR(status)) {
            return status;
        }

        status = gST->ConIn->ReadKeyStroke(
            gST->ConIn, &key
        );

        if (EFI_ERROR(status)) {
            return status;
        }

        if (key.ScanCode == SCAN_ESC) {
            return EFI_SUCCESS;
        }
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
    UINTN instance = 0;
    UINTN index;

    while (TRUE)
    {
        status = smbios->GetNext(smbios, &handle, &type, &record, NULL);

        if (EFI_ERROR(status)) {
            return status;
        }

        if (instance == field->instance) {
            break;
        }

        instance++;
    }

    if (field->offset >= record->Length) {
        return EFI_UNSUPPORTED;
    }

    index = *((UINT8*)record + field->offset);

    if (index == 0) {
        return EFI_NOT_FOUND;
    }

    return smbios->UpdateString(smbios, &handle, &index, value);
}

static
EFI_STATUS
parse_memory_index(
    CONST CHAR8  *path,
    UINTN        *instance,
    CONST CHAR8 **member
)
{
    CONST CHAR8 *p;
    UINTN number = 0;
    UINTN digit;

    if (AsciiStrnCmp(path, "smbios.memory[", 14) != 0) {
        return EFI_NOT_FOUND;
    }

    p = path + 14;

    if (*p < '0' || *p > '9') {
        return EFI_INVALID_PARAMETER;
    }

    if (*p == '0' && p[1] >= '0' && p[1] <= '9') {
        return EFI_INVALID_PARAMETER;
    }

    while (*p >= '0' && *p <= '9')
    {
        digit = *p - '0';

        if (number > (MAX_UINTN - digit) / 10) {
            return EFI_INVALID_PARAMETER;
        }

        number = number * 10 + digit;
        p++;
    }

    if (*p != ']' || p[1] != '.' || p[2] == '\0') {
        return EFI_INVALID_PARAMETER;
    }

    *instance = number;
    *member = p + 2;
    return EFI_SUCCESS;
}

static
EFI_STATUS
find_field(
    CONST CHAR8 *path,
    FIELD       *field
)
{
    EFI_STATUS status;
    UINTN instance;
    CONST CHAR8 *member;

    if (path == NULL || field == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    if (AsciiStrLen(path) >= 64) {
        return EFI_INVALID_PARAMETER;
    }

    for (UINTN i = 0; i < ARRAY_SIZE(fields); i++)
    {
        if (AsciiStrCmp(path, fields[i].path) == 0) {
            *field = fields[i];
            return EFI_SUCCESS;
        }
    }

    status = parse_memory_index(path, &instance, &member);
    if (EFI_ERROR(status)) {
        return status;
    }

    for (UINTN i = 0; i < ARRAY_SIZE(memory_fields); i++)
    {
        if (AsciiStrCmp(member, memory_fields[i].name) == 0)
        {
            field->path = path;
            field->type = SMBIOS_TYPE_MEMORY_DEVICE;
            field->offset = memory_fields[i].offset;
            field->instance = instance;
            return EFI_SUCCESS;
        }
    }

    return EFI_NOT_FOUND;
}

// Read the current SMBIOS string for one selected field.
EFI_STATUS
smbios_get(
    CONST CHAR8 *path,
    CHAR8       *value,
    UINTN        value_size
)
{
    EFI_SMBIOS_PROTOCOL *smbios;
    EFI_SMBIOS_TABLE_HEADER *record = NULL;
    EFI_SMBIOS_HANDLE handle = SMBIOS_HANDLE_PI_RESERVED;
    EFI_SMBIOS_TYPE type;
    EFI_STATUS status;
    FIELD field;
    UINTN instance = 0;
    UINTN remaining = 4096;
    UINTN length;
    UINT8 string_index;
    CHAR8 *str;

    if (value == NULL || value_size == 0) {
        return EFI_INVALID_PARAMETER;
    }
    value[0] = '\0';

    status = find_field(path, &field);
    if (EFI_ERROR(status)) {
        return status;
    }
    status = gBS->LocateProtocol(
        &gEfiSmbiosProtocolGuid, NULL, (VOID**)&smbios
    );
    if (EFI_ERROR(status)) {
        return status;
    }

    type = field.type;
    while (TRUE) {
        status = smbios->GetNext(smbios, &handle, &type, &record, NULL);
        if (EFI_ERROR(status)) {
            return status;
        }
        if (instance++ == field.instance) {
            break;
        }
    }

    if (field.offset >= record->Length) {
        return EFI_UNSUPPORTED;
    }
    string_index = *((UINT8*)record + field.offset);
    if (string_index == 0) {
        return EFI_NOT_FOUND;
    }

    str = (CHAR8*)record + record->Length;
    for (UINTN i = 1; i <= string_index; i++) {
        if (remaining == 0 || *str == '\0') {
            return EFI_NOT_FOUND;
        }
        length = AsciiStrnLenS(str, remaining);
        if (length >= remaining) {
            return EFI_COMPROMISED_DATA;
        }
        if (i == string_index) {
            if (length >= value_size) {
                return EFI_BUFFER_TOO_SMALL;
            }
            CopyMem(value, str, length + 1);
            return EFI_SUCCESS;
        }
        str += length + 1;
        remaining -= length + 1;
    }

    return EFI_NOT_FOUND;
}

EFI_STATUS
smbios_set(
    CONST CHAR8 *path,
    CHAR8       *value
)
{
    EFI_SMBIOS_PROTOCOL *smbios;
    FIELD field;
    CHAR16 name[64];
    EFI_STATUS status;

    status = find_field(path, &field);
    if (EFI_ERROR(status)) {
        return status;
    }

    status = gBS->LocateProtocol(
        &gEfiSmbiosProtocolGuid, NULL, (VOID**)&smbios
    );
    if (EFI_ERROR(status)) {
        return status;
    }

    status = update_field(smbios, &field, value);
    if (EFI_ERROR(status)) {
        return status;
    }

    UnicodeSPrint(name, sizeof(name), L"%a", path);

    return gRT->SetVariable(
        name, &var_guid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
        AsciiStrSize(value), value
    );
}

EFI_STATUS
smbios_reset(
    CONST CHAR8 *path
)
{
    FIELD field;
    CHAR16 name[64];
    EFI_STATUS status;

    status = find_field(path, &field);
    if (EFI_ERROR(status)) {
        return status;
    }

    UnicodeSPrint(name, sizeof(name), L"%a", path);

    status = gRT->SetVariable(name, &var_guid, 0, 0, NULL);

    if (status == EFI_NOT_FOUND) {
        return EFI_SUCCESS;
    }

    return status;
}

static
EFI_STATUS
apply_saved_field(
    EFI_SMBIOS_PROTOCOL *smbios,
    CONST CHAR8         *path,
    CONST FIELD         *field
)
{
    CHAR16 name[64];
    CHAR8 value[256];
    UINTN size = sizeof(value);
    EFI_STATUS status;

    SetMem(value, sizeof(value), 0);
    UnicodeSPrint(name, sizeof(name), L"%a", path);

    status = gRT->GetVariable(name, &var_guid, NULL, &size, value);

    if (status == EFI_NOT_FOUND) {
        return EFI_SUCCESS;
    }

    if (EFI_ERROR(status)) {
        Print(L"Load %a: %r\n", path, status);
        return status;
    }

    if (size == 0 || size > sizeof(value) || value[size - 1] != '\0') {
        status = EFI_COMPROMISED_DATA;
    }
    else {
        status = update_field(smbios, field, value);
    }

    Print(L"Apply %a: %r\n", path, status);
    return status;
}

static
EFI_STATUS
count_memory_records(
    EFI_SMBIOS_PROTOCOL *smbios,
    UINTN               *count
)
{
    EFI_SMBIOS_HANDLE handle = SMBIOS_HANDLE_PI_RESERVED;
    EFI_SMBIOS_TYPE type = SMBIOS_TYPE_MEMORY_DEVICE;
    EFI_SMBIOS_TABLE_HEADER *record;
    EFI_STATUS status;

    *count = 0;

    while (!EFI_ERROR(status = smbios->GetNext(
        smbios, &handle, &type, &record, NULL)))
    {
        (*count)++;
    }

    return status == EFI_NOT_FOUND ? EFI_SUCCESS : status;
}

// Expose the existing Type 17 enumeration to the UI.
EFI_STATUS
smbios_memory_count(UINTN *count)
{
    EFI_SMBIOS_PROTOCOL *smbios;
    EFI_STATUS status;

    if (count == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    status = gBS->LocateProtocol(
        &gEfiSmbiosProtocolGuid, NULL, (VOID**)&smbios
    );
    if (EFI_ERROR(status)) {
        return status;
    }
    return count_memory_records(smbios, count);
}

EFI_STATUS
smbios_apply(
    VOID
)
{
    EFI_SMBIOS_PROTOCOL *smbios;
    EFI_STATUS status;
    EFI_STATUS first_error = EFI_SUCCESS;
    UINTN memory_count;
    CHAR8 path[64];
    FIELD field;

    status = gBS->LocateProtocol(
        &gEfiSmbiosProtocolGuid, NULL, (VOID**)&smbios
    );
    if (EFI_ERROR(status)) {
        return status;
    }

    for (UINTN i = 0; i < ARRAY_SIZE(fields); i++)
    {
        status = apply_saved_field(smbios, fields[i].path, &fields[i]);
        if (EFI_ERROR(status) && !EFI_ERROR(first_error)) {
            first_error = status;
        }
    }

    status = count_memory_records(smbios, &memory_count);
    if (EFI_ERROR(status)) {
        return EFI_ERROR(first_error) ? first_error : status;
    }

    for (UINTN n = 0; n < memory_count; n++)
    {
        for (UINTN i = 0; i < ARRAY_SIZE(memory_fields); i++)
        {
            AsciiSPrint(
                path, sizeof(path), "smbios.memory[%u].%a",
                n, memory_fields[i].name
            );

            field.path = path;
            field.type = SMBIOS_TYPE_MEMORY_DEVICE;
            field.offset = memory_fields[i].offset;
            field.instance = n;

            status = apply_saved_field(smbios, path, &field);
            if (EFI_ERROR(status) && !EFI_ERROR(first_error)) {
                first_error = status;
            }
        }
    }

    return first_error;
}