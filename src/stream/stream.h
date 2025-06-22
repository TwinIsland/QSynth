#pragma once

#include <stdint.h>
#include <stdbool.h>

// Lock-free ring buffer for audio streaming
// Safe for single reader + single writer (which is your use case)
typedef struct {
    double *buffer;          // Buffer to store double values
    volatile uint32_t write_pos;  // Write position (modified by producer)
    volatile uint32_t read_pos;   // Read position (modified by consumer)
    uint32_t size;          // Buffer size (number of double elements)
    uint32_t mask;          // Size mask for fast modulo (size must be power of 2)
} AudioStreamBuffer;

// === PUBLIC INTERFACE ===

/**
 * Initialize the lock-free stream buffer
 * @param stream Stream structure to initialize
 * @param buffer Pre-allocated buffer array
 * @param size Buffer size in number of double elements (must be power of 2)
 */
static inline void Stream_init(AudioStreamBuffer *stream, double *buffer, uint32_t size) {
    stream->buffer = buffer;
    stream->size = size;
    stream->mask = size - 1;  // For power-of-2 sizes: size-1 gives us the mask
    stream->write_pos = 0;
    stream->read_pos = 0;
    
    // Zero out the buffer
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = 0.0;
    }
}

/**
 * Write a double value to the stream
 * @param stream Stream to write to
 * @param value Value to write
 * @return 1 if written successfully, 0 if buffer is full
 */
static inline int Stream_writeDouble(AudioStreamBuffer *stream, double value) {
    uint32_t current_write = stream->write_pos;
    uint32_t next_write = (current_write + 1) & stream->mask;
    
    // Check if buffer is full
    if (next_write == stream->read_pos) {
        return 0;  // Buffer full - do nothing
    }
    
    // Write the value
    stream->buffer[current_write] = value;
    
    // Update write position (this must be atomic)
    stream->write_pos = next_write;
    
    return 1;  // Success
}

/**
 * Read a double value from the stream
 * @param stream Stream to read from
 * @return Value read, or 0.0 if buffer is empty
 */
static inline double Stream_readDouble(AudioStreamBuffer *stream) {
    uint32_t current_read = stream->read_pos;
    
    // Check if buffer is empty
    if (current_read == stream->write_pos) {
        return 0.0;  // Buffer empty - return silence
    }
    
    // Read the value
    double value = stream->buffer[current_read];
    
    // Update read position (this must be atomic)
    stream->read_pos = (current_read + 1) & stream->mask;
    
    return value;
}

// === UTILITY FUNCTIONS ===

/**
 * Get number of elements available for reading
 * @param stream Stream to check
 * @return Number of elements available
 */
static inline uint32_t Stream_available(AudioStreamBuffer *stream) {
    return (stream->write_pos - stream->read_pos) & stream->mask;
}

/**
 * Get number of free spaces available for writing
 * @param stream Stream to check
 * @return Number of free spaces
 */
static inline uint32_t Stream_space(AudioStreamBuffer *stream) {
    return (stream->read_pos - stream->write_pos - 1) & stream->mask;
}

/**
 * Check if stream is empty
 * @param stream Stream to check
 * @return true if empty, false otherwise
 */
static inline bool Stream_isEmpty(AudioStreamBuffer *stream) {
    return stream->read_pos == stream->write_pos;
}

/**
 * Check if stream is full
 * @param stream Stream to check
 * @return true if full, false otherwise
 */
static inline bool Stream_isFull(AudioStreamBuffer *stream) {
    return ((stream->write_pos + 1) & stream->mask) == stream->read_pos;
}

/**
 * Get the fill percentage of the buffer
 * @param stream Stream to check
 * @return Fill percentage (0.0 to 1.0)
 */
static inline double Stream_fillRatio(AudioStreamBuffer *stream) {
    uint32_t available = Stream_available(stream);
    return (double)available / (double)(stream->size - 1);
}
