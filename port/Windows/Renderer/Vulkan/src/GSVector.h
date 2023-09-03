#pragma once

#include <iterator>
#include <emmintrin.h>

class alignas(16) GSVector4
{
public:
	union
	{
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
	};

	static GSVector4 zero() {
		GSVector4 zeroed;
		memset(&zeroed, 0, sizeof(GSVector4));
		return zeroed;
	}

	__forceinline explicit GSVector4() {

	}

	__forceinline explicit GSVector4(float f)
	{
		x = f;
		y = f;
		z = f;
		w = f;
	}

	__forceinline explicit GSVector4(float x, float y, float z, float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
};

class alignas(16) GSVector4i
{
	//static GSVector4i m_xff[17];
	//static GSVector4i m_x0f[17];

public:
	union
	{
		struct { int x, y, z, w; };
		struct { int r, g, b, a; };
		struct { int left, top, right, bottom; };
		int v[4];
		float f32[4];
		int8_t i8[16];
		int16_t i16[8];
		int32_t i32[4];
		int64_t  i64[2];
		uint8_t u8[16];
		uint16_t u16[8];
		uint32_t u32[4];
		uint64_t u64[2];
		__m128i m;
	};

	__forceinline GSVector4i()
	{
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}

	__forceinline explicit GSVector4i(int i)
	{
		*this = i;
	}

	__forceinline explicit GSVector4i(__m128i m)
	{
		this->m = m;
	}

	__forceinline void operator = (__m128i m)
	{
		this->m = m;
	}

	__forceinline operator __m128i() const
	{
		return m;
	}

	__forceinline static GSVector4i load(int i)
	{
		return GSVector4i(_mm_cvtsi32_si128(i));
	}

	__forceinline GSVector4i(int x, int y, int z, int w)
	{
		// 4 gprs

		// m = _mm_set_epi32(w, z, y, x);

		// 2 gprs

		GSVector4i xz = load(x).upl32(load(z));
		GSVector4i yw = load(y).upl32(load(w));

		*this = xz.upl32(yw);
	}

	__forceinline void operator = (int i)
	{
#if _M_SSE >= 0x501

		m = _mm_broadcastd_epi32(_mm_cvtsi32_si128(i));

#else

		m = _mm_set1_epi32(i);

#endif
	}

	__forceinline int width() const
	{
		return right - left;
	}

	__forceinline int height() const
	{
		return bottom - top;
	}

	__forceinline static GSVector4i zero() { return GSVector4i(_mm_setzero_si128()); }

	__forceinline static GSVector4i loadl(const void* p)
	{
		return GSVector4i(_mm_loadl_epi64((__m128i*)p));
	}

	__forceinline static GSVector4i loadh(const void* p)
	{
		return GSVector4i(_mm_castps_si128(_mm_loadh_pi(_mm_setzero_ps(), (__m64*)p)));
	}

	__forceinline static GSVector4i loadh(const void* p, const GSVector4i& v)
	{
		return GSVector4i(_mm_castps_si128(_mm_loadh_pi(_mm_castsi128_ps(v.m), (__m64*)p)));
	}

	__forceinline static GSVector4i load(const void* pl, const void* ph)
	{
		return loadh(ph, loadl(pl));
	}

	template<bool aligned> __forceinline static GSVector4i load(const void* p)
	{
		return GSVector4i(aligned ? _mm_load_si128((__m128i*)p) : _mm_loadu_si128((__m128i*)p));
	}

	__forceinline GSVector4i upl8(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpacklo_epi8(m, a));
	}

