#include "graphics.hpp"

// RGB形式で8ビットごとに予約しているピクセル情報を書き込む
void
RGBResv8BitPerColorPixelWriter::Write(Vector2D<int> pos, const PixelColor& c) {
    // x,y座標に色情報を書き込む
    auto p = PixelAt(pos);
    p[0] = c.r;
    p[1] = c.g;
    p[2] = c.b;
}

// BGR形式で8ビットごとに予約しているピクセル情報を書き込む
void
BGRResv8BitPerColorPixelWriter::Write(Vector2D<int> pos, const PixelColor& c) {
    // x,y座標に色情報を書き込む
    auto p = PixelAt(pos);
    p[0] = c.b;
    p[1] = c.g;
    p[2] = c.r;
}

void
DrawRectangle(PixelWriter& writer,
              const Vector2D<int>& pos,
              const Vector2D<int>& size,
              const PixelColor& c) {
    for (int dx = 0; dx < size.x; ++dx) {
        writer.Write(pos + Vector2D<int>{ dx, 0 }, c);
        writer.Write(pos + Vector2D<int>{ dx, size.y - 1 }, c);
    }
    for (int dy = 1; dy < size.y - 1; ++dy) {
        writer.Write(pos + Vector2D<int>{ 0, dy }, c);
        writer.Write(pos + Vector2D<int>{ size.x - 1, dy }, c);
    }
}

void
FillRectangle(PixelWriter& writer,
              const Vector2D<int>& pos,
              const Vector2D<int>& size,
              const PixelColor& c) {
    for (int dy = 0; dy < size.y; ++dy) {
        for (int dx = 0; dx < size.x; ++dx) {
            writer.Write(pos + Vector2D<int>{ dx, dy }, c);
        }
    }
}

void
DrawDesktop(PixelWriter& writer) {
    const auto width = writer.Width();
    const auto height = writer.Height();
    FillRectangle(writer, { 0, 0 }, { width, height - 50 }, kDesktopBGColor);
    FillRectangle(writer, { 0, height - 50 }, { width, 50 }, { 1, 8, 17 });
    FillRectangle(writer, { 0, height - 50 }, { width / 5, 50 }, { 80, 80, 80 });
    DrawRectangle(writer, { 10, height - 40 }, { 30, 30 }, { 160, 160, 160 });
}
