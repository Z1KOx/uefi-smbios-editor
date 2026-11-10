/**
  Copyright (c) 2026, Z1KOx. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause
**/

#ifndef SMBIOS_H
#define SMBIOS_H

#include <Uefi.h>

EFI_STATUS
smbios_print_info(
    VOID
);

EFI_STATUS
smbios_get(
    CONST CHAR8 *path,
    CHAR8       *value,
    UINTN        value_size
);

EFI_STATUS
smbios_memory_count(
    UINTN *count
);

EFI_STATUS
smbios_set(
    CONST CHAR8 *path,
    CHAR8       *value
);

EFI_STATUS
smbios_reset(
    CONST CHAR8 *path
);

EFI_STATUS
smbios_apply(
    VOID
);

#endif