	__forceinline GSVector4i uph8(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpackhi_epi8(m, a));
	}

	__forceinline GSVector4i upl16(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpacklo_epi16(m, a));
	}

	__forceinline GSVector4i uph16(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpackhi_epi16(m, a));
	}

	__forceinline GSVector4i upl32(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpacklo_epi32(m, a));
	}

	__forceinline GSVector4i uph32(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpackhi_epi32(m, a));
	}

	__forceinline GSVector4i upl64(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpacklo_epi64(m, a));
	}

	__forceinline GSVector4i uph64(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpackhi_epi64(m, a));
	}

	__forceinline GSVector4i upl8() const
	{
#if 0 // _M_SSE >= 0x401 // TODO: compiler bug

		return GSVector4i(_mm_cvtepu8_epi16(m));

#else

		return GSVector4i(_mm_unpacklo_epi8(m, _mm_setzero_si128()));

#endif
	}

	__forceinline GSVector4i uph8() const
	{
		return GSVector4i(_mm_unpackhi_epi8(m, _mm_setzero_si128()));
	}

	__forceinline GSVector4i upl16() const
	{
#if 0 //_M_SSE >= 0x401 // TODO: compiler bug

		return GSVector4i(_mm_cvtepu16_epi32(m));

#else

		return GSVector4i(_mm_unpacklo_epi16(m, _mm_setzero_si128()));

#endif
	}

	__forceinline GSVector4i uph16() const
	{
		return GSVector4i(_mm_unpackhi_epi16(m, _mm_setzero_si128()));
	}

	__forceinline GSVector4i upl32() const
	{
#if 0 //_M_SSE >= 0x401 // TODO: compiler bug

		return GSVector4i(_mm_cvtepu32_epi64(m));

#else

		return GSVector4i(_mm_unpacklo_epi32(m, _mm_setzero_si128()));

#endif
	}

	__forceinline GSVector4i uph32() const
	{
		return GSVector4i(_mm_unpackhi_epi32(m, _mm_setzero_si128()));
	}

	__forceinline GSVector4i upl64() const
	{
		return GSVector4i(_mm_unpacklo_epi64(m, _mm_setzero_si128()));
	}

	__forceinline GSVector4i uph64() const
	{
		return GSVector4i(_mm_unpackhi_epi64(m, _mm_setzero_si128()));
	}

	__forceinline GSVector4i eq8(const GSVector4i& v) const
	{
		return GSVector4i(_mm_cmpeq_epi8(m, v.m));
	}

	__forceinline GSVector4i eq16(const GSVector4i& v) const
	{
		return GSVector4i(_mm_cmpeq_epi16(m, v.m));
	}

	__forceinline GSVector4i eq32(const GSVector4i& v) const
	{
		return GSVector4i(_mm_cmpeq_epi32(m, v.m));
	}

	__forceinline GSVector4i min_i16(const GSVector4i& a) const
	{
		return GSVector4i(_mm_min_epi16(m, a));
	}

	__forceinline GSVector4i max_i16(const GSVector4i& a) const
	{
		return GSVector4i(_mm_max_epi16(m, a));
	}

	__forceinline GSVector4i gt8(const GSVector4i& v) const
	{
		return GSVector4i(_mm_cmpgt_epi8(m, v.m));
	}

	__forceinline GSVector4i gt16(const GSVector4i& v) const
	{
		return GSVector4i(_mm_cmpgt_epi16(m, v.m));
	}

	__forceinline GSVector4i gt32(const GSVector4i& v) const
	{
		return GSVector4i(_mm_cmpgt_epi32(m, v.m));
	}

	__forceinline GSVector4i lt8(const GSVector4i& v) const
	{
		return GSVector4i(_mm_cmplt_epi8(m, v.m));
	}

	__forceinline GSVector4i lt16(const GSVector4i& v) const
	{
		return GSVector4i(_mm_cmplt_epi16(m, v.m));
	}

	__forceinline GSVector4i lt32(const GSVector4i& v) const
	{
		return GSVector4i(_mm_cmplt_epi32(m, v.m));
	}

	__forceinline GSVector4i andnot(const GSVector4i& v) const
	{
		return GSVector4i(_mm_andnot_si128(v.m, m));
	}

	__forceinline int mask() const
	{
		return _mm_movemask_epi8(m);
	}

	__forceinline static void sw4(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		const __m128i epi32_0f0f0f0f = _mm_set1_epi32(0x0f0f0f0f);

		GSVector4i mask(epi32_0f0f0f0f);

		GSVector4i e = (b << 4).blend(a, mask);
		GSVector4i f = b.blend(a >> 4, mask);
		GSVector4i g = (d << 4).blend(c, mask);
		GSVector4i h = d.blend(c >> 4, mask);

		a = e.upl8(f);
		c = e.uph8(f);
		b = g.upl8(h);
		d = g.uph8(h);
	}

	__forceinline static void sw8(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		GSVector4i e = a;
		GSVector4i f = c;

		a = e.upl8(b);
		c = e.uph8(b);
		b = f.upl8(d);
		d = f.uph8(d);
	}

	__forceinline static void sw16(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		GSVector4i e = a;
		GSVector4i f = c;

		a = e.upl16(b);
		c = e.uph16(b);
		b = f.upl16(d);
		d = f.uph16(d);
	}

	__forceinline static void sw16rl(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		GSVector4i e = a;
		GSVector4i f = c;

		a = b.upl16(e);
		c = e.uph16(b);
		b = d.upl16(f);
		d = f.uph16(d);
	}

	__forceinline static void sw16rh(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		GSVector4i e = a;
		GSVector4i f = c;

		a = e.upl16(b);
		c = b.uph16(e);
		b = f.upl16(d);
		d = d.uph16(f);
	}

	__forceinline static void sw32(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		GSVector4i e = a;
		GSVector4i f = c;

		a = e.upl32(b);
		c = e.uph32(b);
		b = f.upl32(d);
		d = f.uph32(d);
	}

	__forceinline static void sw64(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		GSVector4i e = a;
		GSVector4i f = c;

		a = e.upl64(b);
		c = e.uph64(b);
		b = f.upl64(d);
		d = f.uph64(d);
	}

	template<int src, class T> __forceinline GSVector4i gather32_8(const T* ptr) const
	{
		return GSVector4i(
			(int)ptr[extract8<src + 0>()],
			(int)ptr[extract8<src + 1>()],
			(int)ptr[extract8<src + 2>()],
			(int)ptr[extract8<src + 3>()]);
	}

	template<class T> __forceinline void gather32_8(const T* ptr, GSVector4i* dst) const
	{
		dst[0] = gather32_8<0>(ptr);
		dst[1] = gather32_8<4>(ptr);
		dst[2] = gather32_8<8>(ptr);
		dst[3] = gather32_8<12>(ptr);
	}

	__forceinline GSVector4i blend(const GSVector4i& a, const GSVector4i& mask) const
	{
		return GSVector4i(_mm_or_si128(_mm_andnot_si128(mask, m), _mm_and_si128(mask, a)));
	}

	__forceinline void operator += (const GSVector4i& v)
	{
		m = _mm_add_epi32(m, v);
	}

	__forceinline void operator -= (const GSVector4i& v)
	{
		m = _mm_sub_epi32(m, v);
	}

	__forceinline void operator += (int i)
	{
		*this += GSVector4i(i);
	}

	__forceinline void operator -= (int i)
	{
		*this -= GSVector4i(i);
	}

	__forceinline void operator <<= (const int i)
	{
		m = _mm_slli_epi32(m, i);
	}

	__forceinline void operator >>= (const int i)
	{
		m = _mm_srli_epi32(m, i);
	}

	__forceinline void operator &= (const GSVector4i& v)
	{
		m = _mm_and_si128(m, v);
	}

	__forceinline void operator |= (const GSVector4i& v)
	{
		m = _mm_or_si128(m, v);
	}

	__forceinline void operator ^= (const GSVector4i& v)
	{
		m = _mm_xor_si128(m, v);
	}

	__forceinline friend GSVector4i operator + (const GSVector4i& v1, const GSVector4i& v2)
	{
		return GSVector4i(_mm_add_epi32(v1, v2));
	}

	__forceinline friend GSVector4i operator - (const GSVector4i& v1, const GSVector4i& v2)
	{
		return GSVector4i(_mm_sub_epi32(v1, v2));
	}

	__forceinline friend GSVector4i operator + (const GSVector4i& v, int i)
	{
		return v + GSVector4i(i);
	}

	__forceinline friend GSVector4i operator - (const GSVector4i& v, int i)
	{
		return v - GSVector4i(i);
	}

	__forceinline friend GSVector4i operator << (const GSVector4i& v, const int i)
	{
		return GSVector4i(_mm_slli_epi32(v, i));
	}

	__forceinline friend GSVector4i operator >> (const GSVector4i& v, const int i)
	{
		return GSVector4i(_mm_srli_epi32(v, i));
	}

	__forceinline friend GSVector4i operator & (const GSVector4i& v1, const GSVector4i& v2)
	{
		return GSVector4i(_mm_and_si128(v1, v2));
	}

	__forceinline friend GSVector4i operator | (const GSVector4i& v1, const GSVector4i& v2)
	{
		return GSVector4i(_mm_or_si128(v1, v2));
	}

	__forceinline friend GSVector4i operator ^ (const GSVector4i& v1, const GSVector4i& v2)
	{
		return GSVector4i(_mm_xor_si128(v1, v2));
	}

	__forceinline friend GSVector4i operator & (const GSVector4i& v, int i)
	{
		return v & GSVector4i(i);
	}

	__forceinline friend GSVector4i operator | (const GSVector4i& v, int i)
	{
		return v | GSVector4i(i);
	}

	__forceinline friend GSVector4i operator ^ (const GSVector4i& v, int i)
	{
		return v ^ GSVector4i(i);
	}

	__forceinline friend GSVector4i operator ~ (const GSVector4i& v)
	{
		return v ^ (v == v);
	}

	__forceinline friend GSVector4i operator == (const GSVector4i& v1, const GSVector4i& v2)
	{
		return GSVector4i(_mm_cmpeq_epi32(v1, v2));
	}

	__forceinline friend GSVector4i operator != (const GSVector4i& v1, const GSVector4i& v2)
	{
		return ~(v1 == v2);
	}

	__forceinline friend GSVector4i operator > (const GSVector4i& v1, const GSVector4i& v2)
	{
		return GSVector4i(_mm_cmpgt_epi32(v1, v2));
	}

	__forceinline friend GSVector4i operator < (const GSVector4i& v1, const GSVector4i& v2)
	{
		return GSVector4i(_mm_cmplt_epi32(v1, v2));
	}

	__forceinline friend GSVector4i operator >= (const GSVector4i& v1, const GSVector4i& v2)
	{
		return (v1 > v2) | (v1 == v2);
	}

	__forceinline friend GSVector4i operator <= (const GSVector4i& v1, const GSVector4i& v2)
	{
		return (v1 < v2) | (v1 == v2);
	}

	template<bool aligned> __forceinline static void store(void* p, const GSVector4i& v)
	{
		if (aligned) _mm_store_si128((__m128i*)p, v.m);
		else _mm_storeu_si128((__m128i*)p, v.m);
	}

	template<int i> __forceinline int extract8() const
	{
#if _M_SSE >= 0x401

		return _mm_extract_epi8(m, i);

#else

		return (int)u8[i];

#endif
	}

	__forceinline GSVector4i u16to32() const
	{
		return upl16();
	}

	__forceinline GSVector4i sub32(const GSVector4i& v) const
	{
		return GSVector4i(_mm_sub_epi32(m, v.m));
	}

	__forceinline GSVector4i sra32(int i) const
	{
		return GSVector4i(_mm_srai_epi32(m, i));
	}

	__forceinline GSVector4i ps32() const
	{
		return GSVector4i(_mm_packs_epi32(m, m));
	}

	__forceinline static void storel(void* p, const GSVector4i& v)
	{
		_mm_storel_epi64((__m128i*)p, v.m);
	}

	template<int src, class T> __forceinline GSVector4i gather64_8(const T* ptr) const
	{
		GSVector4i v;

		v = load(&ptr[extract8<src + 0>()], &ptr[extract8<src + 1>()]);

		return v;
	}

	template<class T> __forceinline void gather64_8(const T* ptr, GSVector4i* dst) const
	{
		dst[0] = gather64_8<0>(ptr);
		dst[1] = gather64_8<2>(ptr);
		dst[2] = gather64_8<4>(ptr);
		dst[3] = gather64_8<6>(ptr);
		dst[4] = gather64_8<8>(ptr);
		dst[5] = gather64_8<10>(ptr);
		dst[6] = gather64_8<12>(ptr);
		dst[7] = gather64_8<14>(ptr);
	}

