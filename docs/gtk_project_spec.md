# Project Specification: Dynamic GTK4 Dashboard (Wayland/WSL2)

## 1. Project Overview

JSON 設定ファイルを読み込み、ウィジェットと図形を動的に配置・描画する GTK4 アプリケーション。
ターゲット環境は **WSL2 (Ubuntu) + Wayland**。

## 2. Core Architectural Requirements

### 2.1 Window Management

* **Fullscreen:** Wayland の絶対座標制約を回避するため、フルスクリーンモード (`gtk_window_fullscreen`) で起動する。
* **Coordinate System:** ウィンドウの `(0,0)` をモニター左上に一致させ、JSON の座標でウィジェットを配置する。
* **Window Config:** `layout.json` の `window` オブジェクトから `title`, `width`, `height`, `background_color` を取得する。

### 2.2 Layout Engine

* **Root Container:** `GtkFixed` を使用し、全ウィジェット・図形を絶対座標で配置する。
* **Positioning:** `geometry` の `x`, `y` で `gtk_fixed_put()` を呼び出す。
* **Z-Order:** `widgets` 配列のインデックス順。先頭が最背面、末尾が最前面。

### 2.3 Dependencies & Build System

* **Language:** C (C11)
* **Build System:** Meson (`meson.build`) または `build.sh`
* **Libraries:**
    * `gtk4`
    * `json-glib-1.0`
    * `libm` (math — Cairo 図形描画用)

## 3. Supported Widget Types

ファクトリパターンで JSON の `type` 文字列からウィジェットを生成する。
type 名は `docs/json_spec.md` に準拠。

### A. Basic Controls

1. **Button** (`GtkButton`)
   * *Props:* `label` (string), `icon_name` (string, 空文字=なし)
   * *Events:* `clicked`

2. **Label** (`GtkLabel`)
   * *Props:* `label` (string), `font_size` (int, px)
   * *Events:* なし

3. **Entry** (`GtkEntry`)
   * *Props:* `placeholder` (string), `text` (string)
   * *Events:* `activate`

4. **Checkbox** (`GtkCheckButton`)
   * *Props:* `label` (string), `checked` (bool)
   * *Events:* `toggled`

5. **Switch** (`GtkSwitch`)
   * *Props:* `label` (string, 横に GtkLabel を配置), `active` (bool)
   * *Events:* `toggled`

### B. Selection & Input

6. **Combo** (`GtkComboBoxText`)
   * *Props:* `items` (カンマ区切り文字列), `active_index` (int)
   * *Events:* `changed`

7. **Slider** (`GtkScale`)
   * *Props:* `min` (number), `max` (number), `step` (number), `value` (number)
   * *Events:* `value_changed`

8. **Spin** (`GtkSpinButton`)
   * *Props:* `min` (number), `max` (number), `step` (number), `value` (number)
   * *Events:* `value_changed`

### C. Visual & Layout

9. **Image** (`GtkImage`)
   * *Props:* `file_path` (string), `alt_text` (string, file_path が空の場合フォールバック表示)
   * *Events:* なし

10. **Progress** (`GtkProgressBar`)
    * *Props:* `value` (number, 0.0〜1.0), `show_text` (bool)
    * *Events:* なし

11. **Separator** (`GtkSeparator`)
    * *Props:* `orientation` ("horizontal" | "vertical")
    * *Events:* なし

## 4. Supported Shape Types (Cairo Rendering)

図形は `GtkDrawingArea` + Cairo でカスタム描画する。
`style` は `"transparent"` 固定。色情報は `props` 内の `fill_color` / `stroke_color` で指定。

1. **Line** — 直線
   * *Props:* `stroke_color`, `stroke_width`, `direction` ("horizontal" | "vertical" | "diagonal-se" | "diagonal-ne")

2. **Rect** — 矩形
   * *Props:* `fill_color`, `stroke_color`, `stroke_width`, `border_radius`

