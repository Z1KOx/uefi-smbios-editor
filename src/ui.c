/**
  Copyright (c) 2026, Z1KOx. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>

#include "ui.h"
#include "console.h"
#include "smbios.h"

typedef struct {
    CONST CHAR16 *label;
    CONST CHAR8  *name;
} UI_FIELD;

static CONST CHAR16 *CONST main_items[] = {
    L"BIOS", L"System", L"Baseboard", L"Chassis",
    L"Processor", L"Memory"
};

static CONST CHAR16 *CONST action_items[] = {
    L"Write", L"Reset", L"Print", L"Back"
};

static CONST UI_FIELD bios_fields[] = {
    { L"Vendor",       "vendor" },
    { L"Version",      "version" },
    { L"Release Date", "release_date" }
};

static CONST UI_FIELD system_fields[] = {
    { L"Manufacturer", "manufacturer" },
    { L"Product",      "product" },
    { L"Version",      "version" },
    { L"Serial",       "serial" },
    { L"SKU",          "sku" },
    { L"Family",       "family" }
};

static CONST UI_FIELD baseboard_fields[] = {
    { L"Manufacturer", "manufacturer" },
    { L"Product",      "product" },
    { L"Version",      "version" },
    { L"Serial",       "serial" },
    { L"Asset Tag",    "asset_tag" },
    { L"Location",     "location" }
};

static CONST UI_FIELD chassis_fields[] = {
    { L"Manufacturer", "manufacturer" },
    { L"Version",      "version" },
    { L"Serial",       "serial" },
    { L"Asset Tag",    "asset_tag" }
};

static CONST UI_FIELD processor_fields[] = {
    { L"Socket",       "socket" },
    { L"Manufacturer", "manufacturer" },
    { L"Version",      "version" },
    { L"Serial",       "serial" },
    { L"Asset Tag",    "asset_tag" },
    { L"Part Number",  "part_number" }
};

static CONST UI_FIELD memory_fields[] = {
    { L"Device Locator", "device_locator" },
    { L"Bank Locator",   "bank_locator" },
    { L"Manufacturer",   "manufacturer" },
    { L"Serial",         "serial" },
    { L"Asset Tag",      "asset_tag" },
    { L"Part Number",    "part_number" }
};

static
EFI_STATUS
read_key(EFI_INPUT_KEY *key)
{
    EFI_STATUS status;
    UINTN event_index;

    do {
        status = gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &event_index);
        if (EFI_ERROR(status)) {
            return status;
        }
        status = gST->ConIn->ReadKeyStroke(gST->ConIn, key);
    } while (status == EFI_NOT_READY);

    return status;
}

static
EFI_STATUS
choose_menu(
    CONST CHAR16       *title,
    CONST CHAR16       *CONST items[],
    UINTN               count,
    UINTN              *selected
)
{
    EFI_STATUS status;
    EFI_INPUT_KEY key;
    UINTN start;
    UINTN end;

    if (count == 0) {
        return EFI_NOT_FOUND;
    }

    while (TRUE) 
    {
        start = *selected >= 12 ? *selected - 11 : 0;
        end = (count - start > 12) ? start + 12 : count;

        gST->ConOut->ClearScreen(gST->ConOut);
        Print(L"=== %s ===\n\n", title);
        for (UINTN i = start; i < end; i++) {
            Print(L"%s%s\n", i == *selected ? L"> " : L"  ", items[i]);
        }
        Print(L"\nUP/DOWN = Select | ENTER = Open | ESC = Back\n");

        status = read_key(&key);
        if (EFI_ERROR(status)) {
            return status;
        }
        if (key.ScanCode == SCAN_ESC) {
            return EFI_ABORTED;
        }
        if (key.ScanCode == SCAN_UP) {
            *selected = (*selected == 0) ? count - 1 : *selected - 1;
        }
        else if (key.ScanCode == SCAN_DOWN) {
            *selected = (*selected + 1) % count;
        }
        else if (key.UnicodeChar == L'\r') {
            return EFI_SUCCESS;
        }
    }
}

static
VOID
pause_screen(VOID)
{
    EFI_INPUT_KEY key;
    Print(L"\nPress any key to continue...\n");
    read_key(&key);
}

// Simple ASCII input. We use ASCII because the existing SMBIOS engine takes CHAR8.
static
EFI_STATUS
read_value(CHAR8 value[256])
{
    EFI_STATUS status;
    EFI_INPUT_KEY key;
    UINTN length = 0;

    value[0] = '\0';
    while (TRUE) 
    {
        gST->ConOut->ClearScreen(gST->ConOut);
        Print(L"=== Write SMBIOS string ===\n\n");
        Print(L"New value: %a\n\n", value);
        Print(L"ENTER = Save | ESC = Cancel | BACKSPACE = Delete\n");

        status = read_key(&key);
        if (EFI_ERROR(status)) {
            return status;
        }
        if (key.ScanCode == SCAN_ESC) {
            return EFI_ABORTED;
        }
        if (key.UnicodeChar == L'\r') {
            return length == 0 ? EFI_INVALID_PARAMETER : EFI_SUCCESS;
        }
        if (key.UnicodeChar == L'\b') 
        {
            if (length > 0) {
                value[--length] = '\0';
            }
        }
        else if (key.UnicodeChar >= L' ' && key.UnicodeChar <= L'~' && length < 255)
        {
            value[length++] = (CHAR8)key.UnicodeChar;
            value[length] = '\0';
        }
    }
}

static
EFI_STATUS
field_actions(CONST CHAR16 *title, CONST CHAR8 *path)
{
    EFI_STATUS status;
    EFI_INPUT_KEY key;
    CHAR8 value[256];
    UINTN selected = 0;

    while (TRUE) 
    {
        status = choose_menu(title, action_items,
                             ARRAY_SIZE(action_items), &selected);
        if (status == EFI_ABORTED) {
            return EFI_SUCCESS;
        }
        if (EFI_ERROR(status)) {
            return status;
        }
        if (selected == 3) {
            return EFI_SUCCESS;
        }

        if (selected == 0) // Write
        { 
            status = read_value(value);
            if (status == EFI_ABORTED) {
                continue;
            }
            if (!EFI_ERROR(status)) {
                status = smbios_set(path, value);
            }
        }
        else if (selected == 1) { // Reset NVRAM override
            status = smbios_reset(path);
        }
        else { // Print one field
            status = smbios_get(path, value, sizeof(value));
        }

        gST->ConOut->ClearScreen(gST->ConOut);
        Print(L"=== %s ===\n\n", title);
        if (selected == 2 && !EFI_ERROR(status)) {
            Print(L"Current value: %a\n", value);
        }
        else {
            Print(L"%s: %r\n", action_items[selected], status);
        }
        if (selected == 1 && !EFI_ERROR(status)) {
            Print(L"Override removed. Original value returns after reboot.\n");
        }
        if (selected == 0 && !EFI_ERROR(status)) {
            Print(L"\nValue updated and saved to NVRAM.\n");
        }
        Print(L"\n");
        status = read_key(&key);
        if (EFI_ERROR(status)) {
            return status;
        }
    }
}

static
EFI_STATUS
choose_memory_instance(UINTN *instance)
{
    UINTN count;
    EFI_STATUS status = smbios_memory_count(&count);
    EFI_INPUT_KEY key;

    if (EFI_ERROR(status)) {
        return status;
    }
    if (count == 0)
    {
        gST->ConOut->ClearScreen(gST->ConOut);
        Print(L"No Type 17 memory records found.\n");
        pause_screen();
        return EFI_ABORTED;
    }

    *instance = 0;
    while (TRUE) 
    {
        gST->ConOut->ClearScreen(gST->ConOut);

        Print(L"=== Memory ===\n\n");
        Print(L"> Memory[%u] of %u records\n\n", *instance, count);
        Print(L"UP/DOWN = Change slot | ENTER = Open | ESC = Back\n");

        status = read_key(&key);
        if (EFI_ERROR(status)) {
            return status;
        }
        if (key.ScanCode == SCAN_ESC) {
            return EFI_ABORTED;
        }
        if (key.ScanCode == SCAN_UP) {
            *instance = (*instance == 0) ? count - 1 : *instance - 1;
        }
        else if (key.ScanCode == SCAN_DOWN) {
            *instance = (*instance + 1) % count;
        }
        else if (key.UnicodeChar == L'\r') {
            return EFI_SUCCESS;
        }
    }
}

static
EFI_STATUS
show_category(UINTN category)
{
    CONST UI_FIELD *fields;
    CONST CHAR8 *prefix;
    UINTN count;
    UINTN memory_instance = 0;
    UINTN selected = 0;
    EFI_STATUS status;
    CONST CHAR16 *labels[6];
    CHAR16 title[96];
    CHAR16 field_title[128];
    CHAR8 path[64];

    switch (category) 
    {
        case 0:
            fields = bios_fields; count = ARRAY_SIZE(bios_fields); prefix = "bios";
            break;
        case 1:
            fields = system_fields; count = ARRAY_SIZE(system_fields); prefix = "system";
            break;
        case 2:
            fields = baseboard_fields; count = ARRAY_SIZE(baseboard_fields); prefix = "baseboard";
            break;
        case 3:
            fields = chassis_fields; count = ARRAY_SIZE(chassis_fields); prefix = "chassis";
            break;
        case 4:
            fields = processor_fields; count = ARRAY_SIZE(processor_fields); prefix = "processor";
            break;
        default:
            fields = memory_fields; count = ARRAY_SIZE(memory_fields); prefix = "memory";
            status = choose_memory_instance(&memory_instance);
            if (status == EFI_ABORTED) {
                return EFI_SUCCESS;
            }
            if (EFI_ERROR(status)) {
                return status;
            }
            break;
    }

    if (category == 5) {
        UnicodeSPrint(title, sizeof(title), L"Memory[%u]", memory_instance);
    }
    else {
        UnicodeSPrint(title, sizeof(title), L"%s", main_items[category]);
    }

    for (UINTN i = 0; i < count; i++) {
        labels[i] = fields[i].label;
    }

    while (TRUE) 
    {
        status = choose_menu(title, labels, count, &selected);
        if (status == EFI_ABORTED) {
            return EFI_SUCCESS;
        }
        if (EFI_ERROR(status)) {
            return status;
        }

        if (category == 5) {
            AsciiSPrint(path, sizeof(path), "smbios.memory[%u].%a",
                        memory_instance, fields[selected].name);
        }
        else {
            AsciiSPrint(path, sizeof(path), "smbios.%a.%a",
                        prefix, fields[selected].name);
        }

        UnicodeSPrint(field_title, sizeof(field_title), L"%s / %s", title, fields[selected].label);
        status = field_actions(field_title, path);
        if (EFI_ERROR(status)) {
            return status;
        }
    }
}

EFI_STATUS
EFIAPI
run_ui(VOID)
{
    UINTN selected = 0;
    EFI_STATUS status;

    while (TRUE) 
    {
        status = choose_menu(L"UEFI SMBIOS EDITOR", main_items,
                             ARRAY_SIZE(main_items), &selected);
        if (status == EFI_ABORTED) {
            return EFI_SUCCESS;
        }
        if (EFI_ERROR(status)) {
            return status;
        }
        if (selected == 7) {
            return EFI_SUCCESS;
        }
        if (selected == 6) {
            gST->ConOut->ClearScreen(gST->ConOut);
            run_console();
        }
        else 
        {
            status = show_category(selected);
            if (EFI_ERROR(status)) {
                return status;
            }
        }
    }
}
