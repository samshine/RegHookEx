#include "RegHookEx.h"
#include "fde\fde64.h"

bool RegHookEx::CreateHookV6() {
	if (this->lengthOfInstructions > 26 || this->lengthOfInstructions < this->min_size) return false;
	this->HookedAddress = (DWORD64)VirtualAllocEx(this->hProcess, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	ReadProcessMemory(this->hProcess, (LPCVOID)this->FuncAddress, &this->toFixPatch, this->lengthOfInstructions, NULL);
	byte* hkpatch = new byte[83]{
		0x48, 0x8B, 0x05, 0x89, 0x00, 0x00, 0x00,  //  mov rax, [rip + 137]  ;  0x90
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,  //  nop * 26
		0x48, 0x89, 0x0D, 0x60, 0x00, 0x00, 0x00,  //  mov [rip + 96], rcx ; ->0x88
		0x48, 0x89, 0x15, 0x51, 0x00, 0x00, 0x00,  //  mov [rip + 81], rdx ; ->0x80
		0x48, 0x89, 0x2D, 0x42, 0x00, 0x00, 0x00,  //  mov [rip + 66], rbp ; ->0x78
		0x48, 0x89, 0x35, 0x33, 0x00, 0x00, 0x00,  //  mov [rip + 51], rsi ; ->0x70
		0x48, 0x89, 0x3D, 0x24, 0x00, 0x00, 0x00,  //  mov [rip + 36], rdi ; ->0x68
		0x48, 0x89, 0x25, 0x15, 0x00, 0x00, 0x00,  //  mov [rip + 21], rsp ; ->0x60
		0x48, 0x89, 0x1D, 0x06, 0x00, 0x00, 0x00,  //  mov [rip + 6], rbx ; ->0x58
		0xC3  //  ret
	};
	memcpy(hkpatch + 7, &this->toFixPatch, this->lengthOfInstructions);
	WriteProcessMemory(this->hProcess, (LPVOID)this->HookedAddress, hkpatch, 83, NULL);
	byte* funcpath = new byte[32]{
		0x48, 0x89, 0x04, 0x25, 0x90, 0x34, 0x12, 0x00,		// mov [raxpath], rax
		0x48, 0xC7, 0xC0, 0x00, 0x34, 0x12, 0x00,		// mov rax, this->HookedAddress
		0xFF, 0xD0 ,		// call rax ;	0x17
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }; // extra nops
	DWORD64 raxpath = this->HookedAddress + 0x90;
	memcpy(funcpath + 11, &this->HookedAddress, 4);	
	memcpy(funcpath + 4, &raxpath, 4);	
	WriteProcessMemory(this->hProcess, (LPVOID)this->FuncAddress, funcpath, this->lengthOfInstructions, NULL);
	this->HookInstances.push_back(this);
	return true;
}

bool RegHookEx::CreateHookV5() {
	this->HookedAddress = (DWORD64)VirtualAllocEx(this->hProcess, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	byte* hkpatch = new byte[72]{ 0x90, 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x48, 0x89, 0x1D, 0x33, 0x00, 0x00, 0x00, 0x48, 0x89, 0x0D, 0x34, 0x00, 0x00, 0x00, 0x48, 0x89, 0x15, 0x35, 0x00, 0x00, 0x00, 0x48, 0x89, 0x2D, 0x36, 0x00, 0x00, 0x00, 0x48, 0x89, 0x35, 0x37, 0x00, 0x00, 0x00, 0x48, 0x89, 0x3D, 0x38, 0x00, 0x00, 0x00, 0x48, 0x89, 0x25, 0x39, 0x00, 0x00, 0x00, 0xC3 };
	ReadProcessMemory(this->hProcess, (LPCVOID)this->FuncAddress, &this->toFixPatch, this->lengthOfInstructions, NULL);
	memcpy(hkpatch, &this->toFixPatch, this->lengthOfInstructions);
	WriteProcessMemory(this->hProcess, (LPVOID)this->HookedAddress, hkpatch, 72, NULL);
	byte nop[15] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	WriteProcessMemory(this->hProcess, (LPVOID)this->FuncAddress, &nop, this->lengthOfInstructions, NULL);
	byte* funcpath = new byte[9]{ 0x48, 0xc7, 0xc0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xd0 };
	memcpy(funcpath + 3, &this->HookedAddress, 4);
	WriteProcessMemory(this->hProcess, (LPVOID)this->FuncAddress, funcpath, 9, NULL);
	this->HookInstances.push_back(this);
	return true;
}

size_t RegHookEx::GetInstructionLength(void* buff) {
	void *ptr = (void*)buff;
	fde64s cmd;
	decode(ptr, &cmd);
	ptr = (void *)((uintptr_t)ptr + cmd.len);
	return cmd.len;
}

size_t RegHookEx::GetFuncLen() {
	DWORD64 addr = this->FuncAddress;
	while (this->lengthOfInstructions < this->min_size) {
		byte buff[15];
		ReadProcessMemory(this->hProcess, (LPCVOID)addr, &buff, 15, NULL);
		size_t tmpsize = GetInstructionLength(&buff);
		this->lengthOfInstructions += tmpsize;
		addr += tmpsize;
	}
	return this->lengthOfInstructions;
}

DWORD64 RegHookEx::GetAddressOfHook() {
	if (this->HookedAddress == 0) {
		CreateHookV6();
	}
	return this->HookedAddress;
}

void RegHookEx::DestroyHook() {
	if (this->toFixPatch[0]!=0)
		WriteProcessMemory(this->hProcess, (LPVOID)this->FuncAddress, &this->toFixPatch, this->lengthOfInstructions, NULL);
}

void RegHookEx::DestroyAllHooks() {
	for (int i = 0; i < HookInstances.size(); i++) {
		HookInstances[i]->DestroyHook();
	}
}

RegHookEx::RegHookEx(HANDLE _hProcess, DWORD64 _FuncAddress) {
	this->hProcess = _hProcess;
	this->FuncAddress = _FuncAddress;
	this->lengthOfInstructions = this->GetFuncLen();
}


std::vector<RegHookEx*> RegHookEx::HookInstances;