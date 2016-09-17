//
// Created by Administrator on 2016/7/24.
//

#ifndef INJECTOR_MEMORY_H
#define INJECTOR_MEMORY_H
#include <windows.h>

bool GetBaseAddress(DWORD dwPID, LPCVOID *lpBaseAddress, SIZE_T *nBaseSize);
void* findMemory(HANDLE hProcess, const char* szMemory, long start);
bool modifyMemory(HANDLE hProcess, void* address, const char* szMemory);


#endif //INJECTOR_MEMORY_H
