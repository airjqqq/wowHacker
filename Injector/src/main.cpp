#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stack>
#include "memory.h"
#include "LoadLibraryR.h"
#include <ctime>
//#include "ExecThread.h"

using namespace std;




BOOL findAndModifyMemory(HANDLE hProcess, const char *from, const char *to) {
    void *address = findMemory(hProcess, from);
    if (address != NULL) {
        if (modifyMemory(hProcess, address, to)) {
            return true;
        }
    }
    return false;
}

//
//BOOL insertFcn(HANDLE hProcess, LPCVOID lpBaseAddress,int nSize, const char* name, void* fcn)
//{
//    void (*insert)(const char *, void *);
//    insert = (void (*)(const char *, void *)) findMemory(hProcess, lpBaseAddress, nSize, ("48 89 5C 24 08 57 48 83 EC 20 48 8B 3D 87 64 3F"));
//    insert(name, fcn);
//}

LPTHREAD_START_ROUTINE AllocWritePath(HANDLE hTargetProcHandle, LPCSTR dllPath, LPVOID *lpExecParam) {

	unsigned int writeLen = 0;
	LPVOID lpDllAddr = NULL;
	LPVOID lpWriteVal = NULL;
	LPVOID loadLibAddr = NULL;
	lpDllAddr = VirtualAllocEx(hTargetProcHandle, NULL, strlen(dllPath), MEM_RESERVE|MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (WriteProcessMemory(hTargetProcHandle, lpDllAddr, dllPath, strlen(dllPath), NULL) == 0) {
		return NULL;
	}

	*lpExecParam = (LPVOID *)lpDllAddr;
	loadLibAddr = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "LoadLibraryA");
	if (loadLibAddr == NULL) {
		return NULL;
	}
	return (LPTHREAD_START_ROUTINE) loadLibAddr;

}



int inject(HANDLE hProcess) {


	LPTHREAD_START_ROUTINE lpStartExecAddr = NULL;
	LPVOID lpExecParam = NULL;
	HANDLE hTargetProcHandle = hProcess;
	TCHAR tcDllPath[512] = TEXT("");
	char* filename = "../wow/x64/Debug/wow.dll";
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];

	time (&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer,80,"%d-%m-%Y-%I-%M-%S",timeinfo);

	char descFileName[512];
	sprintf_s(descFileName,"../wow/x64/Debug/wow-%s.dll",buffer);
	CopyFileA(filename,descFileName,false);

	//char* filename = "libwow.dll";
	GetFullPathName(descFileName, 512, tcDllPath, NULL);
	//GetFullPathName(filename, 512, tcDllPath, NULL);
	lpStartExecAddr = AllocWritePath(hTargetProcHandle, tcDllPath, &lpExecParam);
	HANDLE rThread;
	//rThread = bCreateRemoteThread(hTargetProcHandle, lpStartExecAddr, lpExecParam);
	rThread = CreateRemoteThread(hTargetProcHandle, NULL, 0, lpStartExecAddr, lpExecParam, 0, NULL);
	if (rThread == NULL) {
		printf("\n[!] CreateRemoteThread Failed! [%d] Exiting....\n", GetLastError());
		return -1;
	} 
	WaitForSingleObject(rThread, INFINITE);


    //HANDLE hFile = NULL;
    //HANDLE hModule = NULL;
    //HANDLE hToken = NULL;
    //LPVOID lpBuffer = NULL;
    //DWORD dwLength = 0;
    //DWORD dwBytesRead = 0;
    //DWORD dwProcessId = 0;
    //TOKEN_PRIVILEGES priv = {0};

    //do {
    //    // Usage: inject.exe [pid] [dll_file]



    //    const char *cpDllFile = "libwow.dll";
    //    hFile = CreateFileA(cpDllFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    //    if (hFile == INVALID_HANDLE_VALUE)
    //        printf("Failed to open the DLL file");

    //    dwLength = GetFileSize(hFile, NULL);
    //    if (dwLength == INVALID_FILE_SIZE || dwLength == 0)
    //        printf("Failed to get the DLL file size");

    //    lpBuffer = HeapAlloc(GetProcessHeap(), 0, dwLength);
    //    if (!lpBuffer)
    //        printf("Failed to get the DLL file size");

    //    if (ReadFile(hFile, lpBuffer, dwLength, &dwBytesRead, NULL) == FALSE)
    //        printf("Failed to alloc a buffer!");

    //    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
    //        priv.PrivilegeCount = 1;
    //        priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    //        if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &priv.Privileges[0].Luid))
    //            AdjustTokenPrivileges(hToken, FALSE, &priv, 0, NULL, NULL);

    //        CloseHandle(hToken);
    //    }
    //    if (!hProcess)
    //        printf("Failed to open the target process");

    //    LPCVOID lpBaseAddress = NULL;
    //    SIZE_T nBaseSize = 0;
    //    DWORD processId = GetProcessId(hProcess);
    //    if (GetBaseAddress(processId, &lpBaseAddress, &nBaseSize)) {
    //        hModule = LoadRemoteLibraryR(hProcess, lpBuffer, dwLength, (LPVOID) lpBaseAddress);
    //        if (!hModule)
    //            printf("Failed to inject the DLL");

    //        printf("\n[+] Injected the '%s' DLL into process %d.", cpDllFile, dwProcessId);

    //        WaitForSingleObject(hModule, -1);
    //    }

    //} while (0);

    //if (lpBuffer)
    //    HeapFree(GetProcessHeap(), 0, lpBuffer);

    //if (hProcess)
    //    CloseHandle(hProcess);

    return 0;
}