#define VECTOR4i_SHUFFLE_4(xs, xn, ys, yn, zs, zn, ws, wn) \
		__forceinline GSVector4i xs##ys##zs##ws() const {return GSVector4i(_mm_shuffle_epi32(m, _MM_SHUFFLE(wn, zn, yn, xn)));} \
		__forceinline GSVector4i xs##ys##zs##ws##l() const {return GSVector4i(_mm_shufflelo_epi16(m, _MM_SHUFFLE(wn, zn, yn, xn)));} \
		__forceinline GSVector4i xs##ys##zs##ws##h() const {return GSVector4i(_mm_shufflehi_epi16(m, _MM_SHUFFLE(wn, zn, yn, xn)));} \
		__forceinline GSVector4i xs##ys##zs##ws##lh() const {return GSVector4i(_mm_shufflehi_epi16(_mm_shufflelo_epi16(m, _MM_SHUFFLE(wn, zn, yn, xn)), _MM_SHUFFLE(wn, zn, yn, xn)));} \

#define VECTOR4i_SHUFFLE_3(xs, xn, ys, yn, zs, zn) \
		VECTOR4i_SHUFFLE_4(xs, xn, ys, yn, zs, zn, x, 0) \
		VECTOR4i_SHUFFLE_4(xs, xn, ys, yn, zs, zn, y, 1) \
		VECTOR4i_SHUFFLE_4(xs, xn, ys, yn, zs, zn, z, 2) \
		VECTOR4i_SHUFFLE_4(xs, xn, ys, yn, zs, zn, w, 3) \

#define VECTOR4i_SHUFFLE_2(xs, xn, ys, yn) \
		VECTOR4i_SHUFFLE_3(xs, xn, ys, yn, x, 0) \
		VECTOR4i_SHUFFLE_3(xs, xn, ys, yn, y, 1) \
		VECTOR4i_SHUFFLE_3(xs, xn, ys, yn, z, 2) \
		VECTOR4i_SHUFFLE_3(xs, xn, ys, yn, w, 3) \

#define VECTOR4i_SHUFFLE_1(xs, xn) \
		VECTOR4i_SHUFFLE_2(xs, xn, x, 0) \
		VECTOR4i_SHUFFLE_2(xs, xn, y, 1) \
		VECTOR4i_SHUFFLE_2(xs, xn, z, 2) \
		VECTOR4i_SHUFFLE_2(xs, xn, w, 3) \

	VECTOR4i_SHUFFLE_1(x, 0)
		VECTOR4i_SHUFFLE_1(y, 1)
		VECTOR4i_SHUFFLE_1(z, 2)
		VECTOR4i_SHUFFLE_1(w, 3)
};