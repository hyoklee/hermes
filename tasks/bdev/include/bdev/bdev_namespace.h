//
// Created by lukemartinlogan on 8/14/23.
//

#ifndef HRUN_TASKS_BDEV_INCLUDE_BDEV_BDEV_NAMESPACE_H_
#define HRUN_TASKS_BDEV_INCLUDE_BDEV_BDEV_NAMESPACE_H_

#include "bdev.h"
#include "bdev_tasks.h"
#include "hrun/hrun_namespace.h"

/** The set of methods in the admin task */
using ::hermes::bdev::AllocateTask;
using ::hermes::bdev::ConstructTask;
using ::hermes::bdev::DestructTask;
using ::hermes::bdev::FreeTask;
using ::hermes::bdev::Method;
using ::hermes::bdev::ReadTask;
using ::hermes::bdev::StatBdevTask;
using ::hermes::bdev::UpdateScoreTask;
using ::hermes::bdev::WriteTask;

/** Create admin requests */
using ::hermes::bdev::Client;

#endif  // HRUN_TASKS_BDEV_INCLUDE_BDEV_BDEV_NAMESPACE_H_
