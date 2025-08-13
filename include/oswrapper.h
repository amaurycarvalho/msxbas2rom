/***
 * @file oswrapper.h
 * @brief Solves some CPP cross OS differences
 * @author Amaury Carvalho
 */

#ifndef OSWRAPPER_H_INCLUDED
#define OSWRAPPER_H_INCLUDED

#ifdef Win
#include <string.h>
#define strlcpy(dst, src, size) strcpy_s((dst), (size), (src))
#define strlcat(dst, src, size) strcat_s((dst), (size), (src))
#define strcasecmp _stricmp
#endif

#endif  // OSWRAPPER_H_INCLUDED