#pragma once
#include <functional>
#include <unordered_set>
#include <random>

#include "parlay/utilities.h"
#include "parlay/primitives.h"
#include "parlay/parallel.h"
#include "parlay/sequence.h"
#include "parlay/hash_table.h"

struct Bucket
{
  unsigned long long bucket_id;
  unsigned int offset;
  unsigned int size : 31;

  inline bool operator!=(Bucket a)
  {
    return a.bucket_id != bucket_id || a.offset != offset || a.size != size;
  }

  inline bool operator==(Bucket a)
  {
    return a.bucket_id == bucket_id && a.size == size && a.offset == offset;
  }
};

struct hash_buckets
{
  using eType = Bucket;
  using kType = unsigned long long;
  eType empty() { return {0, 0, 0, 0}; }
  kType getKey(eType v) { return v.bucket_id; }
  size_t hash(kType v) { return static_cast<size_t>(parlay::hash64(v)); }
  int cmp(kType v, kType b) { return (v > b) ? 1 : ((v == b) ? 0 : -1); }
  bool replaceQ(eType, eType) { return 0; }
  eType update(eType v, eType) { return v; }
  bool cas(eType *p, eType o, eType n)
  {
    auto pb = reinterpret_cast<std::atomic<unsigned long long> *>(&(p->bucket_id));
    if (std::atomic_compare_exchange_strong_explicit(pb, &(o.bucket_id), n.bucket_id, std::memory_order_relaxed, std::memory_order_relaxed))
    {
      *p = n;
      return true;
    }
    return false;
  }
};

unsigned int size_func(unsigned int num_records, double p, unsigned int n, double c)
{
  double lnn = (double)log((double)n);
  double clnn = c * lnn;

  double fs = (num_records + clnn + sqrt(clnn * clnn + 2 * num_records * c * clnn)) / p;
  double array_size = 1.1 * fs;

  return (unsigned int)pow(2, ceil(log(array_size) / log(2)));
}

template <class eType>
bool bucket_cas(eType *p, eType o, eType n)
{
  return std::atomic_compare_exchange_strong_explicit(
      reinterpret_cast<std::atomic<eType> *>(p), &o, n, std::memory_order_relaxed, std::memory_order_relaxed);
}

// round n down to nearest multiple of m
uint64_t round_down(uint64_t n, uint64_t m)
{
  return n >= 0 ? (n / m) * m : ((n - m + 1) / m) * m;
}