#ifndef UNISTRING_H
#define UNISTRING_H

#include <string>
#include <sstream>

#ifdef _UNICODE

typedef std::wstring		unistring;
typedef std::wostringstream	uniostringstream;
typedef std::wistringstream uniistringstream;
typedef std::wstringstream	unistringstream;
typedef	wchar_t				unichar;

#else

typedef std::string			unistring;
typedef std::ostringstream	uniostringstream;
typedef std::istringstream	uniistringstream;
typedef std::stringstream	unistringstream;
typedef char				unichar;

#endif

#endif
