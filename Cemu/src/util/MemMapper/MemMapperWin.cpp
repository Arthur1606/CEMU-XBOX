#include "util/MemMapper/MemMapper.h"

#include <Windows.h>

namespace MemMapper
{
	const size_t sPageSize{ []()
		{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return (size_t)si.dwPageSize;
	}()
	};

	size_t GetPageSize()
	{
		return sPageSize;
	}

	DWORD GetPageProtection(PAGE_PERMISSION permissionFlags)
	{
		DWORD p = 0;
		if (HAS_FLAG(permissionFlags, PAGE_PERMISSION::P_READ) && HAS_FLAG(permissionFlags, PAGE_PERMISSION::P_WRITE) && HAS_FLAG(permissionFlags, PAGE_PERMISSION::P_EXECUTE))
			p = PAGE_EXECUTE_READWRITE;
		else if (HAS_FLAG(permissionFlags, PAGE_PERMISSION::P_READ) && HAS_FLAG(permissionFlags, PAGE_PERMISSION::P_WRITE) && !HAS_FLAG(permissionFlags, PAGE_PERMISSION::P_EXECUTE))
			p = PAGE_READWRITE;
		else if (HAS_FLAG(permissionFlags, PAGE_PERMISSION::P_READ) && !HAS_FLAG(permissionFlags, PAGE_PERMISSION::P_WRITE) && !HAS_FLAG(permissionFlags, PAGE_PERMISSION::P_EXECUTE))
			p = PAGE_READONLY;
		else
			cemu_assert_unimplemented();
		return p;
	}

	void* ReserveMemory(void* baseAddr, size_t size, PAGE_PERMISSION permissionFlags)
	{
		void* r = VirtualAlloc(baseAddr, size, MEM_RESERVE, GetPageProtection(permissionFlags));
		return r;
	}

	void FreeReservation(void* baseAddr, size_t size)
	{
		VirtualFree(baseAddr, size, MEM_RELEASE);
	}

#if defined(CEMU_UWP)
	// AppContainer processes may only create executable pages through the
	// *FromApp APIs, and only when the package declares codeGeneration. Resolve
	// the export at runtime so the DLL keeps loading on systems without it.
	void* AllocateExecutableFromApp(void* baseAddr, size_t size, ULONG allocationType, ULONG protection)
	{
		using VirtualAlloc2FromAppFn = PVOID(WINAPI*)(HANDLE, PVOID, SIZE_T, ULONG, ULONG, void*, ULONG);
		static const auto s_virtualAlloc2FromApp = []() -> VirtualAlloc2FromAppFn
		{
			HMODULE kernelBase = GetModuleHandleW(L"kernelbase.dll");
			return kernelBase ? reinterpret_cast<VirtualAlloc2FromAppFn>(
				GetProcAddress(kernelBase, "VirtualAlloc2FromApp")) : nullptr;
		}();
		if (!s_virtualAlloc2FromApp)
			return nullptr;
		return s_virtualAlloc2FromApp(nullptr, baseAddr, size, allocationType, protection, nullptr, 0);
	}
#endif

	void* AllocateMemory(void* baseAddr, size_t size, PAGE_PERMISSION permissionFlags, bool fromReservation)
	{
		void* r;
		const ULONG allocationType = fromReservation ? MEM_COMMIT : (MEM_RESERVE | MEM_COMMIT);
		const DWORD protection = GetPageProtection(permissionFlags);
		r = VirtualAlloc(baseAddr, size, allocationType, protection);
#if defined(CEMU_UWP)
		if (!r && HAS_FLAG(permissionFlags, PAGE_PERMISSION::P_EXECUTE))
			r = AllocateExecutableFromApp(baseAddr, size, allocationType, protection);
#endif
		return r;
	}

	void FreeMemory(void* baseAddr, size_t size, bool fromReservation)
	{
		if(fromReservation)
			VirtualFree(baseAddr, size, MEM_DECOMMIT);
		else
			VirtualFree(baseAddr, size, MEM_RELEASE);
	}

};
