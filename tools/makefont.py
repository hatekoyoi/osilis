#!/usr/bin/python3

import argparse
import collections
import functools
import re
import sys


BITMAP_PATTERN = re.compile(r'([.*@]+)')

# フォントデータをバイト列にコンパイル
def compile(src: str) -> bytes:
    # 先頭の空白文字を除去する
    src = src.lstrip()
    result = []

    # フォントデータを行単位で処理する
    for line in src.splitlines():
        # 行がビットマップパターンに一致するかを確認する
        m = BITMAP_PATTERN.match(line)
        if not m:
            continue

        # ビットマップパターンをビット配列に変換する
        bits = [(0 if x == '.' else 1) for x in m.group(1)]
        # ビット配列を整数に変換する
        bits_int = functools.reduce(lambda a, b: 2*a + b, bits)
        # 整数をバイト列に変換して結果に追加する
        result.append(bits_int.to_bytes(1, byteorder='little'))
    
    # 結果をバイト列に変換して返す
    return b''.join(result)


def main():
    # 引数を解析する
    parser = argparse.ArgumentParser()
    parser.add_argument('font', help='path to a font file')
    parser.add_argument('-o', help='path to an output file', default='font.out')
    ns = parser.parse_args()

    # 出力ファイルとフォントファイルを開く
    with open(ns.o, 'wb') as out, open(ns.font) as font:
        # フォントファイルからデータを読み込む
        src = font.read()
        # フォントデータをバイト列にコンパイルして出力ファイルに書き込む
        out.write(compile(src))


if __name__ == '__main__':
    main()
