#pragma once

uintptr_t Scan(HMODULE moduleBase, LPCSTR pattern);
uintptr_t ExtractQwordTarget(uintptr_t instructionAddress);
