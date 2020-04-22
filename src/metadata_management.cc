#include "metadata_management.h"

#include <string.h>

#include <string>

#include <thallium/serialization/stl/string.hpp>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "memory_arena.h"
#include "buffer_pool.h"

namespace tl = thallium;

namespace hermes {

u64 LocalGet(IdMap *map, char *key, TicketMutex *mutex) {
  BeginTicketMutex(mutex);
  u64 result = shget(map, key);
  EndTicketMutex(mutex);

  return result;
}

MetadataManager *GetMetadataManagerFromContext(SharedMemoryContext *context) {
  MetadataManager *result =
    (MetadataManager *)(context->shm_base + context->metadata_manager_offset);

  return result;
}

static void MetadataArenaErrorHandler(Arena *arena) {
  // TODO(chogan): GLOG FATAL
  fprintf(stderr, "Metadata arena capacity (%zu bytes) exceeded. ",
          arena->capacity);
  fprintf(stderr, "Consider increasing the value of metadata_arena_percentage "
                  "in the Hermes configuration file.\n");
}

IdMap *GetMap(MetadataManager *mdm, u32 offset) {
  IdMap *result =(IdMap *)((u8 *)mdm + offset);

  return result;
}

IdMap *GetBucketMap(MetadataManager *mdm) {
  IdMap *result = GetMap(mdm, mdm->bucket_map_offset);

  return result;
}

IdMap *GetVBucketMap(MetadataManager *mdm) {
  IdMap *result = GetMap(mdm, mdm->bucket_map_offset);

  return result;
}

IdMap *GetBlobMap(MetadataManager *mdm) {
  IdMap *result = GetMap(mdm, mdm->bucket_map_offset);

  return result;
}

BucketID GetBucketIdByName(SharedMemoryContext *context, char *name,
                           CommunicationContext *comm) {
  BucketID result = {};

  MetadataManager *mdm = GetMetadataManagerFromContext(context);
  int node_id = (stbds_hash_string(name, mdm->map_seed) % comm->num_nodes) + 1;

  if (node_id == comm->node_id) {
    IdMap *map = GetBucketMap(mdm);
    result.as_int = LocalGet(map, name, &mdm->bucket_mutex);
  } else {
    // TODO(chogan): Store this in the metadata arena
    // TODO(chogan): Server must include the node_id
    const char kServerName[] = "ofi+sockets://localhost:8080";
    tl::engine engine("tcp", THALLIUM_CLIENT_MODE);
    tl::remote_procedure remote_get = engine.define("RemoteGet");
    tl::endpoint server = engine.lookup(kServerName);
    result.as_int = remote_get.on(server)(std::string(name));
  }

  return result;
}

Heap *InitHeapInArena(Arena *arena, u32 size, u32 alignment=8) {
  Heap *result = PushStruct<Heap>(arena);
  u32 heap_arena_size = size - sizeof(Heap);
  u8 *heap_base = PushSize(arena, heap_arena_size);
  InitArena(&result->arena, heap_arena_size, heap_base);
  result->alignment = alignment;
  result->free_list_offset = alignment;

  FreeBlock *free_block = (FreeBlock *)(result->arena.base + alignment);
  // NOTE(chogan): offset 0 represents NULL
  free_block->next_offset = 0;
  // NOTE(chogan): Subtract alignment since we're using the first `alignment`
  // sized block as the NULL block.
  free_block->size = heap_arena_size - alignment;

  return result;
}

void InitMetadataManager(MetadataManager *mdm, Arena *arena, Config *config,
                         int node_id) {

  // NOTE(chogan): All MetadatManager offsets are relative to the address of the
  // MDM itself.

  arena->error_handler = MetadataArenaErrorHandler;

  mdm->map_seed = 0x4E58E5DF;
  stbds_rand_seed(mdm->map_seed);

  // Initialize BucketInfo array

  BucketInfo *buckets = PushArray<BucketInfo>(arena,
                                              config->max_buckets_per_node);
  mdm->bucket_info_offset = (u8 *)buckets - (u8 *)mdm;
  mdm->first_free_bucket.bits.node_id = (u32)node_id;
  mdm->first_free_bucket.bits.index = 0;
  mdm->num_buckets = 0;
  mdm->max_buckets = config->max_buckets_per_node;

  for (u32 i = 0; i < config->max_buckets_per_node; ++i) {
    BucketInfo *info = buckets + i;

    if (i == config->max_buckets_per_node - 1) {
      info->next_free.as_int = 0;
    } else {
      info->next_free.bits.node_id = (u32)node_id;
      info->next_free.bits.index = i + 1;
    }
  }

  // Initialize VBucketInfo array

  VBucketInfo *vbuckets = PushArray<VBucketInfo>(arena,
                                                 config->max_vbuckets_per_node);
  mdm->vbucket_info_offset = (u8 *)vbuckets - (u8 *)mdm;
  mdm->first_free_vbucket.bits.node_id = (u32)node_id;
  mdm->first_free_vbucket.bits.index = 0;
  mdm->num_vbuckets = 0;
  mdm->max_vbuckets = config->max_vbuckets_per_node;

  for (u32 i = 0; i < config->max_vbuckets_per_node; ++i) {
    VBucketInfo *info = vbuckets + i;

    if (i == config->max_vbuckets_per_node - 1) {
      info->next_free.as_int = 0;
    } else {
      info->next_free.bits.node_id = (u32)node_id;
      info->next_free.bits.index = i + 1;
    }
  }

  size_t metadata_space_remaining = arena->capacity - arena->used;

  // Remaining space in metadata arena is divided into
  // - BlobID Heap
  // - BufferID Heap
  // - bucket Map = max_buckets * BucketID + max_buckets * avg_string_size
  // - vbucket Map = max_vbuckets * VBucketID + max_vbuckets * avg_string_size
  // - blob Map = avg_blobs_per_file?

  // size_t avg_name_length = 32;
  // size_t estimated_bucket_map_size =
  //   (config->max_buckets_per_node + 16) * sizeof(BucketID) +
  //   (config->max_buckets_per_node + 16) * avg_name_length;

  // size_t estimated_vbucket_map_size =
  //   (config->max_vbuckets_per_node + 16) * sizeof(VBucketID) +
  //   (config->max_vbuckets_per_node + 16) * avg_name_length;

  // size_t estimated_blob_map_size = ?;

  size_t quarter = metadata_space_remaining / 4;
  size_t blob_heap_size = quarter;
  size_t buffer_heap_size = quarter * 3;

  Heap *blobid_heap = InitHeapInArena(arena, blob_heap_size, sizeof(BlobID));
  mdm->blobid_heap_offset = (u8 *)blobid_heap - (u8 *)mdm;

  Heap *bufferid_heap = InitHeapInArena(arena, buffer_heap_size,
                                        sizeof(BufferID));
  mdm->bufferid_heap_offset = (u8 *)bufferid_heap - (u8 *)mdm;

  // ID Maps
  // ptrdiff_t bucket_map_offset;
  // ptrdiff_t vbucket_map_offset;
  // ptrdiff_t blob_map_offset;
}

void CoalesceFreeBlocks(Heap *heap) {
  (void)heap;
}

#if 0
FreeBlock *FindBestFit(FreeBlock *head, size_t desired_size, u32 threshold=0) {
  FreeBlock *result = 0;
  FreeBlock *prev = head;
  FreeBlock *smallest_prev = head;
  u32 smallest_wasted = 0xFFFFFFFF;

  while(head && smallest_wasted > threshold) {
    if (head->size >= desired_size) {
      u32 wasted_space = head->size - desired_size ;
      if (wasted_space < smallest_wasted) {
        smallest_wasted = wasted_space;
        result = head;
        smallest_prev = prev;
      }
    }
    prev = head;
    head = head->next;
  }

  if (result) {
    smallest_prev->next = result->next;
  }

  return result;
}

String *PushString(Heap *heap, const char *str, size_t length) {
  String *result = 0;

  if (heap->free_list) {
    FreeBlock *best_fit = FindBestFit(heap->free_list,
                                      sizeof(String) + length + 1);
    if (best_fit) {
      result = (String *)best_fit;
      result->str = (char *)(result + 1);
    }
  }

  if (!result) {
    result = PushStruct<String>(&heap->arena);
    size_t total_size = RoundUpToMultiple(length + 1, heap->alignment);
    result->str = PushArray<char>(&heap->arena, total_size, 1);
  }

  result->length = length;
  for (int i = 0; i < length; ++i) {
    result->str[i] = str[i];
  }
  result->str[length] = '\0';

  return result;
}

String *PushString(Heap *heap, const char *str) {
  size_t max_size = heap->arena.capacity - heap->arena.used - sizeof(String);
  size_t length = strnlen(str, max_size - 1);
  String *result = PushString(heap, str, length);

  return result;
}

String *PushString(Heap *heap, const std::string &str) {
  String *result = PushString(heap, str.c_str(), str.size());

  return result;
}

void FreeString(Heap *heap, String **str) {
  if (heap && str && *str) {
    FreeBlock *new_block = (FreeBlock *)(*str);
    new_block->next = heap->free_list;
    size_t freed_str_size = RoundUpToMultiple((*str)->length + 1,
                                              heap->alignment);
    new_block->size = freed_str_size + sizeof(**str);
    heap->free_list = new_block;

    // Invalidate caller's pointer so they can't accidentally access freed data
    *str = 0;
  }
}
#endif
}  // namespace hermes
