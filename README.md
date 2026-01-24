# gtk-desktop-test

GTK4を使用したシンプルなデスクトップアプリケーションです。ウィンドウモードとフルスクリーンモードの切り替え機能を備えています。

## 機能

- F12キーでプロパティダイアログを表示
- ウィンドウモード/フルスクリーンモードの切り替え
- コマンドラインからウィンドウサイズ指定が可能

## 必要条件

- GTK4開発ライブラリ
- GCC
- pkg-config

### インストール方法

```bash
# Ubuntu/Debian
sudo apt install libgtk-4-dev

# Fedora
sudo dnf install gtk4-devel

# Arch Linux
sudo pacman -S gtk4
```

## ビルド

```bash
make
```

依存関係の確認:
```bash
make check-deps
```

## 使い方

```bash
./gtk-desktop-test [OPTIONS]
```

### オプション

| オプション | 説明 |
|-----------|------|
| `--geometry WxH` | ウィンドウサイズを指定（例: `--geometry 1024x768`） |
| `--fullscreen` | フルスクリーンモードで起動 |
| `--help` | ヘルプを表示 |

### 例

```bash
# デフォルトサイズ（800x600）で起動
./gtk-desktop-test

# 1920x1080のウィンドウサイズで起動
./gtk-desktop-test --geometry 1920x1080

# フルスクリーンで起動
./gtk-desktop-test --fullscreen
```

## 既知の制限事項（WSL2/WSLg）

WSLg環境では以下の制限があります：

- 非プライマリモニターでフルスクリーンが正常に動作しない場合があります
- F12キーが反応しない、ダイアログがフルスクリーンウィンドウの後ろに表示される、フォーカスの問題が発生する可能性があります
- これはWSLgのWayland/X11ブリッジの制限であり、本アプリケーションの問題ではありません
- **回避策**: プライマリモニターでのみフルスクリーンを使用してください

## ライセンス

MIT
