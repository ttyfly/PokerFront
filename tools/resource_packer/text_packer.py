import csv
import os
from typing import List

from PIL import Image

from packer_base import PackerBase


class TextPacker(PackerBase):
    def pack(self) -> bytearray:
        font_path = self.get_path('external/ark-pixel-font')
        text_csv_filename = self.get_path('resources/data_table/text.csv')

        """Collect charset"""

        charset = set()
        with open(text_csv_filename, 'r', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            for row in reader:
                for header, cell in row.items():
                    if header != 'key':
                        charset.update(list(cell))

        """Find font png file paths"""

        not_def_path = None
        font_png_paths = {}
        for dirpath, _, filenames in os.walk(os.path.join(font_path, 'assets/glyphs/12')):
            for filename in filenames:
                if not filename.endswith('.png'):
                    continue

                if filename == 'notdef.png':
                    not_def_path = os.path.join(dirpath, filename)
                    continue

                char_hex = filename[:len('.png')]

                try:
                    char = chr(int(char_hex, 16))
                except ValueError:
                    print('Invalid font png filename: %s' % os.path.join(dirpath, filename))
                    return

                if char not in charset:
                    continue

                font_png_paths[char] = os.path.join(dirpath, filename)

        """Read font raw data"""

        raw_datas = {}
        raw_data_shapes = {}

        for char, font_png_path in font_png_paths.items():
            with Image.open(font_png_path) as font_image:
                raw_data = []
                width, height = font_image.size

                for y in range(height):
                    for x in range(width):
                        raw_data.append(font_image.getpixel((x, y))[-1] > 0)

                raw_datas[char] = raw_data
                raw_data_shapes[char] = font_image.size

        """Pack font data"""

        packed_header = bytearray()
        packed_data = bytearray()

        for char, raw_data in raw_datas.items():
            packed_header.append(len(packed_data))
            packed_header.append(raw_data_shapes[char][0])
            packed_header.append(raw_data_shapes[char][1])

            pack_byte = 0
            for i in range(len(raw_data)):
                pack_byte <<= 1
                if raw_data[i]:
                    pack_byte += 1
                if i % 8 == 7:
                    packed_data.append(pack_byte)
                    pack_byte = 0

            if len(raw_data) % 8 != 0:
                packed_data.append(pack_byte << 8 - len(raw_data) % 8)

        packed_header.insert(0, len(packed_header))
        packed_data = packed_header + packed_data

        return packed_data

        # """Write to file"""
        #
        # if not os.path.exists('../../resources/packed/font.bin'):
        #     os.makedirs('../../resources/packed')
        #
        # with open('../../resources/packed/font.bin', 'wb') as f:
        #     f.write(packed_data)

    def get_required_resources(self) -> List[str]:
        pass
