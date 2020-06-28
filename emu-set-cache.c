#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <assert.h>
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
  // TODO: Assignment #3
  uint32_t value = 0;

  uint32_t tagM = (addr & 0xFFFFFFC0) >> 6;
  uint32_t idxM = (addr & 0x0000003C) >> 2;

  struct cache_block cb_arr[ASSO_NUM];
  for (uint8_t i = 0; i < ASSO_NUM; i++)
  {
    cb_arr[i] = cache[ASSO_NUM * idxM + i];
  }

  for (uint8_t i = 0; i < ASSO_NUM; i++)
  {
    struct cache_block now_cb;
    now_cb = cb_arr[i];

    if (now_cb.valid && now_cb.tag == tagM)
    {
      num_cache_hit += 1;
      value = now_cb.data;
      cache[ASSO_NUM * idxM + i].counter = 3;

      for (uint8_t j = 0; j < ASSO_NUM; j++)
      {
        if (j != i && now_cb.counter)
        {
          cache[ASSO_NUM * idxM + j].counter -= 1;
        }
      }

      return value;
    }
  }

  //여기까지 왔다는 것은 miss가 났다는 뜻이다.
  num_cache_miss += 1;
  uint8_t *p = ram + addr;
  value = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);

  //가장 작은 counter를 가진 cache_block을 찾아내서 data를 덮어쓴다.
  uint8_t rewrite_index = 0;
  for (uint8_t i = 0; i < ASSO_NUM; i++)
  {
    if (!cb_arr[i].valid)
    {
      rewrite_index = i;
      break;
    }
    if (cb_arr[i].counter < cb_arr[rewrite_index].counter)
    {
      rewrite_index = i;
    }
  }

  //cache-write 함수를 쓰면 불필요한 write-through를 하게 하므로 cache-write 대신 직접 처리함.
  cache[ASSO_NUM * idxM + rewrite_index].tag = tagM;
  cache[ASSO_NUM * idxM + rewrite_index].valid = 1;
  cache[ASSO_NUM * idxM + rewrite_index].data = value;
  cache[ASSO_NUM * idxM + rewrite_index].counter = 3;

  return value;
}

void cache_write(uint32_t addr, uint32_t value)
{
  // TODO: Assignment #3
  uint32_t tagM = (addr & 0xFFFFFFC0) >> 6;
  uint32_t idxM = (addr & 0x0000003C) >> 2;

  struct cache_block cb_arr[ASSO_NUM];
  for (uint8_t i = 0; i < ASSO_NUM; i++)
  {
    cb_arr[i] = cache[ASSO_NUM * idxM + i];
  }

  //가장 작은 counter를 가진 cache_block을 찾아내서 data를 덮어쓴다.
  uint8_t rewrite_index = 0;
  for (uint8_t i = 0; i < ASSO_NUM; i++)
  {
    if (!cb_arr[i].valid)
    {
      rewrite_index = i;
      break;
    }
    if (cb_arr[i].counter < cb_arr[rewrite_index].counter)
    {
      rewrite_index = i;
    }
  }

  //write-allocate
  cache[ASSO_NUM * idxM + rewrite_index].tag = tagM;
  cache[ASSO_NUM * idxM + rewrite_index].valid = 1;
  cache[ASSO_NUM * idxM + rewrite_index].data = value;
  cache[ASSO_NUM * idxM + rewrite_index].counter = 3;

  //write-through
  uint8_t *p = ram + addr;
  p[0] = value & 0xff;
  p[1] = (value >> 8) & 0xff;
  p[2] = (value >> 16) & 0xff;
  p[3] = (value >> 24) & 0xff;
}