void printOffset(HANDLE hProcess, char* name, char* bytes){

	long long base = 0;
	DWORD processId = GetProcessId(hProcess);
	LPCVOID lpBaseAddress = NULL;
	SIZE_T nBaseSize = 0;
	if (GetBaseAddress(processId, &lpBaseAddress, &nBaseSize)) {
		base = (long)lpBaseAddress;
	}
	long long addr = (long long)findMemory(hProcess,bytes);
	long offset;
	if (addr){
		offset = addr-base;
	}
	else{
		offset = 0;
	}
	printf("long %s = 0x%07lx; //%s\n",name,offset,bytes);
}


BOOL hackProcess() {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);          // clean the snapshot object
        return false;
    }
    do {
        if (lstrcmp(pe32.szExeFile, ("WowT-64.exe")) == 0 || lstrcmp(pe32.szExeFile, ("Wow-64.exe")) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
			if (hProcess == NULL){

			}
			else {
				long long base = 0;
				DWORD processId = GetProcessId(hProcess);
				LPCVOID lpBaseAddress = NULL;
				SIZE_T nBaseSize = 0;
				if (GetBaseAddress(processId, &lpBaseAddress, &nBaseSize)) {
					base = (long)lpBaseAddress;
				}
				// 				long offsetFrameScript_RegisterFunction = 0x0B90A0; //48 89 5C 24 08 57 48 83 EC 20 48 8B 3D ?? ?? ?? ?? 48 8B D9 45 33 C0 48 8B CF E8
				// 				long offsetClntObjMgrEnumVisibleObjects = 0x42B630; //48 89 74 24 10 57 48 83 EC 20 48 8B 05 ?? ?? ?? ?? 48 8B F1 48 8B FA 48 8B 88 A0 01 00 00 F6 C1
				// 				long offsetGetObjectPtrByGUID = 0x42C040; //40 53 48 83 EC 30 48 83 3D ?? ?? ?? ?? ?? 8B DA
				// 				long offsetGetGUIDByUnitId = 0x64ECD0; //48 89 5C 24 18 48 89 74 24 20 48 89 4C 24 08 55 57 41 57 48 8B EC 48 83
				// 				long offsetGetNameByObjectPtr = 0x47FD20; //48 89 5C 24 08 48 89 7C 24 10 55 48 8B EC 48 81 EC 80 00 00 00 48 8B FA 48 8B D9 45 85 C0 74 20
				// 				long offsetGetGUIDStringByGUID = 0xA77EA0; //48 89 5C 24 18 48 89 74 24 20 41 56 48 83 EC 20 41 8B F0 48 8B DA 4C 8B F1 45 85 C0 75 13 33 C0
				// 				long offsetGetGUIDByGUIDString = 0xA77470; //48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 54 41 55 41 56 41 57 48 83 EC 20 33 C0 48 8B
				// 				long offsetInteractByGUID = 0x6C0DB0; //40 57 48 83 EC 30 48 8B 05 ?? ?? ?? ?? 48 8B F9 48 85 C0 75 3D 48 8D 4C 
				// 				long offsetGetWorldBase = 0x712F30; //48 8B 05 ?? ?? ?? ?? 48 85 C0 74 08 48 8B 80 38
				// 				long offsetPreHeal = 0x47D8B0; //40 56 48 83 EC 30 48 8B 01 48 8B F1 FF 90 60 03
				// 				long offsetAbsorb = 0x47EBE0;		//40 53 48 83 EC 20 48 8B 01 48 8B D9 FF 90 60 03 00 00 84 C0 74 08 33 C0 48 83 C4 20 5B C3 4C 8D 4C 24 30 4C 8D 05 B6 8C
				// 				long offsetHealAbsorb = 0x47EC30;	//40 53 48 83 EC 20 48 8B 01 48 8B D9 FF 90 60 03 00 00 84 C0 74 08 33 C0 48 83 C4 20 5B C3 4C 8D 4C 24 30 4C 8D 05 66 8C
				// 				long offsetGetPlayerFacing = 0x642AB0; //40 53 48 83 EC 30 48 8B D9 E8 ?? ?? ?? ?? 48 85 C0 75 16 0F 57 C9 48 8B

				// 				long offsetLua_pushstring = 0x199610; //48 85 D2 75 1C 48 8B 51 18 48 8B 05 ?? ?? ?? ?? 48 89 42 10 C7 42 08 00
				// 				long offsetLua_pushnumber = 0x1995F0; //48 8B 51 18 48 8B 05 ?? ?? ?? ?? 48 89 42 10 C7 42 08 03 00 00 00 F2 0F
				// 				long offsetLua_pushboolean = 0x199330; //48 8B 05 ?? ?? ?? ?? 4C 8B 41 18 49 89 40 10 33
				// 				long offsetLua_pushnil = 0x1995D0; // 48 8B 51 18 48 8B 05 ?? ?? ?? ?? 48 89 42 10 C7 42 08 00 00 00 00 48 83 41 18 18 C3 CC
				// 
				// 				long offsetLua_checkstack = 0x199030; //48 83 EC 28 E8 57 F2 FF FF 48 8D 0D ?? ?? ?? ??
				// 				long offsetLua_tolstring = 0x19A0C0; //48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 8B D8 8B F2 48 8B F9 E8 B4 E1 FF FF 4C 8B D0 83
				// 				long offsetLua_tointeger = 0x19A040; //40 57 48 83 EC 40 48 8B F9 E8 42 E2 FF FF 83 78
				// 				long offsetLua_tonumber = 0x19A160; //48 83 EC 48 E8 27 E1 FF FF 83 78 08 03 74 1A 48
				// 
				// 				long offsetLua_getTop = 0x198CF0; //48 8B 51 18 48 B8 AB AA AA AA AA AA AA 2A 48 2B
				// 				long offsetLua_setTop = 0x199F90; //85 D2 78 53 48 63 C2 48 8D 14 40 4C 8D 04 D5 00
				if (false){
					printOffset(hProcess, "offsetFrameScript_RegisterFunction", "48 89 5C 24 08 57 48 83 EC 20 48 8B 3D ?? ?? ?? ?? 48 8B D9 45 33 C0 48 8B CF E8");
					printOffset(hProcess, "offsetClntObjMgrEnumVisibleObjects", "48 89 74 24 10 57 48 83 EC 20 48 8B 05 ?? ?? ?? ?? 48 8B F1 48 8B FA 48 8B 88 A0 01 00 00 F6 C1");
					printOffset(hProcess, "offsetGetObjectPtrByGUID",			"40 53 48 83 EC 30 48 83 3D ?? ?? ?? ?? ?? 8B DA");
					printOffset(hProcess, "offsetGetGUIDByUnitId",				"48 89 5C 24 18 48 89 74 24 20 48 89 4C 24 08 55 57 41 57 48 8B EC 48 83");
					printOffset(hProcess, "offsetGetNameByObjectPtr",			"48 89 5C 24 08 48 89 7C 24 10 55 48 8B EC 48 81 EC 80 00 00 00 48 8B FA 48 8B D9 45 85 C0 74 20");

					printOffset(hProcess, "offsetGetGUIDStringByGUID",			"48 89 5C 24 18 48 89 74 24 20 41 56 48 83 EC 20 41 8B F0 48 8B DA 4C 8B F1 45 85 C0 75 13 33 C0");
					printOffset(hProcess, "offsetGetGUIDByGUIDString",			"48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 54 41 55 41 56 41 57 48 83 EC 20 33 C0 48 8B");
					printOffset(hProcess, "offsetInteractByGUID",				"40 57 48 83 EC 30 48 8B 05 ?? ?? ?? ?? 48 8B F9 48 85 C0 75 3D 48 8D 4C ");
					printOffset(hProcess, "offsetGetWorldBase",					"48 8B 05 ?? ?? ?? ?? 48 85 C0 74 08 48 8B 80 38");
					printOffset(hProcess, "offsetPreHeal",						"40 56 48 83 EC 30 48 8B 01 48 8B F1 FF 90 60 03");

					printOffset(hProcess, "offsetAbsorb", 						"40 53 48 83 EC 20 48 8B 01 48 8B D9 FF 90 60 03 00 00 84 C0 74 08 33 C0 48 83 C4 20 5B C3 4C 8D 4C 24 30 4C 8D 05 ?? ?? ?? ?? BA 45 00 00 00 48");
					printOffset(hProcess, "offsetHealAbsorb", 					"40 53 48 83 EC 20 48 8B 01 48 8B D9 FF 90 60 03 00 00 84 C0 74 08 33 C0 48 83 C4 20 5B C3 4C 8D 4C 24 30 4C 8D 05 ?? ?? ?? ?? BA 2D 01 00 00 48");

					printOffset(hProcess, "offsetUnitCanAttack",				"48 89 5C 24 20 57 48 83 EC 20 48 8B 41 10 45 0F");
					printOffset(hProcess, "offsetUnitCanAssist",				"48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 41 0F B6 F0 48 8B DA 48 8B F9 45 84 C9 75 1C 48 8B");
					printOffset(hProcess, "offsetHandleTerrainClick",			"40 53 48 83 EC 20 83 79 1C 04 48 8B D9 74 09 83");

					printOffset(hProcess, "offsetGetPlayerFacing", 				"40 53 48 83 EC 30 48 8B D9 E8 ?? ?? ?? ?? 48 85 C0 75 16 0F 57 C9 48 8B");


					printOffset(hProcess, "offsetLua_pushstring", 				"48 85 D2 75 1C 48 8B 51 18 48 8B 05 ?? ?? ?? ?? 48 89 42 10 C7 42 08 00");
					printOffset(hProcess, "offsetLua_pushnumber", 				"48 8B 51 18 48 8B 05 ?? ?? ?? ?? 48 89 42 10 C7 42 08 03 00 00 00 F2 0F");
					printOffset(hProcess, "offsetLua_pushboolean", 				"48 8B 05 ?? ?? ?? ?? 4C 8B 41 18 49 89 40 10 33");
					printOffset(hProcess, "offsetLua_pushnil", 					"48 8B 51 18 48 8B 05 ?? ?? ?? ?? 48 89 42 10 C7 42 08 00 00 00 00 48 83 41 18 18 C3 CC");

					printOffset(hProcess, "offsetLua_checkstack", 				"48 83 EC 28 E8 57 F2 FF FF 48 8D 0D ?? ?? ?? ??");
					printOffset(hProcess, "offsetLua_tolstring", 				"48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 8B D8 8B F2 48 8B F9 E8 B4 E1 FF FF 4C 8B D0 83");
					printOffset(hProcess, "offsetLua_tointeger", 				"40 57 48 83 EC 40 48 8B F9 E8 42 E2 FF FF 83 78");
					printOffset(hProcess, "offsetLua_tonumber", 				"48 83 EC 48 E8 27 E1 FF FF 83 78 08 03 74 1A 48");

					printOffset(hProcess, "offsetLua_getTop", 					"48 8B 51 18 48 B8 AB AA AA AA AA AA AA 2A 48 2B");
					printOffset(hProcess, "offsetLua_setTop", 					"85 D2 78 53 48 63 C2 48 8D 14 40 4C 8D 04 D5 00");

				}
				//6B23CB
                if (findAndModifyMemory(hProcess, ("33 C0 33 C9 8B D0 E8 ?? ?? ?? ?? 33 C0 48 83 C4"),
                                        ("33 C0 33 C9 8B D0 90 90 90 90 90 FF C0 48 83 C4"))) {
                    printf(("PLUA new enable\n"));
                }
				// function start = 639440
				// 639704
				if (findAndModifyMemory(hProcess, ("0F 2F C6 F3 0F 11 83 4C 02 00 00 76 0D"),
					("0F 2F C6 F3 0F 11 83 4C 02 00 00 EB 0D"))) {
						printf(("Zoom out limit disabled\n"));
				}
				//function start = 6CD9F0
				//
				if (findAndModifyMemory(hProcess, ("8D 81 20 6C FB FF 85 C0 78 58 8D 81 C0 88 E4 FF"),
					("8D 81 20 6C FB FF 85 C0 EB 58 8D 81 C0 88 E4 FF"))) {
						printf(("Anti AFK enable\n"));

						if (nBaseSize == 27656192) {
							inject(hProcess);
							printf(("Inject succeed\n"));
						}
				}
				//6b27a0
				//31 C0 FF C0 C3
				if (findAndModifyMemory(hProcess, ("40 53 48 83 EC 20 48 83 3D 42 ?? ?? ?? ?? 48 63"),
					("31 C0 FF C0 C3 CC 48 83 3D 42 ?? ?? ?? ?? 48 63"))) {
						printf(("Combat lock\n"));
				}
				CloseHandle(hProcess);

				
				
				
//                 if (findAndModifyMemory(hProcess, ("33 C0 33 C9 8B D0 E8 CA 9F 01 00 33 C0 48 83 C4"),
//                                         ("33 C0 33 C9 8B D0 E8 CA 9F 01 00 FF C0 48 83 C4"))) {
//                     printf(("\nPLUA beta enable"));
//                 }
//                 if (findAndModifyMemory(hProcess, ("75 0C 48 8B 4C 24 48 48 39 4C 24 38 74 22 48 85"),
//                                         ("90 90 48 8B 4C 24 48 48 39 4C 24 38 EB 22 48 85"))) {
//                     printf(("\nUnitPosition enable"));
//                 }
// 
//                 if (findAndModifyMemory(hProcess, ("48 8B 7C 24 70 0F 11 44 24 40 74 11 33 D2 33 C9"),
//                                         ("48 8B 7C 24 70 0F 11 44 24 40 EB 11 33 D2 33 C9"))) {
//                     printf(("\nInact enable"));
//                 }
//                 if (findAndModifyMemory(hProcess, ("18 C1 EA 16 F6 C2 01 75 09 E8 7D 02 00 00 84 C0 74 07 B0 01 48 83 C4 28 C3 32 C0 48 83 C4 28 C3"),
//                                         ("18 C1 EA 16 F6 C2 01 75 09 E8 7D 02 00 00 84 C0 74 07 B0 01 48 83 C4 28 C3 EB F7 48 83 C4 28 C3"))) {
//                     printf(("\nC_scrpt enable"));
//                 }


//                    void * fcn = (void *) findMemory(hProcess, lpBaseAddress, nBaseSize, ("40 53 48 83 EC 40 BA 01 00 00 00 48 8B D9 E8 3D"));
//                    if (fcn != NULL) {
//                        insertFcn(hProcess, lpBaseAddress, nBaseSize, "tprint", fcn);
//
//                    }

            }
        }

    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return true;
}

int main(void) {
    printf(("started\n"));

    hackProcess();
    while(1)
    {
        Sleep(1000);
        hackProcess();
	}
	printf(("\n"));
    system("pause");
    return 0;
}
