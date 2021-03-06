#pragma once

#include "Common.h"
#include <string>

namespace Netcode {

	namespace Utility {

		template<size_t N, typename T>
		inline constexpr size_t ArraySize(T(&)[N]) {
			return N;
		}

		/*
		Checkes for an unsigned int if its a power of 2. Wasteful approach but working for edge cases including 0.
		*/
		constexpr bool IsPowerOf2(unsigned int v) {
			unsigned int bitCount = 0;
			for(; v > 0; v >>= 0x1) {
				if(v & 0x1) {
					++bitCount;
				}
			}
			return bitCount == 1;
		}

		constexpr inline uint32_t HighestActiveBitIndex(uint32_t x) {
			uint32_t pow = 0;
			for(pow = 0; x > 1; pow++) {
				x >>= 1;
			}
			return pow;
		}

		template<typename T>
		bool IsWeakRefEmpty(const std::weak_ptr<T> & weak) {
			return !weak.owner_before(std::weak_ptr<T>{}) && !std::weak_ptr<T>{}.owner_before(weak);
		}

		/*
		Takes a number and returns its closest multiplicative of 65536. The returned number is always bigger or equal than the input number
		*/
		template<typename INT_T = uint32_t>
		constexpr INT_T Align64K(INT_T v) {
			return (v + INT_T{ 0xFFFF }) & (~INT_T{ 0xFFFF });
		}

		/*
		Takes a number and returns its closest multiplicative of 256. The returned number is always bigger or equal than the input number
		*/
		template<typename INT_T = uint32_t>
		constexpr INT_T Align256(INT_T v) {
			return (v + INT_T{ 255 }) & (~INT_T{ 255 });
		}

		template<typename INT_T = uint32_t>
		constexpr INT_T Align512(INT_T v) {
			return (v + INT_T{ 511 }) & (~(INT_T{ 511 }));
		}

		template<typename INT_T = uint32_t, INT_T alignment = 256>
		constexpr INT_T Align(INT_T v) {
			return (v + (alignment - 1)) & (~(alignment - 1));
		}
		
		// returns true if data is possibly UTF16-LE encoded
		bool CheckBinaryDataForUTF16LE(ArrayView<uint8_t> data);

		/*
		formats a wide string into a std::wstring
		*/
		std::wstring WFormat(const wchar_t * format, ...);

		/*
		Prints a widechar printf-like message to the Visual Studio Output window
		*/
		void WDebugf(const wchar_t * format, ...);

		/*
		Prints a printf-like message to the Visual Studio Output window
		*/
		void Debugf(const char * format, ...);

		std::string ToNarrowString(const std::wstring & wideString);

		std::wstring ToWideString(const std::string & narrowString);

		/*
		prints the WinAPI event to VS output window, use the filter argument to not print those messages
		*/
		void DebugEvent(unsigned int msg, std::initializer_list<unsigned int> filter = {});
	}

}
