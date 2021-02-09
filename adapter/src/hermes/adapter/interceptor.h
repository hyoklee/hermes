/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Distributed under BSD 3-Clause license.                                   *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Illinois Institute of Technology.                        *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Hermes. The full Hermes copyright notice, including  *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the top directory. If you do not  *
 * have access to the file, you may request a copy from help@hdfgroup.org.   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef HERMES_INTERCEPTOR_H
#define HERMES_INTERCEPTOR_H

/**
 * Standard headers
 */
#include <dlfcn.h>
#include <stdlib.h>
#include <utils.h>

#include <fstream>
#include <string>
#include <unordered_set>
#include <vector>

/**
 * Library headers
 */

/**
 * Internal headers
 */
#include <buffer_pool_internal.h>
#include <hermes/adapter/constants.h>
#include <hermes/adapter/enumerations.h>
#include <hermes/adapter/singleton.h>

/**
 * Define Interceptor list for adapter.
 */
#define INTERCEPTOR_LIST \
  hermes::adapter::Singleton<hermes::adapter::InterceptorList>::GetInstance()

namespace hermes::adapter {
/**
 * Paths prefixed with the following directories are not tracked in Hermes
 * Exclusion list used by darshan at
 * darshan/darshan-runtime/lib/darshan-core.c
 */
const char* kPathExclusions[] = {"/bin/", "/boot/", "/dev/",  "/etc/",
                                 "/lib/", "/opt/",  "/proc/", "/sbin/",
                                 "/sys/", "/usr/",  "/var/",  "/run/"};
/**
 * Paths prefixed with the following directories are tracked by Hermes even if
 * they share a root with a path listed in path_exclusions
 */
const char* kPathInclusions[] = {"/var/opt/cray/dws/mounts/"};
/**
 * Splits a string given a delimiter
 */
std::vector<std::string> StringSplit(char* str, char delimiter) {
  std::stringstream ss(str);
  std::vector<std::string> v;
  while (ss.good()) {
    std::string substr;
    getline(ss, substr, delimiter);
    v.push_back(substr);
  }
  return v;
}
std::string GetFilenameFromFP(FILE* fh) {
  const int kMaxSize = 0xFFF;
  char proclnk[kMaxSize];
  char filename[kMaxSize];
  int fno = fileno(fh);
  snprintf(proclnk, kMaxSize, "/proc/self/fd/%d", fno);
  size_t r = readlink(proclnk, filename, kMaxSize);
  filename[r] = '\0';
  return filename;
}
/**
 * Interceptor list defines files and directory that should be either excluded
 * or included for interceptions.
 */
struct InterceptorList {
  /**
   * hermes buffering mode.
   */
  BufferMode buffer_mode;
  /**
   * Scratch paths
   */
  std::vector<std::string> paths;
  /**
   * Allow users to override the path exclusions
   */
  std::vector<std::string> user_path_exclusions;
  /**
   * Allow adapter to exclude hermes specific files.
   */
  std::vector<std::string> hermes_paths_exclusion;
  /**
   * Allow adapter to exclude files which are currently flushed.
   */
  std::unordered_set<std::string> hermes_flush_exclusion;

  /**
   * Default constructor
   */
  InterceptorList()
      : buffer_mode(BufferMode::PERSISTENT),
        paths(),
        user_path_exclusions(),
        hermes_paths_exclusion(),
        hermes_flush_exclusion() {}
  void SetupBufferingMode() {
    char* hermes_buffering_mode = getenv(kHermesBufferMode);
    if (hermes_buffering_mode == nullptr) {
      // default is kPersistentHermesBufferMode
      buffer_mode = BufferMode::PERSISTENT;
    } else {
      if (strcmp(kPersistentHermesBufferMode, hermes_buffering_mode) == 0) {
        buffer_mode = BufferMode::PERSISTENT;
      } else if (strcmp(kBypassHermesBufferMode, hermes_buffering_mode) == 0) {
        buffer_mode = BufferMode::BYPASS;
      } else if (strcmp(kScratchHermesBufferMode, hermes_buffering_mode) == 0) {
        buffer_mode = BufferMode::SCRATCH;
      } else {
        // TODO(hari): @error_handling throw error.
        return;
      }
    }
    char* hermes_buffering_paths = getenv(kHermesBufferModeInfo);
    std::vector<std::string> paths_local;
    if (hermes_buffering_paths) {
      paths_local = StringSplit(hermes_buffering_paths, kHermesPathDelimiter);
    }
    paths = paths_local;
  }

