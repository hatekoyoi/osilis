#include "font.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>

void*
operator new(size_t size, void* buf) {
    return buf;
}

void
operator delete(void* obj) noexcept {}

// PixelWriterを格納するためのバッファ
char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;

// カーネルエントリポイント
extern "C" void
KernelMain(const FrameBufferConfig& frame_buffer_config) {
    // ピクセルフォーマットに応じて、RGBまたはBGRのPixelWriterを作成
    switch (frame_buffer_config.pixel_format) {
        case kPixelRGBResv8BitPerColor:
            pixel_writer = new (pixel_writer_buf) RGBResv8BitPerColorPixelWriter{ frame_buffer_config };
            break;
        case kPixelBGRResv8BitPerColor:
            pixel_writer = new (pixel_writer_buf) BGRResv8BitPerColorPixelWriter{ frame_buffer_config };
            break;
    }

    // 全てのピクセルを白色で塗りつぶす
    for (int x = 0; x < frame_buffer_config.horizontal_resolution; ++x) {
        for (int y = 0; y < frame_buffer_config.vertical_resolution; ++y) {
            pixel_writer->Write(x, y, { 255, 255, 255 });
        }
    }

    // (0,0)から(199,99)までの領域を緑色で塗りつぶす
    for (int x = 0; x < 200; ++x) {
        for (int y = 0; y < 100; ++y) {
            pixel_writer->Write(x, y, { 0, 255, 0 });
        }
    }

    int i = 0;
    for (char c = '!'; c <= '~'; ++c, ++i) {
        WriteAscii(*pixel_writer, 8 * i, 50, c, { 0, 0, 0 });
    }
    WriteString(*pixel_writer, 0, 66, "Hello, world!", { 0, 0, 255 });

    char buf[128];
    sprintf(buf, "1 + 2 = %d", 1 + 2);
    WriteString(*pixel_writer, 0, 82, buf, { 0, 0, 0 });

    while (1)
        __asm__("hlt");
}
