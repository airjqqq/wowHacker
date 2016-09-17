//
// Created by Administrator on 2016/7/24.
//

#include "memory.h"
#include <tlhelp32.h>


bool str2hex(const char *szMemory, byte bData[], bool bMask[]) {
    char cBuffer[3];
    int iData = 0;
    for (int i = 0; i < lstrlen(szMemory); i += 3) {
        for (int j = 0; j < 3; j++) {
            cBuffer[j] = szMemory[i + j];
        }
        if (cBuffer[2] == '\0' || cBuffer[2] == ' ') {
            cBuffer[2] = '\0';
            if (strcmp(cBuffer, "??") == 0) {
                bData[iData] = 0;
                bMask[iData] = false;
            }
            else {
                byte bHex = (byte) strtoul(cBuffer, NULL, 16);
                bData[iData] = bHex;
                bMask[iData] = true;
            }
        }
        else {
            return false;
        }
        iData++;
    }
    return true;
}


bool GetBaseAddress(DWORD dwPID, LPCVOID *lpBaseAddress, SIZE_T *nBaseSize) {
    MODULEENTRY32 me32;

    // Take a snapshot of all modules in the specified process.
    HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
    if (hModuleSnap == INVALID_HANDLE_VALUE) {
        return (false);
    }

    // Set the size of the structure before using it.
    me32.dwSize = sizeof(MODULEENTRY32);

    // Retrieve information about the first module,
    // and exit if unsuccessful
    if (!Module32First(hModuleSnap, &me32)) {
        CloseHandle(hModuleSnap);           // clean the snapshot object
        return (false);
    }
    *lpBaseAddress = (LPBYTE) me32.modBaseAddr;
    *nBaseSize = me32.modBaseSize;

    CloseHandle(hModuleSnap);
    return (TRUE);
}

SIZE_T read(HANDLE hProcess, LPVOID lpAddress, byte *buffer, SIZE_T size) {
    DWORD oldProtection;
    SIZE_T nRead;
    VirtualProtectEx(hProcess, (LPVOID) lpAddress, size, PAGE_EXECUTE_READWRITE, &oldProtection);
    BOOL readSuccess = ReadProcessMemory(hProcess, lpAddress, buffer, size, &nRead);
    VirtualProtectEx(hProcess, (LPVOID) lpAddress, size, oldProtection, NULL);
    return nRead;
}

#define BLOCK_SIZE 4096

void *findMemory(HANDLE hProcess, const char *szMemory, long start=0)
{
    int nData = (lstrlen(szMemory) + 1) / 3;
    byte bData[1024];
    bool bMask[1024];
    if (!str2hex(szMemory, bData, bMask)) {
        return NULL;
    }
    DWORD processId = GetProcessId(hProcess);
    LPCVOID lpBaseAddress = NULL;
    SIZE_T nBaseSize = 0;
    if (GetBaseAddress(processId, &lpBaseAddress, &nBaseSize)) {
        byte buffer[BLOCK_SIZE];
        long lastBlockBase = -1;
        int matched = 0;
        for (int offset = start; offset < nBaseSize; offset++) {
            if (lastBlockBase == -1 || !(offset >= lastBlockBase && offset < lastBlockBase + BLOCK_SIZE)) {
                lastBlockBase = offset / BLOCK_SIZE * BLOCK_SIZE;
                read(hProcess, (LPBYTE)lpBaseAddress + lastBlockBase, buffer, BLOCK_SIZE);
            }
            if (!bMask[matched] || bData[matched] == buffer[(offset - lastBlockBase)]) {
                matched++;
            } else {
                offset -= matched;
                matched = 0;
            }
            if (matched == nData) {
                return (void *) ((LPBYTE)lpBaseAddress + offset + 1 - matched);
            }
        }
    }
    return NULL;
}

bool modifyMemory(HANDLE hProcess, void* address, const char* szMemory){

    int nData = (lstrlen(szMemory) + 1) / 3;
    byte bData[1024];
    bool bMask[1024];
    if (!str2hex(szMemory, bData, bMask)) {
        return false;
    }
    DWORD processId = GetProcessId(hProcess);
    LPCVOID lpBaseAddress = NULL;
    SIZE_T nBaseSize = 0;
    if (GetBaseAddress(processId, &lpBaseAddress, &nBaseSize)) {
        DWORD oldProtection;
        VirtualProtectEx(hProcess, address, (SIZE_T) nData, PAGE_EXECUTE_READWRITE, &oldProtection);
        for (int i = 0; i < nData; i++) {
            if (bMask[i]) {
                if (!WriteProcessMemory(hProcess, (byte*)address + i, bData + i, 1, NULL)) {
                    VirtualProtectEx(hProcess, address, (SIZE_T) nData, oldProtection, NULL);
                    return false;
                }
            }
        }
        VirtualProtectEx(hProcess, address, (SIZE_T) nData, oldProtection, NULL);
        return true;
    }
    return false;
}