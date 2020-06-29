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

#define INDEX_BIT_WIDTH 4
#define TAG_BIT_WIDTH (ADDR_BIT_WIDTH - G_BIT_WIDTH - INDEX_BIT_WIDTH)

struct cache_block
{
  uint32_t tag : TAG_BIT_WIDTH;
  uint32_t valid : 1;
  uint32_t data;
  uint32_t counter;
};

struct cache_block cache[NUM_CACHE_BLOCKS];

void cache_init(void)
{
  num_cache_hit = 0;
  num_cache_miss = 0;

  for (uint32_t idx = 0; idx < NUM_CACHE_BLOCKS; idx++)
    cache[idx].valid = 0;
  return;
}

uint32_t cache_read(uint32_t addr)
{
  // TODO: Assignment #3
  uint32_t value = 0;

  uint32_t tagM = (addr & 0xFFFFFFC0) >> 6;
  uint32_t idxM = (addr & 0x0000003C);

  // printf("cache_read tagM: %x\n", tagM);
  // printf("cache_read idxM: %x\n", idxM);

  for (uint32_t i = 0; i < 4; i++)
  {
    if (cache[idxM + i].valid && cache[idxM + i].tag == tagM)
    {
      num_cache_hit += 1;

      value = cache[idxM + i].data;

      cache[idxM + i].counter = 0x3;

      for(uint32_t j = 0; j < 4; j++)
      {
        if (j !=i && cache[idxM + j].counter != 0x0 && cache[idxM+j].tag != tagM)
        {
          cache[idxM + j].counter -= 1;
        }
      }

      // printf("cache_read hit value: %x\n", value);
      return value;
    }
  }

  num_cache_miss += 1;

  uint32_t set_idx = 0;

  for (uint32_t i = 1; i < 4 ;i++)
  {
    if(cache[idxM + i].counter <= cache[idxM + set_idx].counter){
      set_idx = i;
    }
  }

  uint8_t *p = ram + addr;
  value = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);

  //cache-write 함수를 쓰면 불필요한 write-through를 하게 하므로 이렇게 함.
  cache[idxM + set_idx].tag = tagM;
  cache[idxM + set_idx].valid = 1;
  cache[idxM + set_idx].data = value;

  // printf("cache_read miss value: %x\n", value);
  return value;
}

void cache_write(uint32_t addr, uint32_t value)
{
  // TODO: Assignment #3
  uint32_t tagM = (addr & 0xFFFFFFC0) >> 6;
  uint32_t idxM = (addr & 0x0000003C);

  // printf("cache_write tagM: %x\n", tagM);
  // printf("cache_write idxM: %x\n", idxM);
  // printf("cache_write value: %x\n", value);

  for (uint32_t i = 0; i < 4; i++)
  {
    if (cache[idxM + i].valid && 
    cache[idxM + i].tag == tagM && 
    cache[idxM + i].tag == value)
    {
      // already in cache.
      cache[idxM + i].counter = 0x03;

       for(uint32_t j = 0; j < 4; j++)
      {
        if (j !=i && cache[idxM + j].counter != 0x0 && cache[idxM+j].tag != tagM)
        {
          cache[idxM + j].counter -= 1;
        }
      }
      
      return;
    }
  }

  uint32_t set_idx = 0;

  for (uint32_t i = 1; i < 4 ;i++)
  {
    if(cache[idxM + i].counter <= cache[idxM + set_idx].counter){
      set_idx = i;
    }
  }

  //write-allocate
  cache[idxM + set_idx].tag = tagM;
  cache[idxM + set_idx].valid = 1;
  cache[idxM + set_idx].data = value;

  //write-through
  uint8_t *p = ram + addr;
  p[0] = value & 0xff;
  p[1] = (value >> 8) & 0xff;
  p[2] = (value >> 16) & 0xff;
  p[3] = (value >> 24) & 0xff;
}
