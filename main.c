#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

typedef char i8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef int i32;
typedef unsigned u32;
typedef unsigned long u64;

#define PRINT_ERROR(cstring) write(STDERR_FILENO, cstring, sizeof(cstring) - 1)

#pragma pack(1)
struct bmp_header
{
	// Note: header
	i8  signature[2]; // should equal to "BM"
	u32 file_size;
	u32 unused_0;
	u32 data_offset;

	// Note: info header
	u32 info_header_size;
	u32 width; // in px
	u32 height; // in px
	u16 number_of_planes; // should be 1
	u16 bit_per_pixel; // 1, 4, 8, 16, 24 or 32
	u32 compression_type; // should be 0
	u32 compressed_image_size; // should be 0
	// Note: there are more stuff there but it is not important here
};

struct file_content
{
	i8*   data;
	u32   size;
};

struct file_content   read_entire_file(char* filename)
{
	char* file_data = 0;
	unsigned long	file_size = 0;
	int input_file_fd = open(filename, O_RDONLY);
	if (input_file_fd >= 0)
	{
		struct stat input_file_stat = {0};
		stat(filename, &input_file_stat);
		file_size = input_file_stat.st_size;
		file_data = mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, input_file_fd, 0);
		close(input_file_fd);
	}
	return (struct file_content){file_data, file_size};
}

void	huntHeader(struct bmp_header* header, struct file_content file_content) {
	if (header->bit_per_pixel != 32)
	{
		PRINT_ERROR("This example only supports 24-bit BMP files.\n");
		return ;
	}

	u8* pixel_data = (u8*)file_content.data + header->data_offset;
	u32 width = header->width;
	u32 height = header->height;
	u32 row_size = ((width * 3 + 3) / 4) * 4;

	u8 target_b = 127;
	u8 target_g = 188;
	u8 target_r = 217;

	for (u32 y = height - 1; y >= 0; y--)
	{
		for (u32 x = 0; x < width; x++)
		{
			u32 pixel_offset = y * row_size + x * 3;

			u8 blue = pixel_data[pixel_offset];
			u8 green = pixel_data[pixel_offset + 1];
			u8 red = pixel_data[pixel_offset + 2];

			if (blue == target_b && green == target_g && red == target_r)
			{
				printf("Found target pixel at (%u, %u)\n", x, y);
				if (x + 7 < width) {
                    u32 len_pixel_offset = y * row_size + (x + 7) * 4;
                    u8 len_blue = pixel_data[len_pixel_offset];
                    u8 len_green = pixel_data[len_pixel_offset + 1];
                    u8 len_red = pixel_data[len_pixel_offset + 2];
                    u8 len_alpha = pixel_data[len_pixel_offset + 3];

                    printf("Length pixel at (%u, %u) -> #####B: %u, G: %u, #####R: %u, A: %u\n", x + 7, y, len_blue, len_green, len_red, len_alpha);
                }
				return ;
			}
		}
	}
	printf("Target pixel (127, 188, 217) not found.\n");
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		PRINT_ERROR("Usage: decode <input_filename>\n");
		return 1;
	}
	struct file_content file_content = read_entire_file(argv[1]);
	if (file_content.data == NULL)
	{
		PRINT_ERROR("Failed to read file\n");
		return 1;
	}
	struct bmp_header* header = (struct bmp_header*) file_content.data;
	printf("signature: %.2s\nfile_size: %u\ndata_offset: %u\ninfo_header_size: %u\nwidth: %u\nheight: %u\nplanes: %i\nbit_per_px: %i\ncompression_type: %u\ncompression_size: %u\n", header->signature, header->file_size, header->data_offset, header->info_header_size, header->width, header->height, header->number_of_planes, header->bit_per_pixel, header->compression_type, header->compressed_image_size);

	printf("\n==================================================\n");
	huntHeader(header, file_content);

	printf("\n==================================================\n");

	return 0;
}

