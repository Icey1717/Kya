#ifndef _POINTER_CONV_H
#define _POINTER_CONV_H

#ifdef PLATFORM_WIN

#include <optional>
#include <string>

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
	std::optional<void*> ResolvePointer(int key);
	void* ResolvePointerChecked(int key);
}

#define STORE_POINTER(a) PointerConv::AddPointer(a)
//#define STORE_POINTER(a) a.Store()
#define LOAD_POINTER(a) PointerConv::ResolvePointerChecked(a)
//#define LOAD_POINTER(a) a.Load()

#define LOAD_POINTER_CAST(type, a) reinterpret_cast<type>(PointerConv::ResolvePointerChecked(a))
#else
#define STORE_POINTER(a) (int)(a)
#define LOAD_POINTER(a) a
#define LOAD_POINTER_CAST(type, a) ((type)a)
#endif

#endif //_POINTER_CONV_H
