#include <stdio.h>
#include <stdlib.h>

#include "../src/stream/StreamBuffer.h"

const char *const ByteOrderStrs[2] = {
    "LittleEndian", "BigEndian"};

void printArray(uint8_t *buff, int len)
{
    putchar('[');
    while (len-- > 0)
    {
        printf("%02X, ", *buff++);
    }
    putchar(']');
    putchar('\n');
}

int main()
{
    uint8_t buff[23 * 8] = {0};

    printf("------ StreamBuffer Ver: %s -------\n\n", STREAM_VER_STR);

    StreamBuffer stream;

    Stream_init(&stream, buff, sizeof(buff));

    printf("Sys ByteOrder: %s\n", ByteOrderStrs[Stream_getSystemByteOrder()]);
    printf("Available: %u, Space: %u\n", Stream_available(&stream), Stream_space(&stream));

    for (int i = 0; i < 10; i++)
    {
        Stream_writeDouble(&stream, 3.14 + i);
        printf("Available: %u, Space: %u\n", Stream_available(&stream), Stream_space(&stream));
    }

    for (int i = 0; i < 10; i++)
    {
        double val = Stream_readDouble(&stream);
        printf("Read Double: %.2f, Available: %u, Space: %u\n", val, Stream_available(&stream), Stream_space(&stream));
    }

    printf("Available: %u, Space: %u\n", Stream_available(&stream), Stream_space(&stream));
}