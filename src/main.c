/**
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Protocol/Smbios.h>
#include <IndustryStandard/SmBios.h>
#include <Library/BaseMemoryLib.h>

CHAR8*
getstring(
    CHAR8* StringArea,
    UINT8 StringIndex
)
{
    CHAR8* str = StringArea;

    if (StringIndex == 0 || StringArea == NULL) {
        return NULL;
    }

    for (UINT8 i = 1; i < StringIndex; ++i)
    {
        while (*str != '\0') {
            str++;
        }

        str++;
    }

    return str;
}

typedef struct smbios_information
{
    CHAR8    *vendor;
    CHAR8    *bios_version;
    CHAR8    *release_date;
    CHAR8    *manufacturer;
    CHAR8    *product_name;
    CHAR8    *version;
    CHAR8    *serial_number;
    EFI_GUID uuid;
} SMBIOS_INFORMATION;


EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
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

    EFI_SMBIOS_HANDLE hSmbios = SMBIOS_HANDLE_PI_RESERVED;
    EFI_SMBIOS_TABLE_HEADER *record;

    SMBIOS_INFORMATION smbios_info;
    SetMem(&smbios_info, sizeof(smbios_info), 0);

    while (TRUE)
    {
        status = smbios->GetNext(
            smbios,
            &hSmbios,
            NULL,
            &record,
            NULL
        );

        if (EFI_ERROR(status)) {
            break;
        }

        switch (record->Type)
        {
        case SMBIOS_TYPE_BIOS_INFORMATION:
        {
            SMBIOS_TABLE_TYPE0 *type0 = (SMBIOS_TABLE_TYPE0*)record;
            CHAR8 *StringArea = (CHAR8*)record + type0->Hdr.Length;

            smbios_info.vendor = getstring(StringArea, type0->Vendor);
            smbios_info.bios_version = getstring(StringArea, type0->BiosVersion);
            smbios_info.release_date = getstring(StringArea, type0->BiosReleaseDate);

            if (smbios_info.vendor != NULL) {
                Print(L"Vendor: %a\n", smbios_info.vendor);
            }

            if (smbios_info.bios_version != NULL) {
                Print(L"BIOS Version: %a\n", smbios_info.bios_version);
            }

            if (smbios_info.release_date != NULL) {
                Print(L"Release Date: %a\n", smbios_info.release_date);
            }

            break;
        }
        case SMBIOS_TYPE_SYSTEM_INFORMATION:
        {
            SMBIOS_TABLE_TYPE1 *type1 = (SMBIOS_TABLE_TYPE1*)record;
            CHAR8 *StringArea = (CHAR8*)record + type1->Hdr.Length;

            smbios_info.manufacturer = getstring(StringArea, type1->Manufacturer);
            smbios_info.product_name = getstring(StringArea, type1->ProductName);
            smbios_info.version = getstring(StringArea, type1->Version);
            smbios_info.serial_number = getstring(StringArea, type1->SerialNumber);
            smbios_info.uuid = type1->Uuid;

            if (smbios_info.manufacturer != NULL) {
                Print(L"Manufacturer: %a\n", smbios_info.manufacturer);
            }

            if (smbios_info.product_name != NULL) {
                Print(L"Product Name: %a\n", smbios_info.product_name);
            }

            if (smbios_info.version != NULL) {
                Print(L"Version: %a\n", smbios_info.version);
            }

            if (smbios_info.serial_number != NULL) {
                Print(L"Serial Number: %a\n", smbios_info.serial_number);
            }

            Print(L"UUID: %g\n", &smbios_info.uuid);

            break;
        }
        default:
            break;
        }
    }

    return EFI_SUCCESS;
}