/**
  Copyright (c) 2026, Z1KOx. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause
**/

#include <Uefi.h>
#include <Library/UefiLib.h>

#include "utils.h"

static CONST CHAR16 *chassis_names[] = {
    L"Unknown", L"Other", L"Unknown", L"Desktop", L"Low Profile Desktop", L"Pizza Box", L"Mini Tower",
    L"Tower", L"Portable", L"Laptop", L"Notebook", L"Hand Held", L"Docking Station", L"All in One"
};

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

CONST CHAR16*
smbios_chassis_name(
    UINT8 type
)
{
    return type < ARRAY_SIZE(chassis_names) ? chassis_names[type] : L"Unknown";
}