/**
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Protocol/Smbios.h>
#include <IndustryStandard/SmBios.h>

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_SMBIOS_PROTOCOL *smbios;

    EFI_STATUS status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID**)&smbios);
    if (EFI_ERROR (status)) {
        DEBUG ((DEBUG_ERROR, "Couldn't locate smbios protocol: %r\n", status));
        return status;
    }

    EFI_SMBIOS_HANDLE hSmbios = SMBIOS_HANDLE_PI_RESERVED;
    EFI_SMBIOS_TABLE_HEADER *record;

    while (TRUE)
    {
      status = smbios->GetNext (smbios, &hSmbios, NULL, &record, NULL);

      if (EFI_ERROR (status)) {
          break;
      }

      if (record->Type == SMBIOS_TYPE_BIOS_INFORMATION) {
          SMBIOS_TABLE_TYPE0 *Type0 = (SMBIOS_TABLE_TYPE0*) record;
          DEBUG ((DEBUG_INFO, "Found SMBIOS Type 0. Length: %d\n", Type0->Hdr.Length));
          Print (L"Found SMBIOS Type 0. Length: %d\n", Type0->Hdr.Length);
      }
    }

    return EFI_SUCCESS;
}