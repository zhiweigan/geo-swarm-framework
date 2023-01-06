#include <parlay/sequence.h>
#include <agent.h>
// #define DEBUG 1


class SemiSorter 
{
public:
  void setup();
  void reset();
  void semisort();
private:
  int n;
  parlay::sequence<bool> sample_index;
  parlay::sequence<Agent> sample;
  parlay::sequence<uint32_t> differences;
  parlay::sequence<uint32_t> offsets;
  parlay::sequence<uint32_t> counts;
  parlay::sequence<uint64_t> unique_hashed_keys;
  parlay::sequence<uint32_t> light_key_bucket_sample_counts;
  parlay::sequence<Bucket> heavy_key_buckets;
  parlay::sequence<Bucket> light_buckets;
  parlay::sequence<Agent> buckets;
  parlay::sequence<int> interval_length;
  parlay::sequence<int> interval_prefix_sum;
};

// ----------------------- DECLARATION -------------------------
namespace constants
{
  const float HASH_RANGE_K = 2.25;
  const float SAMPLE_PROBABILITY_CONSTANT = 3;
  const float DELTA_THRESHOLD = 1;
  const float F_C = 1.25;
  const float LIGHT_KEY_BUCKET_CONSTANT = 2;
}