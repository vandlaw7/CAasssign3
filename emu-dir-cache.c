#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include "emu-rv32i.h"
#include "emu-cache.h"

// Address in Direct Mapped Cache
//
//   TAG   |      INDEX            | OFFSET
//   TAG   | log(NUM_CACHE_BLOCKS) |   G
// 24 bits |     6 bits            | 2 bits

#define INDEX_BIT_WIDTH 6
#define TAG_BIT_WIDTH (ADDR_BIT_WIDTH - G_BIT_WIDTH - INDEX_BIT_WIDTH)

struct cache_block
{
  uint32_t tag:TAG_BIT_WIDTH;
  uint32_t valid:1;
  uint32_t data;
};

struct cache_block cache[NUM_CACHE_BLOCKS];

void cache_init(void)
{
  num_cache_hit = 0;
  num_cache_miss = 0;

  for(uint32_t idx=0; idx<NUM_CACHE_BLOCKS; idx++)
    cache[idx].valid = 0;
  return;
}

uint32_t cache_read(uint32_t addr)
{
  // TODO: Assignment #3
  uint32_t value = 0;
  return value;
}

void cache_write(uint32_t addr, uint32_t value)
{
  // TODO: Assignment #3
}

