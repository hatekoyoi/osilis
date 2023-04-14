#include "graphics.hpp"

// RGB形式で8ビットごとに予約しているピクセル情報を書き込む
void
RGBResv8BitPerColorPixelWriter::Write(int x, int y, const PixelColor& c) {
    // x,y座標に色情報を書き込む
    auto p = PixelAt(x, y);
    p[0] = c.r;
    p[1] = c.g;
    p[2] = c.b;
}

// BGR形式で8ビットごとに予約しているピクセル情報を書き込む
void
BGRResv8BitPerColorPixelWriter::Write(int x, int y, const PixelColor& c) {
    // x,y座標に色情報を書き込む
    auto p = PixelAt(x, y);
    p[0] = c.r;
    p[1] = c.g;
    p[2] = c.r;
}