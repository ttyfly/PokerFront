#include <fstream>
#include <vector>

namespace pf
{
    class FontCharacterInfo
    {
        uint32_t offset;
        uint8_t width;
        uint8_t height;
    };

    class BinaryTextData
    {
    public:
        void load(std::ifstream file_stream);
        uint8_t* get_font_data(int index, int& out_width, int& out_height);

    private:
        std::vector<FontCharacterInfo> font_info_list;
        std::vector<uint8_t> font_data;
    };
}