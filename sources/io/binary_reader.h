#include <cstdint>
#include <string>
#include <fstream>

namespace pf_io
{
    enum class Endian
    {
#if defined(_MSC_VER) && !defined(__clang__)
        Little = 0,
        Big    = 1,
        Native = Little
#else
        Little = __ORDER_LITTLE_ENDIAN__,
        Big    = __ORDER_BIG_ENDIAN__,
        Native = __BYTE_ORDER__
#endif
    };

    class BinaryReader
    {
    public:
        BinaryReader(std::string filename, Endian endian=Endian::Native);
        ~BinaryReader();

        uint32_t read_uint32();
        uint16_t read_uint16();
        uint8_t read_uint8();

    private:
        bool is_endian_different;
        std::ifstream file_stream;
    };
}