/**
  SPDX-License-Identifier: BSD-2-Clause-Patent
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
    CHAR8* string_area = (CHAR8*)((UINT8*)record + type0->Hdr.Length);

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
    CHAR8* string_area = (CHAR8*)((UINT8*)record + type1->Hdr.Length);

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
    CHAR8* string_area = (CHAR8*)((UINT8*)record + type2->Hdr.Length);

    Print(L"\n--- BASEBOARD INFORMATION (TYPE 2) ---\n");

    smbios_print_string(L"Mainboard Manufacturer", string_area, type2->Manufacturer);
    smbios_print_string(L"Product Name", string_area, type2->ProductName);
    smbios_print_string(L"Version", string_area, type2->Version);
    smbios_print_string(L"Serial Number", string_area, type2->SerialNumber);
}

EFI_STATUS
smbios_print_info(
    VOID
)
{
    EFI_SMBIOS_PROTOCOL *smbios;
    EFI_STATUS status;
    EFI_SMBIOS_HANDLE handle = SMBIOS_HANDLE_PI_RESERVED;
    EFI_SMBIOS_TABLE_HEADER *record;

    status = gBS->LocateProtocol(
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

        default:
            break;
        }
    }

    return EFI_SUCCESS;
}