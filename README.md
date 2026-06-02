**日本語** | [English](README.en.md) | [简体中文](README.zh-CN.md)

<div align="center">

# Greystone

**Unreal Engine 5 で開発中のアクション RPG**

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5-0E1128?logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)
![Platform](https://img.shields.io/badge/platform-Windows-0078D6)
![License](https://img.shields.io/badge/license-TBD-lightgrey)

</div>

---

## 概要

**Greystone** は、Unreal Engine 5 と C++ で開発しているソウルライク・アクション RPG です。
GAS（Gameplay Ability System）プラグインに依存せず、軽量な独自アビリティ基盤 **「Mini-GAS」** を自作することで、ソウルライク特有の重厚な戦闘の手触りと、拡張しやすい設計の両立を目指しています。

本リポジトリは、ゲームプログラマーとしての就職活動用ポートフォリオとして公開しています。

## 主な特徴

### 戦闘システム

- **ライトアタック** — コンボ対応。`AnimNotify` による精密なダメージ判定ウィンドウ
- **ヘビーアタック** — 長押しチャージ式、2 段階の威力
- **ローリング回避** — 無敵フレーム（I-Frame）付き。唯一ルートモーションを使用するアクション
- **ガード** — 正面コーン判定、スタミナ消費、ガードブレイク
- **スタミナ管理** — 行動コストとリソース制御

### アーキテクチャ

- 商用プラグインに頼らない、自作の軽量アビリティ基盤 **Mini-GAS**
- 全モンタージュを `DefaultGroup.DefaultSlot` に統一し、再生制御をシンプルに
- アクションごとの開始条件（クールダウン・スタミナ・状態）を一元的に判定する仕組み

## 技術スタック

| 項目 | 内容 |
| --- | --- |
| エンジン | Unreal Engine 5 |
| 言語 | C++ / Blueprint |
| アビリティ基盤 | 独自実装 Mini-GAS |
| キャラクター | UE5 Mannequin (Quinn) |
| アニメーション | Fab Marketplace |

## スクリーンショット

<!-- プレイ映像の GIF やスクリーンショットを配置してください 例: -->
<!-- ![Gameplay](docs/images/gameplay.gif) -->

> 準備中

## ビルド方法

> **必要環境**: Unreal Engine 5.x / Visual Studio 2022（C++ ワークロード）/ Git LFS

```bash
git clone https://github.com/<your-account>/Greystone.git
```

1. `Greystone.uproject` を右クリックし、**Generate Visual Studio project files** を実行
2. 生成された `.sln` を Visual Studio で開き、**Development Editor** 構成でビルド
3. `Greystone.uproject` を Unreal Engine 5 で開く
4. エディタ上で **Play** を実行

## 操作方法

| 操作 | キー |
| --- | --- |
| 移動 | `W` `A` `S` `D` |
| 視点 | マウス |
| ライトアタック | 左クリック |
| ヘビーアタック | 右クリック（長押しでチャージ） |
| 回避（ローリング） | `Space` |
| ガード | `e` |

<!-- 実際のキーバインドに合わせて編集してください -->

## ロードマップ

- [x] P1: 基本戦闘（ライト / ヘビー / 回避 / ガード）
- [ ] 敵 AI と戦闘ループ
- [ ] ロックオン
- [ ] レベルデザイン
- [ ] UI / HUD

## クレジット

- キャラクター: Fab Marketplace Paragon GreyStone 
- アニメーション: Fab Marketplace


## 連絡先

- GitHub: https://github.com/chynokagami
- Email: chynokagami@gmail.com
