/***
 * @file fswrapper.h
 * @brief File system wrapper header
 * @note Solves some CPP cross OS differences
 * @author Amaury Carvalho
 */

#ifndef FSWRAPPER_H_INCLUDED
#define FSWRAPPER_H_INCLUDED

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <iostream>

using namespace std;

#ifdef Win
#define strlcpy(dst, src, size) strcpy_s((dst), (size), (src))
#define strlcat(dst, src, size) strcat_s((dst), (size), (src))
#define strncpy(dst, src, size) strcpy_s((dst), (size), (src))
#define strncat(dst, src, size) strcat_s((dst), (size), (src))
#define strcasecmp _stricmp
#define fsFolderSeparator '\\'
#else
#define fsFolderSeparator '/'
#endif

/***
 * @brief Check if a file exists
 * @param filename File name to check into file system
 * @return True or False
 */
bool fileExists(const string& filename);

/***
 * @brief Return the path of the file
 * @param filepath File to extract the path
 * @return File path
 */
string getFilePath(const string& filepath);

/***
 * @brief Return the name of the file
 * @param filepath File to extract the name
 * @return File name with extension
 */
string getFileName(const string& filepath);

/***
 * @brief Return the extension of the file
 * @param filepath File to extract the extension
 * @return File extension (e.g: .txt)
 */
string getFileExtension(const string& filepath);

/***
 * @brief Return the name of the file without the extension
 * @param filepath File to extract the name
 * @return File name without extension
 */
string getFileNameWithoutExtension(const string& filepath);

/***
 * @brief Concat two file paths
 * @param base Base file path
 * @param relative Relative file path
 * @return Concatenated file path
 */
string pathJoin(const string& base, const string& relative);

/***
 * @brief Remove quotes character in the begin and end of a string
 */
string removeQuotes(const string& text);

#endif  // FSWRAPPER_H_INCLUDED