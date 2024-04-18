// Inject.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <comdef.h>

using namespace std;

int findProcessID(wchar_t processName[]);


int wmain(int argc, wchar_t* argv[])
{
	// Declare a wchar_t array to put process name.
	//wchar_t processName[100] = L"Tester.exe";
	wchar_t* processName= argv[1];

	printf("1. Get specified process id\n");
	int pid = findProcessID(processName);
	if (pid < 0)
	{
		printf("\t[FAILURE]  Not found the process \"%ls\".\n", processName);
		system("PAUSE");
		return 0;
	}
	printf("\t[SUCCESS]  The process \"%ls\" is found. And PID is %d.\n", processName, pid);

	printf("2. Get the handle to the process\n");
	HANDLE pHandle = OpenProcess(
		PROCESS_CREATE_THREAD |        // Permission to create threads
		PROCESS_QUERY_INFORMATION |    // Permission to get information about the process, such as its exit code, priority.
		PROCESS_VM_OPERATION |         // Permission to manipulate the memory in the process. (VirtualProtectEx and WriteProcessMemory can be used.)
		PROCESS_VM_READ |              // Permission to read the memory in the process. (ReadProcessMemory can be used.)
		PROCESS_VM_WRITE,              // Permission to write the memory in the process. (WriteProcessMemory can be used.)
		false, pid);
	if (pHandle == NULL) {
		printf("\t[FAILURE]  Unable to obtain the handle to the process (%ls, %d).\n", processName, pid);
		system("PAUSE");
		return 0;
	}
	printf("\t[SUCCESS]  The handle to the process (%ls, %d)  is 0x%p.\n", processName, pid, pHandle);

	printf("3. Allocate the memory for the dll path\n");
	// Use full path to dll.
	//const char *dllPath = "D:\\Wang\\github\\DetourHook\\Debug\\DetourHook.dll";
	_bstr_t b(argv[2]);
	const char *dllPath = b;

	LPVOID dllAddr = VirtualAllocEx(
		pHandle,                    // The handle to a process.
		NULL,                       // The pointer that specifies a desired starting address for the region of pages.
		strlen(dllPath),            // The size of the memory we need.
		MEM_RESERVE | MEM_COMMIT,   // Allocation type.
		PAGE_EXECUTE_READWRITE);    // The memory protection for the region of pages to be allocated.
	if (dllAddr == NULL) {
		printf("\t[FAILURE]  Unable to allocate memory.\n");
		system("PAUSE");
		return 0;
	}
	printf("\t[SUCCESS]  Successfully allocate memory at 0x%p.\n", dllAddr);

	printf("4. Write the dll path to the memory\n");
	// WriteProcessMemory(
	//	pHandle,         // The handle to a process.
	//	dllAddr,         // A pointer to the base address in the specified process to which data is written.
	//	dllPath,         // A pointer to the buffer that contains data to be written in the dllAddr.
	//	strlen(dllPath), // The number of bytes to be written to the specified process.
	//	NULL)            // If this is NULL, the parameter is ignored.
	if (WriteProcessMemory(pHandle, dllAddr, dllPath, strlen(dllPath), NULL) == FALSE) {
		printf("\t[FAILURE]  Unable to write the dll path into memory.\n");
		system("PAUSE");
		return 0;
	}
	printf("\t[SUCCESS]  The dll path is successfully written into the memory.\n");

	// Get the address of the function "LoadLibraryA".
	printf("5. Get address of \"LoadLibraryA\"\n");
	// GetModuleHandle : Retrieves a module handle for the specified module. 
	// The module must have been loaded by the calling process.
	// GetProcAddress : Retrieves the address of an exported function 
	// or variable from the specified dynamic-link library (DLL).
	// Function address has nothing to do with the target.
	LPVOID loadLibraryAAddr = (LPVOID)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "LoadLibraryA");
	if (loadLibraryAAddr == NULL) {
		printf("\t[FAILURE]  LoadLibraryA is not found.\n");
		system("PAUSE");
		return 0;
	}
	printf("\t[SUCCESS]  LoadLibraryA is found at 0x%p.\n", loadLibraryAAddr);

	// Create remote thread in the target!!
	printf("6. Create remote thread\n");
	HANDLE remoteThreadHandle = CreateRemoteThread(
		pHandle,                                    // A handle to the process in which the thread is to be created.
		NULL,
		0,                                          // The initial size of the stack, in bytes. 0 for default size.
		(LPTHREAD_START_ROUTINE)loadLibraryAAddr,   // Represents the starting address of the thread in the remote process.
		(LPVOID *)dllAddr,                          // A pointer to a variable to be passed to the thread function.
		0,                                          // 0 for the thread runs immediately after creation.
		NULL);                                      // A pointer to a variable that receives the thread identifier.
	if (remoteThreadHandle == NULL) {
		printf("\t[FAILURE]  Remote thread creation failed.\n");
		system("PAUSE");
		return 0;
	}
	printf("\t[SUCCESS]  The handle to the remote thread is 0x%p.\n", remoteThreadHandle);

	CloseHandle(pHandle);
	VirtualFreeEx(pHandle, remoteThreadHandle, NULL, MEM_RELEASE);

	system("PAUSE");
	return 0;
}

int findProcessID(wchar_t processName[]) {
	PROCESSENTRY32 entry;

	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE) {
		while (Process32Next(snapshot, &entry) == TRUE) {
			if (wcscmp(entry.szExeFile, processName) == 0) {
				return entry.th32ProcessID;
			}
		}
	}
	return -1;
}