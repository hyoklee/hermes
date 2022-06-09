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

#include "traits.h"

#include <functional>

#include "buffer_organizer.h"

namespace hermes {
namespace api {
Trait::Trait(TraitID id, TraitIdArray conflict_traits, TraitType type)
    : id(id),
      conflict_traits(conflict_traits),
      type(type),
      onAttachFn(nullptr),
      onDetachFn(nullptr),
      onLinkFn(nullptr),
      onUnlinkFn(nullptr) {}

using OffsetMap = std::unordered_map<std::string, u64>;

PersistTrait::PersistTrait(const std::string &filename,
                           const OffsetMap &offset_map,
                           bool synchronous)
  : Trait(HERMES_PERSIST_TRAIT, TraitIdArray(), TraitType::PERSIST),
    filename(filename), offset_map(offset_map), synchronous(synchronous) {
  this->onAttachFn = std::bind(&PersistTrait::onAttach, this,
                               std::placeholders::_1, std::placeholders::_2,
                               std::placeholders::_3);
  this->onDetachFn = std::bind(&PersistTrait::onDetach, this,
                               std::placeholders::_1, std::placeholders::_2,
                               std::placeholders::_3);
  this->onLinkFn = std::bind(&PersistTrait::onLink, this,
                             std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3);
  this->onUnlinkFn = std::bind(&PersistTrait::onUnlink, this,
                               std::placeholders::_1, std::placeholders::_2,
                               std::placeholders::_3);
}

void PersistTrait::onAttach(HermesPtr hermes, VBucketID id, Trait *trait) {
  SharedMemoryContext *context = &hermes->context_;
  RpcContext *rpc = &hermes->rpc_;

  auto blob_ids =
    GetBlobsFromVBucketInfo(context, rpc, id);
  for (const auto& blob_id : blob_ids) {
    TraitInput input;
    auto bucket_id = GetBucketIdFromBlobId(context, rpc, blob_id);
    input.bucket_name = GetBucketNameById(context, rpc, bucket_id);
    input.blob_name = GetBlobNameFromId(context, rpc, blob_id);
    onLink(hermes, input, trait);
  }
}

void PersistTrait::onDetach(HermesPtr hermes, VBucketID id, Trait *trait) {
  (void)hermes;
  (void)id;
  (void)trait;
}

void PersistTrait::onLink(HermesPtr hermes, TraitInput &input, Trait *trait) {
  PersistTrait *persist_trait = (PersistTrait *)trait;
  auto iter = persist_trait->offset_map.find(input.blob_name);

  if (iter != persist_trait->offset_map.end()) {
    SharedMemoryContext *context = &hermes->context_;
    RpcContext *rpc = &hermes->rpc_;
    BucketID bucket_id = GetBucketId(context, rpc, input.bucket_name.c_str());
    BlobID blob_id = GetBlobId(context, rpc, input.blob_name, bucket_id, true);
    std::string filename = persist_trait->filename;
    u64 offset = iter->second;

    if (synchronous) {
      FlushBlob(context, rpc, blob_id, filename, offset);
    } else {
      EnqueueFlushingTask(rpc, blob_id, filename, offset);
    }
  }
}

void PersistTrait::onUnlink(HermesPtr hermes, TraitInput &input, Trait *trait) {
  (void)hermes;
  (void)input;
  (void)trait;
}

static BlobID GetBlobIdByName(HermesPtr hermes,
                              const std::pair<std::string,
                              std::string> blob_name) {
  SharedMemoryContext *context = &hermes->context_;
  RpcContext *rpc = &hermes->rpc_;
  BucketID bucket_id = GetBucketId(context, rpc, blob_name.second.c_str());
  BlobID blob_id = GetBlobId(context, rpc, blob_name.first.c_str(), bucket_id,
                             false);

  return blob_id;
}

static size_t GetBlobSize(HermesPtr hermes,
                          const std::pair<std::string, std::string> blob_name) {
  size_t blob_size {0};

  BlobID blob_id = GetBlobIdByName(hermes, blob_name);
  if (!IsNullBlobId(blob_id)) {
    u8 arena_memory[KILOBYTES(4)];
    Arena arena = {};
    InitArena(&arena, sizeof(arena_memory), arena_memory);
    blob_size = GetBlobSizeById(&hermes->context_, &hermes->rpc_, &arena,
                                blob_id);
  } else {
    LOG(ERROR) <<"Blob size error from blob " << blob_name.first
               << " in bucket " << blob_name.second;
  }
  return blob_size;
}

bool OrderingTrait::NameAscend(HermesPtr hermes,
                const std::pair<std::string, std::string> first_blob,
                const std::pair<std::string, std::string> second_blob) {
  (void)hermes;
  LOG(INFO) << "Predefined NameAscend order";
  if (first_blob.second == second_blob.second)
    return (first_blob.first < second_blob.first);
  else
    return (first_blob.second < second_blob.second);
}

bool OrderingTrait::NameDescend(HermesPtr hermes,
                 const std::pair<std::string, std::string> first_blob,
                 const std::pair<std::string, std::string> second_blob) {
  (void)hermes;
  LOG(INFO) << "Predefined NameDescend order";
  if (first_blob.second == second_blob.second)
    return (first_blob.first > second_blob.first);
  else
    return (first_blob.second > second_blob.second);
}

bool OrderingTrait::SizeGreater(HermesPtr hermes,
                 const std::pair<std::string, std::string> first_blob,
                 const std::pair<std::string, std::string> second_blob) {
  LOG(INFO) << "Predefined SizeGreater order";
  size_t size1 = GetBlobSize(hermes, first_blob);
  size_t size2 = GetBlobSize(hermes, second_blob);
  return (size1 > size2);
}

bool OrderingTrait::SizeLess(HermesPtr hermes,
              const std::pair<std::string, std::string> first_blob,
              const std::pair<std::string, std::string> second_blob) {
  LOG(INFO) << "Predefined SizeLess order";
  size_t size1 = GetBlobSize(hermes, first_blob);
  size_t size2 = GetBlobSize(hermes, second_blob);

  return (size1 < size2);
}

bool OrderingTrait::Importance(HermesPtr hermes,
                const std::pair<std::string, std::string> first_blob,
                const std::pair<std::string, std::string> second_blob) {
  LOG(INFO) << "Predefined Importance order";
  BlobID blob_id1 = GetBlobIdByName(hermes, first_blob);
  BlobID blob_id2 = GetBlobIdByName(hermes, second_blob);
  f32 score1 = GetBlobImportanceScore(&hermes->context_, &hermes->rpc_,
                                      blob_id1);
  f32 score2 = GetBlobImportanceScore(&hermes->context_, &hermes->rpc_,
                                      blob_id2);

  return (score1 > score2);
}

OrderingTrait::OrderingTrait(u8 num_blob_prefetch, TraitOrder order_func,
                             const std::vector<int> &vect)
    : Trait(HERMES_ORDER_TRAIT, TraitIdArray(), TraitType::DATA),
      hermes_(nullptr),
      blobs_order_(),
      blobs_customerized_order_(vect),
      num_blob_prefetch_(num_blob_prefetch),
      order_func_(order_func) {
  this->onAttachFn = std::bind(&OrderingTrait::onAttach, this,
                               std::placeholders::_1, std::placeholders::_2,
                               std::placeholders::_3);
  this->onDetachFn = std::bind(&OrderingTrait::onDetach, this,
                               std::placeholders::_1, std::placeholders::_2,
                               std::placeholders::_3);
  this->onLinkFn = std::bind(&OrderingTrait::onLink, this,
                             std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3);
  this->onUnlinkFn = std::bind(&OrderingTrait::onUnlink, this,
                               std::placeholders::_1, std::placeholders::_2,
                               std::placeholders::_3);
  this->onGetFn = std::bind(&OrderingTrait::onGet, this,
                            std::placeholders::_1, std::placeholders::_2,
                            std::placeholders::_3);
}

void OrderingTrait::Sort(HermesPtr hermes) {
  LOG(INFO) << "OrderingTrait::Sort(hermes)";
  for (auto iter=blobs_order_.begin(); iter != blobs_order_.end();
       ++iter) {
    LOG(INFO) << iter->first << '\t' << iter->second << std::endl;
  }

  if (blobs_customerized_order_.size() > 0) {
    LOG(INFO) << "vector size = " << blobs_customerized_order_.size();
    return;
  } else if (this->order_func_ && this->blobs_order_.size() > 0) {
     LOG(INFO) << "predefined sorting method";
     std::sort(this->blobs_order_.begin(), this->blobs_order_.end(),
               std::bind(order_func_, hermes, std::placeholders::_1,
                         std::placeholders::_2));
  }
  LOG(INFO) << "After sort";
  for (auto iter=blobs_order_.begin(); iter != blobs_order_.end();
       ++iter) {
    LOG(INFO) << iter->first << '\t' << iter->second << std::endl;
  }

  return;
}

void OrderingTrait::onAttach(HermesPtr hermes, VBucketID id, Trait *trait) {
  (void)trait;
  LOG(INFO) << "OrderingTrait::onAttach";
  if (blobs_order_.size() == 0) {
    LOG(INFO) << "empty list";
    SharedMemoryContext *context = &hermes->context_;
    RpcContext *rpc = &hermes->rpc_;

    auto blob_ids =
      GetBlobsFromVBucketInfo(context, rpc, id);
    for (const auto& blob_id : blob_ids) {
      auto bucket_id = GetBucketIdFromBlobId(context, rpc, blob_id);
      std::string bucket_name = GetBucketNameById(context, rpc, bucket_id);
      std::string blob_name = GetBlobNameFromId(context, rpc, blob_id);
      blobs_order_.push_back(std::make_pair(blob_name, bucket_name));
      LOG(INFO) << "appending " << blob_name << ", " << bucket_name;
    }
  }
  if (blobs_order_.size() > 0) {
    LOG(INFO) << "OrderingTrait::onAttach calls Sort()";
    this->Sort(hermes);
  }
}

void OrderingTrait::onDetach(HermesPtr hermes, VBucketID id, Trait *trait) {
  (void)hermes;
  (void)id;
  (void)trait;
}

void OrderingTrait::onLink(HermesPtr hermes, TraitInput &input, Trait *trait) {
  (void)trait;
  LOG(INFO) << "OrderingTrait::onLink";
  this->blobs_order_.push_back(std::make_pair(input.blob_name,
                                              input.bucket_name));
  this->Sort(hermes);
}

void OrderingTrait::onUnlink(HermesPtr hermes, TraitInput &input,
                             Trait *trait) {
  (void)hermes;
  (void)trait;
  this->blobs_order_.erase(std::remove(this->blobs_order_.begin(),
                           this->blobs_order_.end(),
                           std::make_pair(input.blob_name, input.bucket_name)),
                           this->blobs_order_.end());
}

void OrderingTrait::GetNextN(HermesPtr hermes, std::string blob_name,
                             std::string bkt_name, u8 num_blob_prefetch) {
  LOG(INFO) << "GetNextN from vbucket list";
  std::vector<std::pair<std::string, std::string>>::iterator findIter =
  std::find(blobs_order_.begin(), blobs_order_.end(),
            std::make_pair(blob_name, bkt_name));
  if (blobs_order_.end() == findIter) {
    LOG(ERROR) << "Did not find blob in OrderingTrait";
    return;
  }
  SharedMemoryContext *context = &hermes->context_;
  RpcContext *rpc = &hermes->rpc_;

  if (this->blobs_customerized_order_.size() == 0) {
    LOG(INFO) << "predefined sorting";
    for (auto i {1}; i <= num_blob_prefetch; ++i) {
      if (findIter+i != blobs_order_.end()) {
        BucketID bucket_id = GetBucketId(context, rpc,
                                         (findIter+i)->second.c_str());
        LOG(INFO) << "bucket_id: " << bucket_id.as_int;
        BlobID blob_id = GetBlobId(context, rpc, (findIter+i)->first, bucket_id,
                                   true);
        LOG(INFO) << "blob_id: " << blob_id.as_int;
        LOG(INFO) << "OrganizeBlob predefined order for blob "
                  << (findIter+i)->first;
        OrganizeBlob(context, rpc, bucket_id, (findIter+i)->first.c_str(),  0,
                     1);
      }
    }
  } else {
    LOG(INFO) << "customerized sorting";
    size_t distance = std::distance(blobs_order_.begin(), findIter);
    LOG(INFO) << "distance is " << distance;
    auto find_order = std::find(blobs_customerized_order_.begin(),
                                blobs_customerized_order_.end(), distance);
    if (blobs_customerized_order_.end() == find_order) {
      LOG(ERROR) << "NOT found blob order in OrderingTrait";
      return;
    }

    for (auto i {1}; i <= num_blob_prefetch; ++i) {
      LOG(INFO) << "i = " << i;
      auto next = std::next(find_order, i);
      if (next == blobs_customerized_order_.end()) {
        LOG(INFO) << "reach end of blobs_customerized_order_";
        break;
      }
      LOG(INFO) << "next blob is " << *next;
      if (((size_t)*next) > blobs_order_.size()) {
        LOG(ERROR) << "Blob order number out of range";
        return;
      }
      std::vector<std::pair<std::string, std::string>>::iterator blob_to_fetch
          = blobs_order_.begin();
      std::advance(blob_to_fetch, *next);
      LOG(INFO) << "fetch blob (" << blob_to_fetch->first << ", "
                << blob_to_fetch->second << ")";
      BucketID bucket_id = GetBucketId(context, rpc,
                                       blob_to_fetch->second.c_str());
      BlobID blob_id = GetBlobId(context, rpc, blob_to_fetch->first, bucket_id,
                                 true);
      LOG(INFO) << "blob_id: " << blob_id.as_int;
      OrganizeBlob(context, rpc, bucket_id, blob_to_fetch->first.c_str(),  0,
                   1);
    }
  }
}

void OrderingTrait::onGet(HermesPtr hermes, TraitInput &input, Trait *trait) {
  OrderingTrait *prefetching_trait = (OrderingTrait *)trait;
  LOG(INFO) << "OrderingTrait::onGet";

  OrderingTrait::GetNextN(hermes, input.blob_name, input.bucket_name,
                          prefetching_trait->num_blob_prefetch_);
}
}  // namespace api
}  // namespace hermes
