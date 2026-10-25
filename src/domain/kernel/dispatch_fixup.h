/***
 * @file dispatch_fixup.h
 * @brief Deferred kernel dispatch fixup
 */

#ifndef DISPATCH_FIXUP_H_INCLUDED
#define DISPATCH_FIXUP_H_INCLUDED

struct DispatchFixup {
  unsigned int dispatchIndex;
  unsigned int address;
};

#endif  // DISPATCH_FIXUP_H_INCLUDED
