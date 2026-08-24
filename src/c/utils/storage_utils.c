#include "storage_utils.h"

// ---------------------------------------------------------------------------
// Write a buffer across consecutive persist keys.
//
// Splits `length` bytes into PERSIST_KEY_MAX_BYTES-sized chunks and writes
// each to `key`, `key + 1`, `key + 2`, ... Stopping as soon as the whole
// buffer is written or the caller's `max_blocks` budget is exhausted.
//
// Returns the total number of bytes written (mirroring persist_write_data,
// which reports how many bytes made it to storage). This is exactly `length`
// on a full write, and shorter when the buffer is too big for `max_blocks` or
// a write fails partway through.
// ---------------------------------------------------------------------------
uint32_t persist_write_data_multi(uint32_t key, const void *buffer,
                                  uint32_t length, uint32_t max_blocks) {
  uint8_t *data = (uint8_t *)buffer;
  uint32_t written_total = 0;
  uint32_t block_index = 0;

  while (written_total < length && block_index < max_blocks) {
    uint32_t chunk_len = length - written_total;
    if (chunk_len > PERSIST_DATA_MAX_LENGTH) {
      chunk_len = PERSIST_DATA_MAX_LENGTH;
    }

    uint32_t written = persist_write_data(key + block_index,
                                          data + written_total,
                                          (uint16_t)chunk_len);
    written_total += written;

    // If a single key write came up short, there is no point continuing; the
    // remaining keys would only widen the hole in the stored buffer.
    if (written < chunk_len) {
      break;
    }

    block_index++;
  }

  return written_total;
}

// ---------------------------------------------------------------------------
// Read a buffer spanning consecutive persist keys.
//
// Reads PERSIST_KEY_MAX_BYTES chunks from `key`, `key + 1`, `key + 2`, ...
// up to `length` bytes or until `max_blocks` keys have been consumed. The one
// "missing" key (or an empty key) ends the span cleanly.
//
// Returns the total number of bytes read (mirroring persist_read_data's
// contract). A short return means the stored buffer is smaller than the
// requested `length`, the span was truncated by `max_blocks`, or data was
// missing.
// ---------------------------------------------------------------------------
uint32_t persist_read_data_multi(uint32_t key, void *buffer,
                                 uint32_t length, uint32_t max_blocks) {
  uint8_t *data = (uint8_t *)buffer;
  uint32_t read_total = 0;
  uint32_t block_index = 0;

  while (read_total < length && block_index < max_blocks) {
    uint32_t chunk_len = length - read_total;
    if (chunk_len > PERSIST_DATA_MAX_LENGTH) {
      chunk_len = PERSIST_DATA_MAX_LENGTH;
    }

    uint32_t got = persist_read_data(key + block_index,
                                     data + read_total,
                                     (uint16_t)chunk_len);
    if (got == 0) {
      // Nothing stored under this key: the span ends here.
      break;
    }

    read_total += got;

    // A block that holds less than a full chunk means the stored buffer
    // finishes inside this key, so later keys carry no further data.
    if (got < chunk_len) {
      break;
    }

    block_index++;
  }

  return read_total;
}