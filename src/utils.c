/**
  Copyright (c) 2026, Z1KOx. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause
**/

#include <Uefi.h>
#include <Library/UefiLib.h>

#include "utils.h"

CHAR8*
smbios_get_string(
    CHAR8 *string_area,
    UINT8 string_index
)
{
    CHAR8 *str = string_area;

    if (string_index == 0 || string_area == NULL) {
        return NULL;
    }

    for (UINT8 i = 1; i < string_index; i++) 
    {
        while (*str != '\0') {
            str++;
        }
        str++;
    }

    return str;
}

VOID
smbios_print_string(
    CONST CHAR16 *label,
    CHAR8        *string_area,
    UINT8        string_index
)
{
    CHAR8 *str = smbios_get_string(string_area, string_index);

    if (str != NULL && *str != '\0') {
        Print(L"%s: %a\n", label, str);
    }
}