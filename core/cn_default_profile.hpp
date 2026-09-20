#pragma once
namespace owh {
// The shipped profile lives in profiles/cn-16x9.json so recognition steps can be
// edited, versioned and shared without rebuilding. This is the same document
// embedded as a fallback: an unattended recovery must not lose its navigation
// because a data file went missing.
inline const char *defaultCNProfileJson() {
  return
#include "core/cn_default_profile.inc"
      ;
}
} // namespace owh
