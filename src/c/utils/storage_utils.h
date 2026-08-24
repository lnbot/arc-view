#pragma once
#include <pebble.h>

// ---------------------------------------------------------------------------
// Multi-key persistent storage helpers.
//
// A single Pebble persist key can hold at most PERSIST_KEY_MAX_BYTES bytes.
// These helpers transparently span a buffer across the consecutive persist
// keys `key`, `key + 1`, `key + 2`, ... so larger buffers can be stored.
//
// A caller that supplies `max_blocks` caps the number of consecutive keys the
// helpers may touch, so a fixed-size region of persist keys can be laid out and
// never overflow into unrelated keys.
// ---------------------------------------------------------------------------

// Write `length` bytes from `data` across consecutive persist keys,
// returning the number of bytes actually written.  May be less than `length`
// if the buffer spans more than `max_blocks` keys.  Matches the return code
// convention of persist_write_data().
uint32_t persist_write_data_multi(uint32_t key, const void *data,
                                  uint32_t length, uint32_t max_blocks);

// Read up to `length` bytes into `data` across consecutive persist keys,
// returning the number of bytes actually read.  Stops early (short return)
// once a missing/empty key is hit or `max_blocks` is reached.  Matches the
// return code convention of persist_read_data().
uint32_t persist_read_data_multi(uint32_t key, void *data,
                                 uint32_t length, uint32_t max_blocks);