#include "Memory.h"

bool ctrlh(DWORD event)
{
	if (event == CTRL_CLOSE_EVENT) {
		std::cout << "Deleting All Hooks" << std::endl;
		RegHookEx::DestroyAllHooks();
		return TRUE;
	}
	return FALSE;
}

void main() {
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)(ctrlh), TRUE);

	RPM rpm;
	rpm.attach("STAR WARS BATTLEFRONT II");

	char sig[] = { 0x44, 0x0F, 0x28, 0x6C, 0x24, 0x70, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF3, 0x41, 0x0F, 0x59, 0xC1, 0xF3, 0x0F, 0x5C, 0xD8, 0x0F, 0x28, 0xC2 };
	DWORD64 AngleFunc = (DWORD64)rpm.PatternScanEx(sig, "xxxxxxx????????xx??????xxxxxxxxxxxx");		
	//0x141613a7a

	RegHookEx AngleFuncHook(rpm.hProcess, AngleFunc, 23, RegHookEx::Regs::RSI);
	DWORD64 AngleFuncPtr = AngleFuncHook.GetAddressOfHook();
	
	std::cout << "Hooked address at: "<<std::hex << AngleFuncPtr << std::endl;

	std::cout << "Done" << std::endl;
	getchar();
}