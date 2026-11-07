/**
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>

#include "smbios.h"

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE       image_handle,
    IN EFI_SYSTEM_TABLE *system_table
)
{
    EFI_STATUS status;

    status = smbios_print_info();

    if (EFI_ERROR(status)) 
    {
        DEBUG((
            DEBUG_ERROR,
            "Failed to read SMBIOS information: %r\n",
            status
            ));

        return status;
    }

    return EFI_SUCCESS;
}