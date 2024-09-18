//
// Created by lukemartinlogan on 6/29/23.
//

#ifndef HRUN_ram_bdev_H_
#define HRUN_ram_bdev_H_

#include "bdev/bdev.h"
#include "hermes/hermes_types.h"
#include "hrun/api/hrun_client.h"
#include "hrun/hrun_namespace.h"
#include "hrun/queue_manager/queue_manager_client.h"
#include "hrun/task_registry/task_lib.h"
#include "hrun_admin/hrun_admin.h"

namespace hermes::ram_bdev {
#include "bdev/bdev_namespace.h"
}  // namespace hermes::ram_bdev

#endif  // HRUN_ram_bdev_H_
