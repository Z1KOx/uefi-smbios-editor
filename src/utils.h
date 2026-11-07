/**
  Copyright (c) 2026, Z1KOx. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause
**/

#ifndef UTILS_H
#define UTILS_H

#include <Uefi.h>

CHAR8*
smbios_get_string(
    CHAR8 *string_area,
    UINT8 string_index
);

VOID
smbios_print_string(
    CONST CHAR16* label,
    CHAR8*        string_area,
    UINT8         string_index
);

#endif