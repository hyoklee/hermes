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

#ifndef HRUN_INCLUDE_HRUN_HRUN_NAMESPACE_H_
#define HRUN_INCLUDE_HRUN_HRUN_NAMESPACE_H_

#include "hrun/api/hrun_client.h"
#include "hrun/task_registry/task_lib.h"

using hrun::BinaryInputArchive;
using hrun::BinaryOutputArchive;
using hrun::DataTransfer;
using hrun::DomainId;
using hrun::MultiQueue;
using hrun::PriorityInfo;
using hrun::QueueId;
using hrun::RunContext;
using hrun::Task;
using hrun::TaskFlags;
using hrun::TaskLib;
using hrun::TaskLibClient;
using hrun::TaskMethod;
using hrun::TaskNode;
using hrun::TaskPointer;
using hrun::TaskPrio;
using hrun::TaskStateId;
using hrun::config::QueueManagerInfo;

using hshm::bitfield;
using hshm::bitfield32_t;
using hshm::Mutex;
using hshm::RwLock;
typedef hshm::bitfield<uint64_t> bitfield64_t;
using hipc::LPointer;
using hshm::ScopedRwReadLock;
using hshm::ScopedRwWriteLock;

#endif  // HRUN_INCLUDE_HRUN_HRUN_NAMESPACE_H_
