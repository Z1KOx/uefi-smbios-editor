/**  
  Copyright (c) 2026, Z1KOx. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include "console.h"
#include "smbios.h"

static
VOID
handle_command(
    CONST CHAR16 *buffer
)
{
    CHAR8  line[256];
    CHAR8 *eq, *end;

    UnicodeStrToAsciiStrS(buffer, line, sizeof(line));
    
    if (AsciiStrnCmp(line, "smbios.reset ", 13) == 0)
    {
        Print(L"%r\n", smbios_reset(line + 13));
        return;
    }

    eq = AsciiStrStr(line, " = \"");
    end = line + AsciiStrLen(line) - 1;

    if (eq == NULL || end <= eq + 4 || *end != '"')
    {
        Print(L"Unknown command\n");
        return;
    }

    *eq = '\0';
    *end = '\0';

    Print(L"%r\n", smbios_set(line, eq + 4));
}

VOID
EFIAPI
run_console(
    VOID
)
{
    EFI_INPUT_KEY key;
    EFI_STATUS    status;
    CHAR16        buffer[256];
    UINTN         index = 0;

    SetMem(buffer, sizeof(buffer), 0);

    Print(L"=== UEFI-SMBIOS-EDITOR ===\n");

    while (TRUE)
    {
        status = gST->ConIn->ReadKeyStroke(gST->ConIn, &key);

        if (!EFI_ERROR(status))
        {
            if (key.ScanCode == SCAN_ESC) {
                break;
            }

            if (key.UnicodeChar != 0)
            {
                if (key.UnicodeChar == L'\r' || key.UnicodeChar == L'\n') // Enter
                {
                    Print(L"\n");

                    if (StrCmp(buffer, L"smbios.print") == 0) {
                        smbios_print_info();
                    }
                    else if (buffer[0] != L'\0') { 
                        handle_command(buffer);
                    }

                    SetMem(buffer, sizeof(buffer), 0);
                    index = 0;
                }
                else if (key.UnicodeChar == 8) // Backspace
                {
                    if (index > 0)
                    {
                        index--;
                        buffer[index] = L'\0';
                        Print(L"\b \b");
                    }
                }
                else
                {
                    if (index < (sizeof(buffer) / sizeof(CHAR16)) - 1)
                    {
                        buffer[index] = key.UnicodeChar;
                        index++;
                        buffer[index] = L'\0';

                        Print(L"%c", key.UnicodeChar);
                    }
                }
            }
        }
    }
}