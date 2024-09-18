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

#ifndef HERMES_SRC_DATA_STRUCTURES_H_
#define HERMES_SRC_DATA_STRUCTURES_H_

#include <hermes_shm/data_structures/containers/charbuf.h>
#include <hermes_shm/data_structures/containers/converters.h>
#include <hermes_shm/data_structures/data_structure.h>
#include <hermes_shm/data_structures/ipc/list.h>
#include <hermes_shm/data_structures/ipc/mpsc_queue.h>
#include <hermes_shm/data_structures/ipc/slist.h>
#include <hermes_shm/data_structures/ipc/string.h>
#include <hermes_shm/data_structures/ipc/unordered_map.h>
#include <hermes_shm/data_structures/ipc/vector.h>
#include <hermes_shm/thread/lock.h>
#include <hermes_shm/thread/thread_model_manager.h>
#include <hermes_shm/types/atomic.h>
#include <hermes_shm/util/auto_trace.h>

using hshm::bitfield32_t;
using hshm::Mutex;
using hshm::RwLock;
using hshm::ScopedRwReadLock;
using hshm::ScopedRwWriteLock;

#include <list>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#endif  // HERMES_SRC_DATA_STRUCTURES_H_
