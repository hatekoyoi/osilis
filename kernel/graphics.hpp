#pragma once

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
    uint8_t* PixelAt(int x, int y) { return config_.frame_buffer + 4 * (config_.pixels_per_scan_line * y + x); }

  private:
    const FrameBufferConfig& config_;
};

// RGB形式で8ビットごとに予約しているピクセル情報を書き込む
class RGBResv8BitPerColorPixelWriter : public PixelWriter {
  public:
    using PixelWriter::PixelWriter;

    // x,y座標に色情報を書き込む
    virtual void Write(int x, int y, const PixelColor& c) override;
};

// BGR形式で8ビットごとに予約しているピクセル情報を書き込む
class BGRResv8BitPerColorPixelWriter : public PixelWriter {
  public:
    using PixelWriter::PixelWriter;

    // x,y座標に色情報を書き込む
    virtual void Write(int x, int y, const PixelColor& c) override;
};

template<typename T>
struct Vector2D {
    T x, y;

    template<typename U>
    Vector2D<T>& operator+=(const Vector2D<U>& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
};

void
DrawRectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c);

void
FillRectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c);
