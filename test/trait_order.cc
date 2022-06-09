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

#include <cstdio>
#include <iostream>
#include <unordered_map>
#include <random>

#include <mpi.h>

#include "hermes.h"
#include "bucket.h"
#include "vbucket.h"
#include "test_utils.h"

namespace hapi = hermes::api;
std::shared_ptr<hermes::api::Hermes> hermes_app;

int main(int argc, char **argv) {
  int mpi_threads_provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &mpi_threads_provided);
  if (mpi_threads_provided < MPI_THREAD_MULTIPLE) {
    fprintf(stderr, "Didn't receive appropriate MPI threading specification\n");
    return 1;
  }

  char *config_file = 0;
  if (argc == 2) {
    config_file = argv[1];
  }

  std::shared_ptr<hapi::Hermes> hermes = hapi::InitHermes(config_file);

  int data_size = 8 * 1024;
  hapi::Blob put_data(data_size, rand() % 255);
  hapi::Blob get_data(data_size, 255);
  if (hermes->IsApplicationCore()) {
    int app_rank = hermes->GetProcessRank();

    hapi::Status status;
    int bytes_per_blob = 1024;
    std::string bloba_name = "bloba";
    std::string blobb_name = "blobb";
    std::string blobc_name = "blobc";
    std::string blobx_name = "blobx";
    std::string bloby_name = "bloby";
    std::string blobz_name = "blobz";
    hapi::Blob bloba, blobb, blobc, blobx, bloby, blobz;

    std::random_device rd;  // obtain a random number from hardware
    std::mt19937 gen(rd());  // seed the generator
    std::uniform_int_distribution<unsigned char> distr(0, 255);
    for (int n = 0; n < bytes_per_blob; ++n) {
      bloba.push_back(distr(gen));
      blobb.push_back(distr(gen));
      blobc.push_back(distr(gen));
      blobx.push_back(distr(gen));
      bloby.push_back(distr(gen));
      blobz.push_back(distr(gen));
    }

    hapi::Context ctx;
    const std::string bucket_name = "order_bucket";
    hapi::Bucket bkt(bucket_name, hermes, ctx);
    LOG(INFO) << "bucket id = " << bkt.GetId();
    status = bkt.Put(bloba_name, bloba, ctx);
    Assert(status.Succeeded());
    status = bkt.Put(blobb_name, blobb, ctx);
    Assert(status.Succeeded());
    status = bkt.Put(blobc_name, blobc, ctx);
    Assert(status.Succeeded());
    status = bkt.Put(blobx_name, blobx, ctx);
    Assert(status.Succeeded());
    status = bkt.Put(bloby_name, bloby, ctx);
    Assert(status.Succeeded());
    status = bkt.Put(blobz_name, blobz, ctx);
    Assert(status.Succeeded());

    const std::string vbucket_name = "prefetching";
    hermes::api::VBucket vb(vbucket_name, hermes, ctx);
/*
    //auto trait = hermes::api::OrderingTrait(3);
    //auto trait = hermes::api::OrderingTrait(3, hermes::api::NameAscend);
    //auto trait = hermes::api::OrderingTrait(3, hermes::api::NameDescend);
    auto trait = hermes::api::OrderingTrait(3, hermes::api::SizeGreater);
    LOG(INFO) << "trait.cc VBucket Attach";
    status = vb.Attach(&trait);
    Assert(status.Succeeded());
*/
    LOG(INFO) << "trait.cc Link";
    vb.Link(blobx_name, bucket_name);
    vb.Link(blobb_name, bucket_name);
    vb.Link(bloba_name, bucket_name);
    vb.Link(blobc_name, bucket_name);
    vb.Link(blobz_name, bucket_name);
    vb.Link(bloby_name, bucket_name);

    LOG(INFO) << "trait.cc VBucket ContainsBlob";
    Assert(vb.ContainsBlob(bloba_name, bucket_name) == 1);
    Assert(vb.ContainsBlob(blobb_name, bucket_name) == 1);
    Assert(vb.ContainsBlob(blobc_name, bucket_name) == 1);
    Assert(vb.ContainsBlob(blobx_name, bucket_name) == 1);
    Assert(vb.ContainsBlob(bloby_name, bucket_name) == 1);
    Assert(vb.ContainsBlob(blobz_name, bucket_name) == 1);

    std::vector<int> blob_order{5, 2, 4, 3, 0, 1};

    // auto trait = hermes::api::OrderingTrait(3, hermes::api::NameAscend);
    // auto trait =
    //   hermes::api::OrderingTrait(3, hermes::api::OrderingTrait::NameAscend);
    // auto trait =
    //   hermes::api::OrderingTrait(3, hermes::api::OrderingTrait::NameDescend);
    auto trait = hermes::api::OrderingTrait(3, hapi::OrderingTrait::Importance);
    // auto trait = hermes::api::OrderingTrait(3, nullptr, blob_order);
    LOG(INFO) << "trait.cc VBucket Attach";
    status = vb.Attach(&trait);
    Assert(status.Succeeded());

    hapi::Blob retrieved_blob;
    // size_t sizes = bkt.Get(blobc_name, retrieved_blob, ctx);
    size_t sizes = vb.Get(blobc_name, &bkt, retrieved_blob);
    // size_t sizes = vb.Get(blobc_name, &bkt, retrieved_blob, ctx);

    retrieved_blob.resize(sizes);

    // sizes = bkt.Get(blobc_name, retrieved_blob, ctx);
    sizes = vb.Get(blobc_name, &bkt, retrieved_blob);
    // sizes = vb.Get(blobc_name, &bkt, retrieved_blob, ctx);
    Assert(blobc == retrieved_blob);
    Assert(sizes == retrieved_blob.size());

    // trait.Sort();

    hermes->AppBarrier();
    status = bkt.Destroy();
    Assert(status.Succeeded());
    hermes->AppBarrier();
    if (app_rank == 0) {
      status = vb.Destroy();
      Assert(status.Succeeded());
    }
    hermes->AppBarrier();
  } else {
    // Hermes core. No user code here.
  }

  hermes->Finalize();

  MPI_Finalize();

  return 0;
}
