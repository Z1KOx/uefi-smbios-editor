/**
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Protocol/Smbios.h>
#include <IndustryStandard/SmBios.h>

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

        CHAR8 *StringArea = (CHAR8 *)record + type0->Hdr.Length;

        const CHAR8 *vendor = getstring(StringArea, type0->Vendor);
        const CHAR8 *version = getstring(StringArea, type0->BiosVersion);
        const CHAR8 *date = getstring(StringArea, type0->BiosReleaseDate);

        if (vendor != NULL) {
           Print(L"Vendor: %a\n", vendor);
        }

        if (version != NULL) {
            Print(L"Version: %a\n", version);
        }
              
        if (date != NULL) {
            Print(L"Release Date: %a\n", date);
        }
      }
    }

    return EFI_SUCCESS;
}