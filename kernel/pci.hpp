#pragma once

#include <array>
#include <cstdint>

#include "error.hpp"

namespace pci {
    // CONFIG_ADDRESS レジスタのIOポートアドレス
    const uint16_t kConfigAddress = 0x0cf8;
    // CONFIG_DATA レジスタのIOポートアドレス
    const uint16_t kConfigData = 0x0cfc;

    // CONFIG_ADDRESS に指定された整数を書き込む
    void WriteAddress(uint32_t address);
    // CONFIG_DATA に指定された整数を書き込む
    void WriteData(uint32_t value);
    // CONFIG_DATA から32ビット整数を読み込む
    uint32_t ReadData();

    // ベンダIDレジスタを読み取る(全ヘッダタイプ共通)
    uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function);
    // デバイスIDレジスタを読み取る(全ヘッダタイプ共通)
    uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function);
    // ヘッダタイプレジスタを読み取る(全ヘッダタイプ共通)
    uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function);
    /**
     * クラスコードレジスタを読み取る(全ヘッダタイプ共通)
     * 返される32ビット整数の構造は次の通り
     * - 31:24  : ベースクラス
     * - 23:16  : サブクラス
     * - 15:8   : インターフェース
     * - 7:0    : リビジョン
     */
    uint32_t ReadClassCode(uint8_t bus, uint8_t device, uint8_t function);

    /**
     * バス番号レジスタを読み取る(ヘッダタイプ1用)
     * 返される32ビット整数の構造は次の通り
     * - 23:16  : サブオーディネイトバス番号
     * - 15:8   : セカンダリバス番号
     * - 7:0    : リビジョン番号
     */
    uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function);

    // 単一ファンクションの場合に真を返す
    bool IsSingleFunctionDevice(uint8_t header_type);

    /**
     * PCIデバイスを操作するための基礎データを格納す
     * バス番号、デバイス番号、ファンクション番号はデバイスを特定するのに必須
     * その他の情報は単に利便性のために加えていある
     */
    struct Device {
        uint8_t bus, device, function, header_type;
    };

    // ScanAllBus()により発見されたPCIデバイスの一覧
    inline std::array<Device, 32> devices;
    // devicesの有効な要素の数
    inline int num_device;
    /**
     * PCIデバイスをすべて探索しdeviceに格納する
     * バス0から再帰的にPCIデバイスを探索しdevicesの先頭から詰めて書き込む
     * 発見したデバイスの数をnum_devicesに設定する
     */
    Error ScanAllBus();
}