3. **Ellipse** — 楕円
   * *Props:* `fill_color`, `stroke_color`, `stroke_width`

4. **Triangle** — 三角形
   * *Props:* `fill_color`, `stroke_color`, `stroke_width`, `direction` ("up" | "down" | "left" | "right")

5. **Diamond** — ひし形
   * *Props:* `fill_color`, `stroke_color`, `stroke_width`

6. **Arrow** — 矢印線
   * *Props:* `stroke_color`, `stroke_width`, `direction` ("right" | "left" | "up" | "down")

7. **Star** — 星形
   * *Props:* `fill_color`, `stroke_color`, `stroke_width`, `points` (int, 3〜20)

## 5. Configuration Schema (JSON)

詳細は [json_spec.md](json_spec.md) を参照。

```json
{
  "window": {
    "title": "Control Panel",
    "width": 1920,
    "height": 1080,
    "background_color": "#2E3440"
  },
  "widgets": [
    {
      "id": "btn_main",
      "type": "Button",
      "geometry": { "x": 50, "y": 50, "width": 150, "height": 60 },
      "style": { "background_color": "#D08770", "color": "#FFFFFF" },
      "props": { "label": "EMERGENCY STOP", "icon_name": "" },
      "events": { "clicked": "on_emergency_stop" }
    },
    {
      "id": "slider_vol",
      "type": "Slider",
      "geometry": { "x": 50, "y": 150, "width": 300, "height": 40 },
      "style": { "background_color": "#ECEFF4", "color": "#A3BE8C" },
      "props": { "min": 0, "max": 100, "step": 1, "value": 50 },
      "events": { "value_changed": "on_volume_change" }
    },
    {
      "id": "rect_bg",
      "type": "Rect",
      "geometry": { "x": 30, "y": 20, "width": 400, "height": 300 },
      "style": { "background_color": "transparent", "color": "transparent" },
      "props": { "fill_color": "#3B4252", "stroke_color": "#4C566A", "stroke_width": 1, "border_radius": 12 },
      "events": {}
    }
  ]
}
```

## 6. Module Structure

| モジュール | ファイル | 責務 |
|-----------|---------|------|
| Entry Point | `src/main.c` | アプリケーション起動 |
| Application | `src/app.h/c` | ウィンドウ管理、レイアウト構築、キーイベント |
| JSON Parser | `src/json_parser.h/c` | `layout.json` のパース |
| Widget Factory | `src/widget_factory.h/c` | type 名からウィジェット生成 |
| Shape Renderer | `src/shape_renderer.h/c` | Cairo による図形描画 |
| Style Manager | `src/style_manager.h/c` | CSS スタイル生成・適用 |

### Processing Flow

```
main()
  → dashboard_app_run()
    → layout_config_load_from_file()  // JSON パース
    → on_activate()
      → create window (title, width, height from config)
      → build_dashboard()
        → for each widget in config:
          → is_shape_type()?
            → Yes: shape_renderer_create()  // Cairo 描画
            → No:  widget_factory_create()  // GTK ウィジェット
          → gtk_fixed_put() で配置
          → style_manager で CSS 適用 (ウィジェットのみ)
      → style_manager_apply()
```

## 7. Style System

* ウィジェットの `style` オブジェクトから `background_color`, `color` を CSS に変換。
* CSS セレクタはウィジェット ID: `#button_1 { background: ...; color: ...; }`
* ウィンドウ背景色: `window { background-color: ...; }`
* 図形の `style` は `"transparent"` 固定。図形の色は `props` で制御。

## 8. Known Limitations (WSL2/WSLg)

* 非プライマリモニターでフルスクリーンが正常に動作しない場合がある。
* F12 キーが反応しない、ダイアログがフルスクリーンウィンドウの後ろに表示される場合がある。
* WSLg の Wayland/X11 ブリッジの制限。プライマリモニターでの使用を推奨。
