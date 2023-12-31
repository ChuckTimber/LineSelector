# LineSelector

[ラインセレクタ - DIY Fx and Synthesizer](https://scrapbox.io/diyfx/%E3%83%A9%E3%82%A4%E3%83%B3%E3%82%BB%E3%83%AC%E3%82%AF%E3%82%BF)

## 仕様
- 基本仕様は A-B-Thru
- I/O は，Input / Output / SendA-ReturnA / SendB-ReturnB / TunerOut
- ２ボタン（A/B切替ボタン と Thru/FX切替ボタン）コントロール
- A/B切り替えボタンで SendA-ReturnA / SendB-ReturnB を切り替え
- Thru/FX ボタン長押しで Tunerモード（本線系はミュートしてチューナにのみ信号を流す）．Thru/FXボタンかA/Bボタンで復帰．
- なお切り替え制御はワンチップマイコン使用

## 回路図

- [version 1.0](Line_Selector_v1p0.pdf) 一部間違いあり，基板パターンを改修した上で動作実績あり
- [version 1.1](Line_Selector_v1p1.pdf) v.1.1を修正したもの．ただし基板は製作しておらず未検証

## 基板修正図

- [Patch 1.0 to 1.1](Patch_v1p0_to_v1p1.pdf) verion 1.0 の基板に対するパッチ情報（基板カットとジャンパ線）
