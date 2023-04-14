#include <cstddef>
#include <cstdint>

#include "frame_buffer_config.hpp"

// ピクセルの色情報
struct PixelColor {
    uint8_t r, g, b;
};

// ピクセル情報を書き込む
class PixelWriter {
    public:
    // FrameBufferConfig構造体を受け取るコンストラクタ
    PixelWriter(const FrameBufferConfig& config)
      : config_{ config } {}
    // 仮想デストラクタ
    virtual ~PixelWriter() = default;
    // x,y座標に色情報cを書き込む純粋仮想関数
    virtual void Write(int x, int y, const PixelColor& c) = 0;

    protected:
    // x,y座標のピクセルへのポインタを返す
    uint8_t* PixelAt(int x, int y) {
        return config_.frame_buffer + 4 * (config_.pixels_per_scan_line * y + x);
    }

    private:
    const FrameBufferConfig& config_;
};

// RGB形式で8ビットごとに予約しているピクセル情報を書き込む
class RGBResv8BitPerColorPixelWriter : public PixelWriter {
    public:
    using PixelWriter::PixelWriter;

    // x,y座標に色情報を書き込む
    virtual void Write(int x, int y, const PixelColor& c) override {
        auto p = PixelAt(x, y);
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
    }
};

// BGR形式で8ビットごとに予約しているピクセル情報を書き込む
class BGRResv8BitPerColorPixelWriter : public PixelWriter {
    public:
    using PixelWriter::PixelWriter;

    // x,y座標に色情報を書き込む
    virtual void Write(int x, int y, const PixelColor& c) override {
        auto p = PixelAt(x, y);
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.r;
    }
};

/**
 * WritePixel: 1つの点を描画
 * @retval 0    成功
 * @retval !0   失敗
 */
int
WritePixel(const FrameBufferConfig& config, int x, int y, const PixelColor& c) {
    const int pixel_position = config.pixels_per_scan_line * y + x;
    if (config.pixel_format == kPixelRGBResv8BitPerColor) {
        uint8_t* p = &config.frame_buffer[4 * pixel_position];
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
    } else if (config.pixel_format == kPixelBGRResv8BitPerColor) {
        uint8_t* p = &config.frame_buffer[4 * pixel_position];
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
    } else {
        return -1;
    }
    return 0;
}

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

    while (1)
        __asm__("hlt");
}
