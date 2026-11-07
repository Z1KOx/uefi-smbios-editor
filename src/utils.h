/**
  Copyright (c) 2026, Z1KOx. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause
**/

#ifndef UTILS_H
#define UTILS_H

#include <Uefi.h>
#include <Protocol/Smbios.h>
#include <IndustryStandard/SmBios.h>

VOID
smbios_print_string(
    CONST CHAR16            *label,
    EFI_SMBIOS_TABLE_HEADER *record,
    UINT8                   string_index
);

#endif