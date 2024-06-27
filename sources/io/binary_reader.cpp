#include "binary_reader.h"

namespace pf_io
{
    static Endian get_endian()
    {
        union { uint32_t i32; uint8_t i8_4[4]; } test;
        test.i32 = 1;
        return test.i8_4[0] ? Endian::Little : Endian::Big;
    }

    BinaryReader::BinaryReader(std::string filename, Endian endian):
        file_stream(filename, std::ios::binary),
        is_endian_different(endian != Endian::Native && endian != get_endian())
    {}

    BinaryReader::~BinaryReader()
    {
        file_stream.close();
    }

    uint32_t BinaryReader::read_uint32()
    {
        uint8_t bytes[4];
        file_stream.read(reinterpret_cast<char*>(bytes), 4);
        if (is_endian_different)
        {
            return bytes[3] | (bytes[2] << 8) | (bytes[1] << 16) | (bytes[0] << 32);
        }
        else
        {
            return *reinterpret_cast<uint32_t*>(&bytes);
        }
    }

    uint16_t BinaryReader::read_uint16()
    {
        uint8_t bytes[2];
        file_stream.read(reinterpret_cast<char*>(bytes), 2);
        if (is_endian_different)
        {
            return bytes[1] | (bytes[0] << 8);
        }
        else
        {
            return *reinterpret_cast<uint16_t*>(&bytes);
        }
    }

    uint8_t BinaryReader::read_uint8()
    {
        uint8_t byte;
        file_stream.read(reinterpret_cast<char*>(&byte), 1);
        return byte;
    }
}
