/***
 * @file compilation_use_case.h
 * @brief Application compilation workflow use case
 * @author Amaury Carvalho (2026)
 */

#ifndef COMPILATION_USE_CASE_H_INCLUDED
#define COMPILATION_USE_CASE_H_INCLUDED

#include "compilation_request.h"
#include "compilation_result.h"

class CompilationUseCase {
 public:
  CompilationResult execute(const CompilationRequest& request);
};

#endif  // COMPILATION_USE_CASE_H_INCLUDED