  bool Persists(FILE* fh) { return Persists(GetFilenameFromFP(fh)); }

  bool Persists(std::string path) {
    if (buffer_mode == BufferMode::PERSISTENT) {
      if (paths.empty())
        return true;
      else {
        for (const auto& pth : paths) {
          if (path.find(pth) == 0) {
            return true;
          }
        }
        return false;
      }
    } else if (buffer_mode == BufferMode::SCRATCH) {
      if (paths.empty())
        return false;
      else {
        for (const auto& pth : paths) {
          if (path.find(pth) == 0) {
            return false;
          }
        }
        return true;
      }
    }
    return true;
  }
};

/**
 * This method is bind in the runtime of the application. It is called during
 * __exit of the program. This function avoid any internal c++ runtime call to
 * get intercepted. On invocation, we set the adaptor to not intercept any
 * calls. If we do not have this method, the program might try to access
 * deallocated structures within adapter.
 */
void OnExit(void);

}  // namespace hermes::adapter

/**
 * HERMES_PRELOAD variable defines if adapter layer should intercept.
 */
#ifdef HERMES_PRELOAD
/**
 * Typedef and function declaration for intercepted functions.
 */
#define HERMES_FORWARD_DECL(func_, ret_, args_) \
  typedef ret_(*real_t_##func_##_) args_;       \
  ret_(*real_##func_##_) args_ = NULL;

#define HERMES_DECL(func_) func_
/**
 * The input function is renamed as real_<func_name>_. And a ptr to function
 * is obtained using dlsym.
 */
#define MAP_OR_FAIL(func_)                                            \
  if (!(real_##func_##_)) {                                           \
    real_##func_##_ = (real_t_##func_##_)dlsym(RTLD_NEXT, #func_);    \
    if (!(real_##func_##_)) {                                         \
      LOG(ERROR) << "HERMES Adapter failed to map symbol: " << #func_ \
                 << std::endl;                                        \
      exit(1);                                                        \
    }                                                                 \
  }

namespace hermes::adapter {
/**
 * Loads the buffering paths for Hermes Adapter to exclude. It inserts the
 * buffering mount points in the InclusionsList.hermes_paths_exclusion.
 */
void PopulateBufferingPath();

/**
 * Check if path should be tracked. In this method, the path is compared against
 * multiple inclusion and exclusion lists.
 *
 * @param path, const std::string&, path to be compared with exclusion and
 * inclusion.
 * @return true, if file should be tracked.
 *         false, if file should not be intercepted.
 */
bool IsTracked(const std::string& path);

/**
 * Check if fh should be tracked. In this method, first Convert fh to path.
 * Then, call IsTracked(path).
 *
 * @param fh, file handlers to be compared for exclusion or inclusion.
 * @return true, if file should be tracked.
 *         false, if file should not be intercepted.
 */
bool IsTracked(FILE* fh);
}  // namespace hermes::adapter
#else
/**
 * Do not intercept if HERMES_PRELOAD is not defined.
 */
#define HERMES_FORWARD_DECL(name_, ret_, args_) \
  extern ret_ real_##name_##_ args_;
#define HERMES_DECL(name_) wrap_##name_##_
#define MAP_OR_FAIL(func_)
#endif
#endif  // HERMES_INTERCEPTOR_H
