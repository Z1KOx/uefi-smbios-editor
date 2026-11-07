/**
  Copyright (c) 2026, Z1KOx. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause
**/

#include <Uefi.h>
#include <Library/UefiLib.h>

#include "utils.h"

VOID
smbios_print_string(
    CONST CHAR16            *label,
    EFI_SMBIOS_TABLE_HEADER *record,
    UINT8                   string_index
)
{
    CHAR8 *str = (CHAR8*)record + record->Length;

    if (string_index == 0) {
        return;
    }

    while (--string_index && *str) {
        while (*str++) {}
    }

    if (*str) {
        Print(L"%s: %a\n", label, str);
    }
}