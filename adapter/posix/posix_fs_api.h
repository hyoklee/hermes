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

#ifndef HERMES_ADAPTER_POSIX_NATIVE_H_
#define HERMES_ADAPTER_POSIX_NATIVE_H_

#include <memory>

#include "adapter/filesystem/filesystem.h"
#include "adapter/filesystem/filesystem_mdm.h"
#include "posix_api.h"
#include "posix_io_client.h"

namespace hermes::adapter::fs {

/** A class to represent POSIX IO file system */
class PosixFs : public hermes::adapter::fs::Filesystem {
 public:
  PosixFs()
      : hermes::adapter::fs::Filesystem(HERMES_POSIX_IO_CLIENT,
                                        AdapterType::kPosix) {}

  template <typename StatT>
  int Stat(File &f, StatT *buf) {
    auto mdm = HERMES_FS_METADATA_MANAGER;
    auto existing = mdm->Find(f);
    if (existing) {
      AdapterStat &astat = *existing;
      /*memset(buf, 0, sizeof(StatT));
      buf->st_dev = 0;
      buf->st_ino = 0;*/
      buf->st_mode = 0100644;
      /*buf->st_nlink = 1;*/
      buf->st_uid = HERMES_SYSTEM_INFO->uid_;
      buf->st_gid = HERMES_SYSTEM_INFO->gid_;
      // buf->st_rdev = 0;
      buf->st_size = GetSize(f, astat);
      /*buf->st_blksize = 0;
      buf->st_blocks = 0;*/
      buf->st_atime = astat.st_atim_.tv_sec;
      buf->st_mtime = astat.st_mtim_.tv_sec;
      buf->st_ctime = astat.st_ctim_.tv_sec;
      errno = 0;
      return 0;
    } else {
      errno = EBADF;
      HELOG(kError, "File with descriptor {} does not exist in Hermes",
            f.hermes_fd_)
      return -1;
    }
  }

  template <typename StatT>
  int Stat(const char *__filename, StatT *buf) {
    bool stat_exists;
    AdapterStat stat;
    stat.flags_ = O_RDONLY;
    stat.st_mode_ = 0;
    File f = Open(stat, __filename);
    if (!f.status_) {
      // HILOG(kInfo, "Failed to stat the file {}", __filename);
      memset(buf, 0, sizeof(StatT));
      return -1;
    }
    int result = Stat(f, buf);
    Close(f, stat_exists);
    return result;
  }

  /** Whether or not \a fd FILE DESCRIPTOR was generated by Hermes */
  static bool IsFdTracked(int fd, std::shared_ptr<AdapterStat> &stat) {
    if (!HERMES->IsInitialized() || fd < 8192) {
      return false;
    }
    hermes::adapter::fs::File f;
    f.hermes_fd_ = fd;
    stat = HERMES_FS_METADATA_MANAGER->Find(f);
    return stat != nullptr;
  }

  /** Whether or not \a fd FILE DESCRIPTOR was generated by Hermes */
  static bool IsFdTracked(int fd) {
    std::shared_ptr<AdapterStat> stat;
    return IsFdTracked(fd, stat);
  }

  /** get the file name from \a fd file descriptor */
  std::string GetFilenameFromFD(int fd) {
    std::vector<char> proclnk(kMaxPathLen);
    std::vector<char> filename(kMaxPathLen);
    snprintf(proclnk.data(), kMaxPathLen - 1, "/proc/self/fd/%d", fd);
    size_t r = readlink(proclnk.data(), filename.data(), kMaxPathLen);
    filename[r] = '\0';
    return std::string(filename.data(), filename.size());
  }
};

/** Simplify access to the stateless PosixFs Singleton */
#define HERMES_POSIX_FS \
  hshm::EasySingleton<hermes::adapter::fs::PosixFs>::GetInstance()
#define HERMES_POSIX_FS_T hermes::adapter::fs::PosixFs *

}  // namespace hermes::adapter::fs

#endif  // HERMES_ADAPTER_POSIX_NATIVE_H_
