# gtk-desktop-test

GTK4 + json-glib を使用した、JSON 駆動のデスクトップアプリケーションです。
`layout.json` を読み込み、ウィジェットと図形をネイティブ GUI として動的に構築します。

## 機能

- JSON ファイルからウィジェット・図形を動的生成
- 11 種のウィジェット (Button, Label, Entry, Checkbox, Switch, Combo, Slider, Spin, Image, Progress, Separator)
- 7 種の図形を Cairo で描画 (Line, Rect, Ellipse, Triangle, Diamond, Arrow, Star)
- GtkFixed による絶対座標配置
- CSS スタイリング (背景色・文字色)
- F12 でプロパティダイアログ / Ctrl+F12 でフルスクリーン切替

## 必要条件

- GTK4 開発ライブラリ
- json-glib-1.0
- GCC (C11)
- pkg-config

### インストール方法

```bash
# Ubuntu/Debian
sudo apt install libgtk-4-dev libjson-glib-dev

# Fedora
sudo dnf install gtk4-devel json-glib-devel

# Arch Linux
sudo pacman -S gtk4 json-glib
```

## ビルド

### build.sh (簡易ビルド)

```bash
bash build.sh
```

### Meson (推奨)

```bash
meson setup builddir
meson compile -C builddir
```

## 使い方

```bash
./builddir/gtk-dashboard [LAYOUT_FILE]
```

### 引数

| 引数 | 説明 |
|------|------|
| `LAYOUT_FILE` | レイアウト定義 JSON ファイルのパス |
| `--help` | ヘルプを表示 |

### 例

```bash
# layout.json を読み込んで起動
./builddir/gtk-dashboard layout.json

# ヘルプ表示
./builddir/gtk-dashboard --help
```

### キーボードショートカット

| キー | 動作 |
|------|------|
| F12 | プロパティダイアログを表示 |
| Ctrl+F12 | フルスクリーンモードを切替 |

## プロジェクト構成

```
├── src/
│   ├── main.c              # エントリポイント
│   ├── app.h / app.c       # アプリケーションライフサイクル・ウィンドウ管理
│   ├── json_parser.h / .c  # layout.json パーサ
│   ├── widget_factory.h / .c  # ウィジェット生成ファクトリ
│   ├── shape_renderer.h / .c  # Cairo 図形描画
│   └── style_manager.h / .c   # CSS スタイル管理
├── docs/
│   ├── json_spec.md         # layout.json 仕様書
│   └── gtk_project_spec.md  # プロジェクト仕様書
├── layout.json              # サンプルレイアウト
├── build.sh                 # 簡易ビルドスクリプト
└── meson.build              # Meson ビルド設定
```

## JSON スキーマ概要

詳細は [docs/json_spec.md](docs/json_spec.md) を参照。

```json
{
  "window": {
    "title": "My App",
    "width": 1920,
    "height": 1080,
    "background_color": "#2E3440"
  },
  "widgets": [
    {
      "id": "button_1",
      "type": "Button",
      "geometry": { "x": 50, "y": 50, "width": 120, "height": 40 },
      "style": { "background_color": "#ECEFF4", "color": "#2E3440" },
      "props": { "label": "Click Me", "icon_name": "" },
      "events": { "clicked": "on_button_click" }
    }
  ]
}
```

### 対応 type 一覧

**ウィジェット**: `Button`, `Label`, `Entry`, `Checkbox`, `Switch`, `Combo`, `Slider`, `Spin`, `Image`, `Progress`, `Separator`

**図形 (Cairo 描画)**: `Line`, `Rect`, `Ellipse`, `Triangle`, `Diamond`, `Arrow`, `Star`

## 既知の制限事項 (WSL2/WSLg)

- 非プライマリモニターでフルスクリーンが正常に動作しない場合があります
- F12 キーが反応しない、ダイアログがフルスクリーンウィンドウの後ろに表示される場合があります
- これは WSLg の Wayland/X11 ブリッジの制限です
- **回避策**: プライマリモニターでのみフルスクリーンを使用してください

## ライセンス

MIT
