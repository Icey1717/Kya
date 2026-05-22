#ifndef _POINTER_CONV_H
#define _POINTER_CONV_H

#ifdef PLATFORM_WIN

#include <cstddef>
#include <optional>

template<typename T>
struct PointerWrapper
{
	int key;

	PointerWrapper Store(T* ptr)
	{
		key = STORE_POINTER_INT(ptr);
		return *this;
	}
};

#define strd_ptr(type) int
//#define strd_ptr(type) PointerWrapper<type>

namespace PointerConv
{
	int AddPointer(void* value);
	int AddTransientPointer(void* value);
	std::optional<void*> ResolvePointer(int key);
	void* ResolvePointerChecked(int key);
	void ReleasePointer(int key);
	void ResetTransient();
	void ResetAll();
	size_t GetLiveHandleCount();
	size_t GetCapacity();
}

#define STORE_POINTER(a) PointerConv::AddPointer(a)
#define STORE_TRANSIENT_POINTER(a) PointerConv::AddTransientPointer(a)
//#define STORE_POINTER(a) a.Store()
#define LOAD_POINTER(a) PointerConv::ResolvePointerChecked(a)
//#define LOAD_POINTER(a) a.Load()
#define RELEASE_POINTER(a) PointerConv::ReleasePointer(a)
#define POINTERCONV_RESET_TRANSIENT() PointerConv::ResetTransient()
#define POINTERCONV_RESET_ALL() PointerConv::ResetAll()

#define LOAD_POINTER_CAST(type, a) reinterpret_cast<type>(PointerConv::ResolvePointerChecked(a))
#else
#define STORE_POINTER(a) (int)(a)
#define STORE_TRANSIENT_POINTER(a) (int)(a)
#define LOAD_POINTER(a) a
#define RELEASE_POINTER(a) ((void)0)
#define POINTERCONV_RESET_TRANSIENT() ((void)0)
#define POINTERCONV_RESET_ALL() ((void)0)
#define LOAD_POINTER_CAST(type, a) ((type)a)
#endif

#endif //_POINTER_CONV_H
