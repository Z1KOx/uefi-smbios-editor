/**
  Copyright (c) 2026, Z1KOx. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause
**/

#include <Uefi.h>
#include <Library/UefiLib.h>

#include "console.h"

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE       image_handle,
    IN EFI_SYSTEM_TABLE *system_table
)
{
    run_console();

    return EFI_SUCCESS;
}