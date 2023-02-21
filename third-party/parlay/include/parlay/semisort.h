
#ifndef PARLAY_SEMISORT_H_
#define PARLAY_SEMISORT_H_

#include "internal/sample_sort.h"
#include "internal/get_time.h"
#include "internal/uninitialized_sequence.h"

#include "sequence.h"
#include "primitives.h"
#include "slice.h"
#include <type_traits>

namespace parlay {

constexpr size_t PARLAY_SEMI_SORT_BASE_CASE_SIZE = 1 << 14;
constexpr size_t PARLAY_SEMI_SORT_LESS_BASE_CASE_SIZE = 1 << 14;

template<typename s_size_t, typename inplace_tag, typename assignment_tag, typename InIterator, typename OutIterator,
         typename GetKey, typename Hash, typename Equal>
sequence<s_size_t> semisort_equal_serial(slice<InIterator, InIterator> In, slice<OutIterator, OutIterator> Out,
                                         GetKey g, Hash hash, Equal equal, bool return_offsets, size_t shift_bits) {
  size_t n = In.size();
  if (n == 0) {
    return sequence<s_size_t>{};
  }
  size_t bits = log2_up(static_cast<size_t>(n * 1.1));
  size_t size = size_t{1} << bits;
  size_t mask = size - 1;
  auto next = sequence<size_t>::uninitialized(n);
  auto table = sequence<size_t>(size, ULLONG_MAX);
  for (size_t j = 0; j < n; j++) {
    size_t v = (hash(g(In[j])) >> shift_bits) & mask;
    while (table[v] != ULLONG_MAX && !equal(g(In[j]), g(In[table[v]]))) {
      v = (v + 1) & mask;
    }
    next[j] = table[v];
    table[v] = j;
  }
  size_t pos = 0;
  sequence<s_size_t> offsets;
  for (size_t j = 0; j < size; j++) {
    if (return_offsets) {
      offsets.push_back(pos);
    }
    size_t idx = table[j];
    while (idx != ULLONG_MAX) {
      Out[pos++] = In[idx];
      idx = next[idx];
    }
  }
  assert(pos == n);
  if constexpr (inplace_tag::value == true) {
    for (size_t i = 0; i < n; i++) {
      assign_dispatch(In[i], Out[i], assignment_tag());
    }
  }
  return offsets;
}

template<typename Iterator, typename GetKey,
         typename Hash =
             parlay::hash<typename std::invoke_result<GetKey, typename slice<Iterator, Iterator>::value_type>::type>,
         typename Equal =
             std::equal_to<typename std::invoke_result<GetKey, typename slice<Iterator, Iterator>::value_type>::type>>
void semisort_serial_inplace(slice<Iterator, Iterator> In, GetKey g, Hash hash = {}, Equal equal = {}) {
  using in_type = typename slice<Iterator, Iterator>::value_type;
  auto Tmp = sequence<in_type>::uninitialized(In.size());
  size_t max32 = static_cast<size_t>((std::numeric_limits<uint32_t>::max)());
  if (In.size() < max32) {
    semisort_equal_serial<uint32_t, std::true_type, uninitialized_copy_tag>(In, make_slice(Tmp), g, hash, equal, false,
                                                                            0);
  } else {
    semisort_equal_serial<size_t, std::true_type, uninitialized_copy_tag>(In, make_slice(Tmp), g, hash, equal, false,
                                                                          0);
  }
}

template<typename InIterator, typename GetKey, typename Hash, typename Equal>
auto sample_heavy_keys(slice<InIterator, InIterator> In, const GetKey& g, const Hash& hash, const Equal& equal,
                       size_t shift_bits) {
  size_t n = In.size();
  size_t HEAVY_THRESHOLD = log(n);
  constexpr size_t SAMPLE_QUOTIENT = 500;
  size_t NUM_SAMPLES = HEAVY_THRESHOLD * SAMPLE_QUOTIENT;

  sequence<std::pair<size_t, size_t>> heavy_seq;
  size_t hash_table_size = size_t{1} << log2_up(static_cast<size_t>(NUM_SAMPLES * 1.1));
  size_t hash_table_mask = hash_table_size - 1;
  sequence<std::pair<size_t, size_t>> hash_table(hash_table_size, {ULLONG_MAX, 0});
  for (size_t i = 0; i < NUM_SAMPLES; i++) {
    size_t v = hash64(i) % n;
    size_t idx = hash(g(In[v])) >> shift_bits & hash_table_mask;
    while (hash_table[idx].first != ULLONG_MAX && !equal(g(In[v]), g(In[hash_table[idx].first]))) {
      idx = (idx + 1) & hash_table_mask;
    }
    if (hash_table[idx].first == ULLONG_MAX) {
      hash_table[idx].first = v;
    }
    hash_table[idx].second++;
  }
  size_t heavy_buckets = 0;
  for (size_t i = 0; i < hash_table_size; i++) {
    if (hash_table[i].second >= HEAVY_THRESHOLD) {
      heavy_seq.push_back({hash_table[i].first, heavy_buckets++});
    }
  }
  return heavy_seq;
}

template<typename s_size_t, typename inplace_tag, typename assignment_tag, typename InIterator, typename OutIterator,
         typename TmpIterator, typename GetKey, typename Hash, typename Equal>
sequence<s_size_t> semisort_equal_(slice<InIterator, InIterator> In, slice<OutIterator, OutIterator> Out,
                                   slice<TmpIterator, TmpIterator> Tmp, const GetKey& g, const Hash& hash,
                                   const Equal& equal, bool return_offsets = false, size_t shift_bits = 0,
                                   double parallelism = 1.0) {
  size_t n = In.size();
  using in_type = typename slice<InIterator, InIterator>::value_type;
  using key_type = typename std::invoke_result<GetKey, in_type>::type;
  constexpr size_t hash_bits = sizeof(typename std::invoke_result<Hash, key_type>::type) * 8;
  if (n < PARLAY_SEMI_SORT_BASE_CASE_SIZE || parallelism < .0001 || shift_bits == hash_bits) {
    return semisort_equal_serial<s_size_t, inplace_tag, assignment_tag>(In, Out, g, hash, equal, return_offsets,
                                                                        shift_bits);
  }
  internal::timer t;

  // 1. sampling
  auto heavy_seq = sample_heavy_keys(In, g, hash, equal, shift_bits);
  size_t heavy_id_size = size_t{1} << log2_up(heavy_seq.size() * 5 + 1);
  size_t heavy_id_mask = heavy_id_size - 1;
  sequence<std::pair<size_t, size_t>> heavy_id(heavy_id_size, {ULLONG_MAX, ULLONG_MAX});
  for (const auto& [k, v] : heavy_seq) {
    size_t idx = hash(g(In[k])) >> shift_bits & heavy_id_mask;
    while (heavy_id[idx].first != ULLONG_MAX) {
      idx = (idx + 1) & heavy_id_mask;
    }
    heavy_id[idx] = {k, v};
  }
  auto lookup = [&](size_t k) {
    size_t idx = hash(g(In[k])) >> shift_bits & heavy_id_mask;
    while (heavy_id[idx].first != ULLONG_MAX && !equal(g(In[heavy_id[idx].first]), g(In[k]))) {
      idx = (idx + 1) & heavy_id_mask;
    }
    return heavy_id[idx].second;
  };

  // if (parallelism == 1.0) t.next("sampling");

  internal::timer t_dis;

  // 2. count the number of light/heavy keys
  size_t LOG2_LIGHT_KEYS = std::min<size_t>(hash_bits - shift_bits, 10);
  size_t LIGHT_MASK = (1 << LOG2_LIGHT_KEYS) - 1;
  size_t light_buckets = 1 << LOG2_LIGHT_KEYS;
  size_t heavy_buckets = heavy_seq.size();
  size_t num_buckets = heavy_buckets + light_buckets;
#ifdef BREAKDOWN
  if (parallelism == 1.0) {
    printf("### heavy_buckets: %zu\n", heavy_buckets);
    printf("### light_buckets: %zu\n", light_buckets);
  }
#endif

  auto f = [&](size_t i) {
    size_t it = lookup(i);
    if (it != ULLONG_MAX) {
      // In[i] is a heavy key
      return it + light_buckets;
    } else {
      // In[i] is a light key
      size_t hash_v = hash(g(In[i])) >> shift_bits;
      return hash_v & LIGHT_MASK;
    }
  };
  auto get_bits = delayed_seq<size_t>(n, f);
  auto bucket_offsets =
      std::get<0>(internal::count_sort<assignment_tag>(In, Out, make_slice(get_bits), num_buckets, parallelism));

  // if (parallelism == 1.0) t_dis.next("component of distribute: buckets");

  if constexpr (inplace_tag::value == true) {
    size_t light_keys = bucket_offsets[light_buckets];
    parallel_for(0, n - light_keys,
                 [&](size_t i) { assign_dispatch(In[light_keys + i], Out[light_keys + i], assignment_tag()); });
  }

  // if (parallelism == 1.0) t_dis.next("component of distribute: copy");

  // if (parallelism == 1.0) t.next("distribute");

  // 3. sort within each bucket
  sequence<sequence<s_size_t>> inner_offsets;
  if (return_offsets) {
    inner_offsets = sequence<sequence<s_size_t>>(light_buckets + 1);
  }
  parallel_for(
      0, light_buckets,
      [&](size_t i) {
        size_t start = bucket_offsets[i];
        size_t end = bucket_offsets[i + 1];
        if (start != end) {
          auto a = Out.cut(start, end);
          auto b = Tmp.cut(start, end);
          auto r = semisort_equal_<s_size_t, typename std::negation<inplace_tag>::type, assignment_tag>(
              a, b, a, g, hash, equal, return_offsets, shift_bits + LOG2_LIGHT_KEYS,
              (parallelism * (end - start)) / (n + 1));
          if (return_offsets) {
            parallel_for(0, r.size(), [&](size_t j) { r[j] += start; });
            inner_offsets[i] = r;
          }
        }
      },
      1);

  // if (parallelism == 1.0) t.next("local sort");

  sequence<s_size_t> offsets;
  if (return_offsets) {
    inner_offsets[light_buckets] =
        tabulate<s_size_t>(heavy_buckets, [&](size_t i) { return bucket_offsets[i + light_buckets]; });
    offsets = flatten(inner_offsets);
  }
  // if (parallelism == 1.0) t.next("compute offsets");
  return offsets;
}

template<typename Iterator, typename GetKey,
         typename Hash =
             parlay::hash<typename std::invoke_result<GetKey, typename slice<Iterator, Iterator>::value_type>::type>,
         typename Equal =
             std::equal_to<typename std::invoke_result<GetKey, typename slice<Iterator, Iterator>::value_type>::type>>
auto semisort_equal(slice<Iterator, Iterator> In, GetKey g, Hash hash = {}, Equal equal = {}) {
  using in_type = typename slice<Iterator, Iterator>::value_type;
  auto Out = sequence<in_type>::uninitialized(In.size());
  auto Tmp = sequence<in_type>::uninitialized(In.size());
  size_t max32 = static_cast<size_t>((std::numeric_limits<uint32_t>::max)());
  if (In.size() < max32) {
    semisort_equal_<uint32_t, std::false_type, uninitialized_copy_tag>(In, make_slice(Out), make_slice(Tmp), g, hash,
                                                                       equal);
  } else {
    semisort_equal_<size_t, std::false_type, uninitialized_copy_tag>(In, make_slice(Out), make_slice(Tmp), g, hash,
                                                                     equal);
  }
  return Out;
}

template<typename Iterator, typename GetKey,
         typename Hash =
             parlay::hash<typename std::invoke_result<GetKey, typename slice<Iterator, Iterator>::value_type>::type>,
         typename Equal =
             std::equal_to<typename std::invoke_result<GetKey, typename slice<Iterator, Iterator>::value_type>::type>>
void semisort_equal_inplace(slice<Iterator, Iterator> In, GetKey g, Hash hash = {}, Equal equal = {}) {
  using in_type = typename slice<Iterator, Iterator>::value_type;
  auto Tmp = sequence<in_type>::uninitialized(In.size());
  size_t max32 = static_cast<size_t>((std::numeric_limits<uint32_t>::max)());
  if (In.size() < max32) {
    semisort_equal_<uint32_t, std::true_type, uninitialized_copy_tag>(In, make_slice(Tmp), In, g, hash, equal);
  } else {
    semisort_equal_<size_t, std::true_type, uninitialized_copy_tag>(In, make_slice(Tmp), In, g, hash, equal);
  }
}

template<typename s_size_t, typename inplace_tag, typename assignment_tag, typename InIterator, typename OutIterator,
         typename TmpIterator, typename GetKey, typename Hash, typename Compare>
sequence<s_size_t> semisort_less_(slice<InIterator, InIterator> In, slice<OutIterator, OutIterator> Out,
                                  slice<TmpIterator, TmpIterator> Tmp, const GetKey& g, const Hash& hash,
                                  const Compare& comp, bool return_offsets = false, size_t shift_bits = 0,
                                  double parallelism = 1.0) {
  using in_type = typename slice<InIterator, InIterator>::value_type;
  using key_type = typename std::invoke_result<GetKey, in_type>::type;
  constexpr size_t hash_bits = sizeof(typename std::invoke_result<Hash, key_type>::type) * 8;
  auto equal = [comp](const key_type& a, const key_type& b) { return !comp(a, b) && !comp(b, a); };
  size_t n = In.size();
  assert(In.size() == Out.size());
  if (n < PARLAY_SEMI_SORT_LESS_BASE_CASE_SIZE || parallelism < .0001 || shift_bits == hash_bits) {
    if constexpr (inplace_tag::value == true) {
      internal::seq_sort_inplace(
          In, [&](const in_type& a, const in_type& b) { return comp(g(a), g(b)); }, false);
    } else {
      internal::seq_sort_<assignment_tag>(
          In, Out, [&](const in_type& a, const in_type& b) { return comp(g(a), g(b)); }, false);
    }
    sequence<s_size_t> offsets;
    if (return_offsets) {
      for (size_t i = 0; i < n; i++) {
        if (i == 0 || !equal(g(Out[i]), g(Out[i - 1]))) {
          offsets.push_back(i);
        }
      }
    }
    return offsets;
  }

  // internal::timer t;

  // 1. sampling
  auto heavy_seq = sample_heavy_keys(In, g, hash, equal, shift_bits);
  size_t heavy_id_size = size_t{1} << log2_up(heavy_seq.size() * 5 + 1);
  size_t heavy_id_mask = heavy_id_size - 1;
  sequence<std::pair<size_t, size_t>> heavy_id(heavy_id_size, {ULLONG_MAX, ULLONG_MAX});
  for (const auto& [k, v] : heavy_seq) {
    size_t idx = hash(g(In[k])) >> shift_bits & heavy_id_mask;
    while (heavy_id[idx].first != ULLONG_MAX) {
      idx = (idx + 1) & heavy_id_mask;
    }
    heavy_id[idx] = {k, v};
  }
  auto lookup = [&](size_t k) {
    size_t idx = hash(g(In[k])) >> shift_bits & heavy_id_mask;
    while (heavy_id[idx].first != ULLONG_MAX && !equal(g(In[heavy_id[idx].first]), g(In[k]))) {
      idx = (idx + 1) & heavy_id_mask;
    }
    return heavy_id[idx].second;
  };

  // if (parallelism == 1.0) t.next("sampling");

  // 2. count the number of light/heavy keys
  size_t LOG2_LIGHT_KEYS = std::min<size_t>(hash_bits - shift_bits, 10);
  size_t LIGHT_MASK = (1 << LOG2_LIGHT_KEYS) - 1;
  size_t light_buckets = 1 << LOG2_LIGHT_KEYS;
  size_t heavy_buckets = heavy_seq.size();
  size_t num_buckets = heavy_buckets + light_buckets;
#ifdef BREAKDOWN
  if (parallelism == 1.0) {
    printf("### heavy_buckets: %zu\n", heavy_buckets);
    printf("### light_buckets: %zu\n", light_buckets);
  }
#endif
  internal::timer t_dis;

  auto f = [&](size_t i) {
    size_t it = lookup(i);
    if (it != ULLONG_MAX) {
      // In[i] is a heavy key
      return it + light_buckets;
    } else {
      // In[i] is a light key
      size_t hash_v = hash(g(In[i])) >> shift_bits;
      return hash_v & LIGHT_MASK;
    }
  };
  auto get_bits = delayed_seq<size_t>(n, f);
  auto bucket_offsets =
      std::get<0>(internal::count_sort<assignment_tag>(In, Out, make_slice(get_bits), num_buckets, parallelism));

  if (parallelism == 1.0) t_dis.next("component of distribute: buckets");

  if constexpr (inplace_tag::value == true) {
    size_t light_keys = bucket_offsets[light_buckets];
    parallel_for(0, n - light_keys,
                 [&](size_t i) { assign_dispatch(In[light_keys + i], Out[light_keys + i], assignment_tag()); });
  }

  if (parallelism == 1.0) t_dis.next("component of distribute: copy");

  // if (parallelism == 1.0) t.next("distribute");

  // 3. sort within each bucket
  sequence<sequence<s_size_t>> inner_offsets;
  if (return_offsets) {
    inner_offsets = sequence<sequence<s_size_t>>(light_buckets + 1);
  }
  parallel_for(
      0, light_buckets,
      [&](size_t i) {
        size_t start = bucket_offsets[i];
        size_t end = bucket_offsets[i + 1];
        if (start != end) {
          auto a = Out.cut(start, end);
          auto b = Tmp.cut(start, end);
          auto r = semisort_less_<s_size_t, typename std::negation<inplace_tag>::type, assignment_tag>(
              a, b, a, g, hash, comp, return_offsets, shift_bits + LOG2_LIGHT_KEYS,
              (parallelism * (end - start)) / (n + 1));
          if (return_offsets) {
            parallel_for(0, r.size(), [&](size_t j) { r[j] += start; });
            inner_offsets[i] = r;
          }
        }
      },
      1);

  // if (parallelism == 1.0) t.next("local sort");

  sequence<s_size_t> offsets;
  if (return_offsets) {
    inner_offsets[light_buckets] =
        tabulate<s_size_t>(heavy_buckets, [&](size_t i) { return bucket_offsets[i + light_buckets]; });
    offsets = flatten(inner_offsets);
  }
  // if (parallelism == 1.0) t.next("compute offsets");
  return offsets;
}

template<typename Iterator, typename GetKey,
         typename Hash =
             parlay::hash<typename std::invoke_result<GetKey, typename slice<Iterator, Iterator>::value_type>::type>,
         typename Compare =
             std::less<typename std::invoke_result<GetKey, typename slice<Iterator, Iterator>::value_type>::type>>
auto semisort_less(slice<Iterator, Iterator> In, GetKey g, Hash hash = {}, Compare comp = {}) {
  using in_type = typename slice<Iterator, Iterator>::value_type;
  auto Out = sequence<in_type>::uninitialized(In.size());
  auto Tmp = sequence<in_type>::uninitialized(In.size());
  size_t max32 = static_cast<size_t>((std::numeric_limits<uint32_t>::max)());
  if (In.size() < max32) {
    semisort_less_<uint32_t, std::false_type, uninitialized_copy_tag>(In, make_slice(Out), make_slice(Tmp), g, hash,
                                                                      comp);
  } else {
    semisort_less_<size_t, std::false_type, uninitialized_copy_tag>(In, make_slice(Out), make_slice(Tmp), g, hash,
                                                                    comp);
  }
  return Out;
}

template<typename Iterator, typename GetKey,
         typename Hash =
             parlay::hash<typename std::invoke_result<GetKey, typename slice<Iterator, Iterator>::value_type>::type>,
         typename Compare =
             std::less<typename std::invoke_result<GetKey, typename slice<Iterator, Iterator>::value_type>::type>>
void semisort_less_inplace(slice<Iterator, Iterator> In, GetKey g, Hash hash = {}, Compare comp = {}) {
  using in_type = typename slice<Iterator, Iterator>::value_type;
  auto Tmp = sequence<in_type>::uninitialized(In.size());
  size_t max32 = static_cast<size_t>((std::numeric_limits<uint32_t>::max)());
  if (In.size() < max32) {
    semisort_less_<uint32_t, std::true_type, uninitialized_copy_tag>(In, make_slice(Tmp), In, g, hash, comp);
  } else {
    semisort_less_<size_t, std::true_type, uninitialized_copy_tag>(In, make_slice(Tmp), In, g, hash, comp);
  }
}

}  // namespace parlay
#endif  // PARLAY_SEMISORT_H_
