/**
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Protocol/Smbios.h>
#include <IndustryStandard/SmBios.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
    EFI_STATUS               Status;
    EFI_SMBIOS_PROTOCOL      *Smbios;
    EFI_SMBIOS_TABLE_HEADER  *Record;
    EFI_SMBIOS_HANDLE        HSmbios;

    Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID**)&Smbios);
    if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "Couldn't locate smbios protocol: %r\n", Status));
        return Status;
    }

    HSmbios = SMBIOS_HANDLE_PI_RESERVED;

    while (TRUE)
    {
      Status = Smbios->GetNext (Smbios, &HSmbios, NULL, &Record, NULL);

      if (EFI_ERROR (Status)) {
          break;
      }

      if (Record->Type == SMBIOS_TYPE_BIOS_INFORMATION) {
          SMBIOS_TABLE_TYPE0 *Type0 = (SMBIOS_TABLE_TYPE0*) Record;
          DEBUG ((DEBUG_INFO, "Found SMBIOS Type 0. Length: %d\n", Type0->Hdr.Length));
          Print (L"Found SMBIOS Type 0. Length: %d\n", Type0->Hdr.Length);
      }
    }

    Print (L"Hello World from UEFI!\n");
    return EFI_SUCCESS;
}