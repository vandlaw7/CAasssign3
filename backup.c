#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include "emu-rv32i.h"
#include "emu-cache.h"

// Address in 4-Way Set-associative Cache
//
//   TAG   |      INDEX                         | OFFSET
//         | log(NUM_CACHE_BLOCKS)-log(NUM_WAY) | G
// 26 bits |     4 bits                         | 2 bits

#define NUM_WAY 4
#define INDEX_BIT_WIDTH 4
#define TAG_BIT_WIDTH (ADDR_BIT_WIDTH - G_BIT_WIDTH - INDEX_BIT_WIDTH)
#define ASSO_NUM 4

struct cache_block
{
  uint32_t tag : TAG_BIT_WIDTH;
  uint32_t valid : 1;
  uint32_t data;
  uint32_t counter : 2;
};

struct cache_block cache[NUM_CACHE_BLOCKS];

void cache_init(void)
{
  num_cache_hit = 0;
  num_cache_miss = 0;

  for (uint32_t idx = 0; idx < NUM_CACHE_BLOCKS; idx++)
  {
    cache[idx].valid = 0;
    cache[idx].counter = 0;
  }

  return;
}

uint32_t cache_read(uint32_t addr)
{
  // printf("cache_read addr: %x\n", addr);
  uint32_t value = 0;

  uint32_t tagM = (addr & 0xffffffc0) >> 6;
  uint32_t idxM = (addr & 0x0000003c) >> 2;

  uint32_t is_hit = 0;
  for (uint32_t i = 0; i < 4; i++)
  {
    // printf("is_valid: %x\n", cache[4 * idxM + i].valid);
    // printf("tag: %x\n", cache[4 * idxM + i].tag);
    if (cache[4 * idxM + i].valid && cache[4 * idxM + i].tag == tagM)
    {
      value = cache[4 * idxM + i].data;

      is_hit = 1;
      // printf("cache hit! \n");
      // printf("set index: %x\n", i);
      num_cache_hit += 1;

      // printf("cache_read value: %x\n", value);
      return value;
    }
  }

  if (is_hit == 0)
  { 
    // printf("cache miss! \n");
    num_cache_miss += 1;
    uint8_t *p = ram + addr;
    value = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
    uint32_t set_idx = (num_cache_miss + num_cache_hit) % 4;

    cache[idxM * 4 + set_idx].valid = 1;
    cache[idxM * 4 + set_idx].tag = tagM;
    cache[idxM * 4 + set_idx].data = value;
  } 

  // printf("cache_read value: %x\n", value);
  return value;
}

void cache_write(uint32_t addr, uint32_t value)
{
  // printf("cache write address: %x\n", addr);
  uint32_t tagM = (addr & 0xffffffc0) >> 6;
  uint32_t idxM = (addr & 0x0000003c) >> 2;

  uint32_t set_idx = (num_cache_miss + num_cache_hit) % 4;

  cache[idxM * 4 + set_idx].valid = 1;
  cache[idxM * 4 + set_idx].tag = tagM;
  cache[idxM * 4 + set_idx].data = value;

  uint8_t *p = ram + addr;
  p[0] = value & 0x000000ff;
  p[1] = (value & 0x0000ff00) >> 8;
  p[2] = (value & 0x00ff0000) >> 16;
  p[3] = (value & 0xff000000) >> 24;

  // printf("cache write value whole: %x\n", value);
  // printf("cache write value: %x\n", p[0]);
  // printf("cache write value: %x\n", p[1]);
  // printf("cache write value: %x\n", p[2]);
  // printf("cache write value: %x\n", p[3]);
  return;

}

//end

