/**
 * @file layer.hpp
 *
 * 重ね合わせ処理を提供する
 */

#pragma once

#include <map>
#include <memory>
#include <vector>

#include "graphics.hpp"
#include "window.hpp"

/**
 * @brief Layerは1つの層を表す
 *
 * 現状では1つのウィンドウしか保持できない設計だが、
 * 将来的には複数のウィンドウを持ち得る
 */
class Layer {
  public:
    /** @brief 指定されたIDを持つレイヤーを生成する */
    Layer(unsigned int id = 0);
    /** @brief このインスタンスのIDを返す */
    unsigned int ID() const;

    /** @brief ウィンドウを設定する 既存のウィンドウはこのレイヤーから外れる */
    Layer& SetWindow(const std::shared_ptr<Window>& window);
    /** @brief 設定されたウィンドウを返す */
    std::shared_ptr<Window> GetWindow() const;

    /** @brief レイヤーの位置情報を指定された絶対座標へと更新する 再描画はしない */
    Layer& Move(Vector2D<int> pos);
    /** @brief レイヤーの位置情報を指定された相対座標へと更新する 再描画はしない */
    Layer& MoveRelative(Vector2D<int> pos_diff);

    /** @brief writerに現在設定されているウィンドウの内容を描画する */
    void DrawTo(PixelWriter& writer) const;

  private:
    unsigned int id_;
    Vector2D<int> pos_;
    std::shared_ptr<Window> window_;
};

/** @brief LayerManagerは複数のレイヤーを管理する */
class LayerManager {
  public:
    /** @brief Drawメソッドなどで描画する際の描画先を設定する */
    void SetWriter(PixelWriter* writer);
    /**
     * @brief 新しいレイヤーを生成して参照を返す
     *
     * 新しく生成されたレイヤーの実態はLayerManager内部のコンテナで保持される
     */
    Layer& NewLayer();

    /** @brief 現在表示状態にあるレイヤーを描画する */
    void Draw() const;

    /** @brief レイヤーの位置情報を指定された絶対座標へと更新する 再描画はしない */
    void Move(unsigned int id, Vector2D<int> new_position);
    /** @brief レイヤーの位置情報を指定された相対座標へと更新する 再描画はしない */
    void MoveRelative(unsigned int id, Vector2D<int> pos_diff);

    /**
     * @brief レイヤーの高さ方向の位置を指定された位置に移動する
     *
     * new_heightに負の高さを指定するとレイヤーは非表示となり
     * 0以上を指定するとその高さとなる
     * 現在のレイヤー数以上の数値を指定した場合は最前面のレイヤーとなる
     */
    void UpDown(unsigned int id, int new_height);
    /** @brief レイヤーを非表示とする */
    void Hide(unsigned int id);

  private:
    PixelWriter* writer_{ nullptr };
    std::vector<std::unique_ptr<Layer>> layers_{};
    std::vector<Layer*> layer_stack_{};
    unsigned int latest_id_{ 0 };

    Layer* FindLayer(unsigned int id);
};

extern LayerManager* layer_manager;