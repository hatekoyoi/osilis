#pragma once

#include <array>
#include <cstdint>

#include "error.hpp"

namespace pci {
    // CONFIG_ADDRESS レジスタのIOポートアドレス
    const uint16_t kConfigAddress = 0x0cf8;
    // CONFIG_DATA レジスタのIOポートアドレス
    const uint16_t kConfigData = 0x0cfc;

    // PCIデバイスのクラスコード
    struct ClassCode {
        uint8_t base, sub, interface;

        // ベースクラスが等しい場合に真を返す
        bool Match(uint8_t b) { return b == base; }
        // ベースクラスとサブクラスが等しい場合に真を返す
        bool Match(uint8_t b, uint8_t s) { return Match(b) && s == sub; }
        // ベース、サブ、インターフェースが等しい場合に真を返す
        bool Match(uint8_t b, uint8_t s, uint8_t i) { return Match(b, s) && i == interface; }
    };

    /**
     * PCIデバイスを操作するための基礎データを格納する
     * バス番号、デバイス番号、ファンクション番号はデバイスを特定するのに必須
     * その他の情報は単に利便性のために加えてある
     */
    struct Device {
        uint8_t bus, device, function, header_type;
        ClassCode class_code;
    };

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
    ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function);

    inline uint16_t ReadVendorId(const Device& dev) {
        return ReadVendorId(dev.bus, dev.device, dev.function);
    }

    // 指定された PCI デバイスの 32 ビットレジスタを読み取る
    uint32_t ReadConfReg(const Device& dev, uint8_t reg_addr);

    void WriteConfReg(const Device& dev, uint8_t reg_addr, uint32_t value);

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

    constexpr uint8_t CalcBarAddress(unsigned int bar_index) {
        return 0x10 + 4 * bar_index;
    }

    WithError<uint64_t> ReadBar(Device& device, unsigned int bar_index);
}