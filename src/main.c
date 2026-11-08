/**
  Copyright (c) 2026, Z1KOx. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Protocol/ShellParameters.h>

#include "console.h"
#include "smbios.h"

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE       image_handle,
    IN EFI_SYSTEM_TABLE *system_table
)
{
    EFI_SHELL_PARAMETERS_PROTOCOL *params;

    if (!EFI_ERROR(gBS->HandleProtocol(image_handle, &gEfiShellParametersProtocolGuid, (VOID**)&params)) &&
        params->Argc > 1 && StrCmp(params->Argv[1], L"/apply") == 0)
    {
        return smbios_apply();
    }

    run_console();

    return EFI_SUCCESS;
}