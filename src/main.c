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

CHAR8* getstring(CHAR8 *StringArea, UINT8 type0index)
{
    CHAR8 *vendorstr = StringArea;

    if (type0index == 0) {
        return NULL;
    }

    for (UINT8 i = 1; i < type0index; i++)
    {
        while (*vendorstr != '\0') {
            vendorstr++;
        }

        vendorstr++;
    }

    return vendorstr;
}

typedef struct smbios_information 
{
    CHAR8 *vendor;
    CHAR8 *bios_version;
    CHAR8 *release_date;
    CHAR8 *manufacturer;
    CHAR8 *product_name;
    CHAR8 *version;
    CHAR8 *serial_number;
} SMBIOS_INFORMATION;

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
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

        if (record->Type == SMBIOS_TYPE_BIOS_INFORMATION) 
        {
            SMBIOS_TABLE_TYPE0 *type0 = (SMBIOS_TABLE_TYPE0*) record;
            DEBUG((
                DEBUG_INFO, 
                "Found SMBIOS Type 0. Length: %d\n", 
                type0->Hdr.Length
            ));
                Print(
                L"Found SMBIOS Type 0. Length: %d\n", 
                type0->Hdr.Length
            );

            CHAR8 *StringArea = (CHAR8*)record + type0->Hdr.Length;

            smbios_info.vendor = getstring(StringArea, type0->Vendor);
            smbios_info.bios_version = getstring(StringArea, type0->BiosVersion);
            smbios_info.release_date = getstring(StringArea, type0->BiosReleaseDate);

            if (smbios_info.vendor != NULL) {
                Print(L"Vendor: %a\n", smbios_info.vendor);
            }

            if (smbios_info.bios_version != NULL) {
                Print(L"Bios Version: %a\n", smbios_info.bios_version);
            }
              
            if (smbios_info.release_date != NULL) {
                Print(L"Release Date: %a\n", smbios_info.release_date);
            }


        }
    }

    return EFI_SUCCESS;
}