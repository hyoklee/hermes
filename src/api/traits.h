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

#ifndef HERMES_TRAITS_H
#define HERMES_TRAITS_H

#include <unordered_map>

#include "hermes.h"
#include "hermes_types.h"

namespace hermes {
namespace api {

/** A blob's hosting bucket and blob names */
struct BlobInfo {
  /** The blob-hosting bucket name */
  std::string bucket_name;
  /** The blob's name (in the bucket) */
  std::string blob_name;
};

typedef BlobInfo TraitInput;
struct Trait;
using HermesPtr = std::shared_ptr<Hermes>;

/** Callback for blob->vbucket link events */
typedef std::function<void(HermesPtr, TraitInput &, Trait *)> OnLinkCallback;
/** Callback for trait->vbucket attach events */
typedef std::function<void(HermesPtr, VBucketID, Trait *)> OnAttachCallback;
typedef std::function<bool(HermesPtr, std::pair<std::string, std::string>,
                           std::pair<std::string, std::string>)>
    TraitOrder;
/** Traits represent vbucket behavior */
struct Trait {
  /** The trait's ID */
  TraitID id;
  /** \todo ??? */
  TraitIdArray conflict_traits;
  /** The trait's type */
  TraitType type;
  /** Callback for trait->vbucket attach events */
  OnAttachCallback onAttachFn;
  /** Callback for trait-<vbucket detach events */
  OnAttachCallback onDetachFn;
  /** Callback for blob->vbucket link events */
  OnLinkCallback onLinkFn;
  /** Callback for blob-<vbucket unlink events */
  OnLinkCallback onUnlinkFn;
  /** Callback for VBucket::Get events */
  OnLinkCallback onGetFn;

  Trait() {}
  Trait(TraitID id, TraitIdArray conflict_traits, TraitType type);
};

#define HERMES_PERSIST_TRAIT 11
#define HERMES_ORDER_TRAIT 12

/** (File) Persistence trait */
struct PersistTrait : public Trait {
  std::string filename;
  std::unordered_map<std::string, u64> offset_map;
  bool synchronous;

  explicit PersistTrait(bool synchronous);
  explicit PersistTrait(const std::string &filename,
                        const std::unordered_map<std::string, u64> &offset_map,
                        bool synchronous = false);

  void onAttach(HermesPtr hermes, VBucketID id, Trait *trait);
  void onDetach(HermesPtr hermes, VBucketID id, Trait *trait);
  void onLink(HermesPtr hermes, TraitInput &blob, Trait *trait);
  void onUnlink(HermesPtr hermes, TraitInput &blob, Trait *trait);
};

struct OrderingTrait : public Trait {
 private:
  HermesPtr hermes_;
  std::vector<std::pair<std::string, std::string>> blobs_order_;
  std::vector<int> blobs_customerized_order_;
  void GetNextN(HermesPtr hermes, std::string blob_name, std::string bkt_name,
                u8 num_blob_prefetch);
  void Sort(HermesPtr hermes);

 public:
  u8 num_blob_prefetch_;
  TraitOrder order_func_;
  OrderingTrait(u8 num_blob_prefetch, TraitOrder order_func = nullptr,
                const std::vector<int> &vect = std::vector<int>());
  void onAttach(HermesPtr hermes, VBucketID id, Trait *trait);
  void onDetach(HermesPtr hermes, VBucketID id, Trait *trait);
  void onLink(HermesPtr hermes, TraitInput &blob, Trait *trait);
  void onUnlink(HermesPtr hermes, TraitInput &blob, Trait *trait);
  void onGet(HermesPtr hermes, TraitInput &blob, Trait *trait);
  static bool NameAscend(HermesPtr hermes,
                         const std::pair<std::string, std::string>,
                         const std::pair<std::string, std::string>);
  static bool NameDescend(HermesPtr hermes,
                          const std::pair<std::string, std::string>,
                          const std::pair<std::string, std::string>);

  static bool SizeGreater(HermesPtr hermes,
                          const std::pair<std::string, std::string>,
                          const std::pair<std::string, std::string>);

  static bool SizeLess(HermesPtr hermes,
                       const std::pair<std::string, std::string>,
                       const std::pair<std::string, std::string>);

  static bool Importance(HermesPtr hermes,
                         const std::pair<std::string, std::string>,
                         const std::pair<std::string, std::string>);
};
}  // namespace api
}  // namespace hermes

#endif  // HERMES_TRAITS_H
