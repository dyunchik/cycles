/* SPDX-FileCopyrightText: 2019-2022 Blender Foundation
 *
 * SPDX-License-Identifier: Apache-2.0 */

#ifdef _WIN32
#  include <windows.h>
#ifdef UNICODE
#include <malloc.h>
#endif
#include <string>
#include <map>
#else
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "util/windows.h"

CCL_NAMESPACE_BEGIN

bool system_windows_version_at_least(const int major, const int build)
{
#if defined(_WIN32) && WINAPI_FAMILY != WINAPI_FAMILY_APP
  HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
  if (hMod == 0) {
    return false;
  }

  typedef NTSTATUS(WINAPI * RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
  RtlGetVersionPtr rtl_get_version = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
  if (rtl_get_version == nullptr) {
    return false;
  }

  RTL_OSVERSIONINFOW rovi = {0};
  rovi.dwOSVersionInfoSize = sizeof(rovi);
  if (rtl_get_version(&rovi) != 0) {
    return false;
  }

  return (rovi.dwMajorVersion > major ||
          (rovi.dwMajorVersion == major && rovi.dwBuildNumber >= build));
#else
  (void)major;
  (void)build;
  return false;
#endif
}

int getenv(const char* name, char* buffer, int buffer_size) {
	if (name == nullptr || buffer_size < 0
		|| (buffer == nullptr && buffer_size > 0))
		return INT_MIN;

	int result = 0;
	int term_zero_idx = 0;
	size_t value_length = 0;

#ifdef _WIN32
	value_length = GetEnvironmentVariableA(name, buffer, buffer_size);
#else
	const char* value = ::getenv(name);
	value_length = value == nullptr ? 0 : strlen(value);
#endif

	if (value_length > INT_MAX)
		result = INT_MIN;
	else {
		int int_value_length = (int)value_length;
		if (int_value_length >= buffer_size) {
			result = -int_value_length;
		}
		else {
			term_zero_idx = int_value_length;
			result = int_value_length;
#ifndef _WIN32
			if (value) strncpy(buffer, value, buffer_size - 1);
#endif
		}
	}

	if (buffer != nullptr) buffer[term_zero_idx] = '\0';
	return result;
}

const char* getenv(char const* varName)
{
#ifdef _WIN32
	static std::map<std::string, std::string> retStrMap;
	int buffer_size = getenv(varName, nullptr, 0);
	if (buffer_size>0)
	{
		std::string retStr;
		retStr.resize(buffer_size+1);
		if (getenv(varName, retStr.data(), buffer_size) > 0)
		{
			retStrMap[varName] = retStr;
			return retStrMap[varName].c_str();
		}
	}
	return (const char*)nullptr;
#else
	return ::getenv(varName);
#endif
}

CCL_NAMESPACE_END
