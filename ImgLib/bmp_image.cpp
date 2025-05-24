#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

	struct BmpColor {
		BmpColor(byte blue, byte green, byte red) :b(blue), g(green), r(red) {};
		BmpColor() = default;
		byte b;
		byte g;
		byte r;
	};

	PACKED_STRUCT_BEGIN BitmapFileHeader{
		// поля заголовка Bitmap File Header
	char sign[2];
	uint32_t file_size;
	uint32_t reserve;
	uint32_t offset;
	}
		PACKED_STRUCT_END

		PACKED_STRUCT_BEGIN BitmapInfoHeader{
		// поля заголовка Bitmap Info Header
	uint32_t header_size;
	int32_t width;
	int32_t height;
	uint16_t plate=1;
	uint16_t bpp = 24;
	uint32_t type = 0;
	uint32_t data_bite;
	int32_t hor_pbm = 11811;
	int32_t vert_bpm = 11811;
	int32_t colors = 0;
	int32_t true_colors = 0x1000000;
	}
		PACKED_STRUCT_END

		// функция вычисления отступа по ширине
		static int GetBMPStride(int w) {
		return 4 * ((w * 3 + 3) / 4);
	}
	BitmapFileHeader InitBitmapFileHeader(const Image& image) {
		BitmapFileHeader result;
		result.sign[0] = 'B';
		result.sign[1] = 'M';
		result.offset = sizeof(BitmapFileHeader)+sizeof(BitmapInfoHeader);
		result.reserve = 0;
		result.file_size = GetBMPStride(image.GetWidth()) * image.GetHeight() + result.offset;
		return result;
	}

	BitmapInfoHeader InitBitmapInfoHeader(const Image& image) {
		BitmapInfoHeader result;
		result.header_size = sizeof(BitmapInfoHeader);
		result.width = image.GetWidth();
		result.height = image.GetHeight();
		result.data_bite = GetBMPStride(image.GetWidth()) * image.GetHeight();
		return result;
	}

	// напишите эту функцию
	bool SaveBMP(const Path& file, const Image& image) {
		BitmapFileHeader bmp_file_header = InitBitmapFileHeader(image);
		BitmapInfoHeader bmp_info_header = InitBitmapInfoHeader(image);
		ofstream bmp_file(file,ios::binary);
		if (!bmp_file) {
			return false;
		}
		bmp_file.write(reinterpret_cast<char*>(&bmp_file_header), sizeof(BitmapFileHeader));
		bmp_file.write(reinterpret_cast<char*>(&bmp_info_header), sizeof(BitmapInfoHeader));
		int width = image.GetWidth();
		int height = image.GetHeight();
		vector<byte> offset(GetBMPStride(width) - width * 3);
		for (int y = height-1; y >=0; --y) {
			for (int x = 0; x < width; ++x) {
				Color ppm_color = image.GetPixel(x, y);
				BmpColor bmp_color(ppm_color.b, ppm_color.g, ppm_color.r);
				bmp_file.write(reinterpret_cast<char*>(&bmp_color), sizeof(BmpColor));
			}
			if (offset.size() > 0) {
				bmp_file.write(reinterpret_cast<char*>(offset.data()), offset.size());
			}
		}
		bmp_file.close();
		return true;
	}

	// напишите эту функцию
	Image LoadBMP(const Path& file) {
	
		ifstream bmp_file(file,ios::binary);
		if (!bmp_file) {
			return Image();
		}
		
		BitmapFileHeader bmp_file_header;
		bmp_file.read(reinterpret_cast<char*>(&bmp_file_header), sizeof(BitmapFileHeader));
		if (bmp_file_header.sign[0] != 'B' && bmp_file_header.sign[1] != 'M') {
			return Image();
		}
		BitmapInfoHeader bmp_info_header;
		bmp_file.read(reinterpret_cast<char*>(&bmp_info_header), sizeof(BitmapInfoHeader));
		int width = bmp_info_header.width;
		int height = bmp_info_header.height;
		Image image(width, height, img_lib::Color::Black());
		int offset = GetBMPStride(width)-3*width;
		for (int y = 0; y < height; ++y) {

			for (int x = 0; x < width; ++x) {
				BmpColor bmp_color;
				bmp_file.read(reinterpret_cast<char*>(&bmp_color), sizeof(BmpColor));
				img_lib::Color& ppm_color = image.GetPixel(x, height-y-1);
				ppm_color.r = bmp_color.r;
				ppm_color.g = bmp_color.g;
				ppm_color.b = bmp_color.b;
				ppm_color.a = (byte)255;
			}
			bmp_file.ignore(offset);
		}
		bmp_file.close();
	

		return image;
	}

}

