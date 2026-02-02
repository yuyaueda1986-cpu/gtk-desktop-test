# layout.json 仕様書

Visual Layout Designer が出力する `layout.json` の完全なスキーマ定義。
このファイルを C/C++ ランタイム (GTK / Qt 等) で読み込み、ネイティブ GUI を構築する。

---

## 1. トップレベル構造

```json
{
  "window":  { ... },
  "widgets": [ ... ]
}
```

| キー | 型 | 必須 | 説明 |
|------|------|:----:|------|
| `window` | Object | Yes | ウィンドウ / 画面全体の設定 |
| `widgets` | Array\<Object\> | Yes | ウィジェット・図形の配列。配列順 = Z-order (先頭が最背面) |

---

## 2. `window` オブジェクト

```json
{
  "title": "Designed Interface",
  "width": 1920,
  "height": 1080,
  "background_color": "#2E3440"
}
```

| キー | 型 | 必須 | デフォルト | 説明 |
|------|------|:----:|-----------|------|
| `title` | String | Yes | `"Designed Interface"` | ウィンドウタイトル |
| `width` | Integer | Yes | `1920` | 画面幅 (px) |
| `height` | Integer | Yes | `1080` | 画面高さ (px) |
| `background_color` | String | Yes | `"#2E3440"` | 背景色 (`#RRGGBB`) |

**GTK**: `gtk_window_set_title()`, `gtk_window_set_default_size(w, h)`, CSS provider で `window { background: ...; }`
**Qt**: `setWindowTitle()`, `resize(w, h)`, `QPalette::setColor(QPalette::Window, ...)`

---

## 3. `widgets[]` 要素 — 共通構造

配列の各要素は **全タイプ共通** で以下 6 フィールドを持つ。

```json
{
  "id":       "button_1",
  "type":     "Button",
  "geometry": { "x": 50, "y": 50, "width": 120, "height": 40 },
  "style":    { "background_color": "#ECEFF4", "color": "#2E3440" },
  "props":    { ... },
  "events":   { ... }
}
```

| キー | 型 | 必須 | 説明 |
|------|------|:----:|------|
| `id` | String | Yes | 一意識別子。英数字 + `_` のみ。シグナル接続やルックアップに使用 |
| `type` | String | Yes | タイプ名。本書 Section 4/5 の値と完全一致 |
| `geometry` | Object | Yes | 配置とサイズ |
| `style` | Object | Yes | 外観スタイル |
| `props` | Object | Yes | タイプ固有プロパティ |
| `events` | Object | Yes | シグナルマッピング (図形タイプでは `{}`) |

### 3.1 `geometry`

```json
{ "x": 50, "y": 50, "width": 120, "height": 40 }
```

| キー | 型 | 単位 | 説明 |
|------|------|------|------|
| `x` | Integer | px | 左上隅 X 座標 (画面左端 = 0) |
| `y` | Integer | px | 左上隅 Y 座標 (画面上端 = 0) |
| `width` | Integer | px | 幅 (>= 1) |
| `height` | Integer | px | 高さ (>= 1) |

- 原点は画面左上 `(0, 0)`。X 軸は右方向、Y 軸は下方向に増加。
- GTK: `GtkFixed` + `gtk_fixed_put(fixed, widget, x, y)` + `gtk_widget_set_size_request(widget, width, height)`
- Qt: `QWidget::setGeometry(x, y, width, height)` (レイアウトマネージャは不使用)

### 3.2 `style`

```json
{ "background_color": "#ECEFF4", "color": "#2E3440" }
```

| キー | 型 | 説明 |
|------|------|------|
| `background_color` | String | 背景色。`#RRGGBB` または `"transparent"` |
| `color` | String | テキスト色。`#RRGGBB` または `"transparent"` |

- **ウィジェットタイプ**: `style` でウィジェット自体の背景色・文字色を制御する。
- **図形タイプ**: `style` は `"transparent"` 固定。色情報は `props` 内の `fill_color` / `stroke_color` を使用する。

---

## 4. `type` 一覧 — ウィジェット

| type | 説明 | GTK ウィジェット | Qt ウィジェット |
|------|------|-----------------|----------------|
| `Button` | 押下可能なボタン | `GtkButton` | `QPushButton` |
| `Label` | テキスト表示 | `GtkLabel` | `QLabel` |
| `Entry` | 1行テキスト入力 | `GtkEntry` | `QLineEdit` |
| `Checkbox` | チェックボックス | `GtkCheckButton` | `QCheckBox` |
| `Switch` | トグルスイッチ | `GtkSwitch` | カスタム / `QCheckBox` |
| `Combo` | ドロップダウンリスト | `GtkComboBoxText` | `QComboBox` |
| `Slider` | スライダー | `GtkScale` | `QSlider` |
| `Spin` | 数値スピンボタン | `GtkSpinButton` | `QSpinBox` |
| `Image` | 画像表示 | `GtkImage` | `QLabel` + `QPixmap` |
| `Progress` | プログレスバー | `GtkProgressBar` | `QProgressBar` |
| `Separator` | 区切り線 | `GtkSeparator` | `QFrame` |

### 4.1 `props` 定義 — Button

```json
{ "label": "Click Me", "icon_name": "" }
```

| キー | 型 | 説明 |
|------|------|------|
| `label` | String | ボタンラベルテキスト |
| `icon_name` | String | アイコン名 (空文字 = なし)。GTK: `gtk_button_set_icon_name()` / Qt: `QIcon::fromTheme()` |

### 4.2 `props` 定義 — Label

```json
{ "label": "Hello", "font_size": 14 }
```

| キー | 型 | 説明 |
|------|------|------|
| `label` | String | 表示テキスト |
| `font_size` | Integer | フォントサイズ (px) |

### 4.3 `props` 定義 — Entry

```json
{ "placeholder": "Enter text...", "text": "" }
```

| キー | 型 | 説明 |
|------|------|------|
| `placeholder` | String | プレースホルダテキスト |
| `text` | String | 初期テキスト |

### 4.4 `props` 定義 — Checkbox

```json
{ "label": "Checkbox", "checked": false }
```

| キー | 型 | 説明 |
|------|------|------|
| `label` | String | ラベルテキスト |
| `checked` | Boolean | 初期チェック状態 |

### 4.5 `props` 定義 — Switch

```json
{ "label": "Switch", "active": false }
```

| キー | 型 | 説明 |
|------|------|------|
| `label` | String | スイッチ横のラベル (GTK では別途 `GtkLabel` を配置) |
| `active` | Boolean | 初期 ON/OFF 状態 |

### 4.6 `props` 定義 — Combo

```json
{ "items": "Item 1,Item 2,Item 3", "active_index": 0 }
```

| キー | 型 | 説明 |
|------|------|------|
| `items` | String | カンマ区切りの選択肢。パース時に `,` で split する |
| `active_index` | Integer | 初期選択インデックス (0始まり) |

### 4.7 `props` 定義 — Slider

```json
{ "min": 0, "max": 100, "value": 50, "step": 1 }
```

| キー | 型 | 説明 |
|------|------|------|
| `min` | Number | 最小値 |
| `max` | Number | 最大値 |
| `value` | Number | 初期値 |
| `step` | Number | ステップ幅 |

### 4.8 `props` 定義 — Spin

```json
{ "min": 0, "max": 100, "value": 0, "step": 1 }
```

| キー | 型 | 説明 |
|------|------|------|
| `min` | Number | 最小値 |
| `max` | Number | 最大値 |
| `value` | Number | 初期値 |
| `step` | Number | ステップ幅 |

### 4.9 `props` 定義 — Image

```json
{ "file_path": "/path/to/image.png", "alt_text": "Image" }
```

| キー | 型 | 説明 |
|------|------|------|
| `file_path` | String | 画像ファイルパス。空文字の場合は `alt_text` をフォールバック表示 |
| `alt_text` | String | 代替テキスト |

### 4.10 `props` 定義 — Progress

```json
{ "value": 0.5, "show_text": false }
```

| キー | 型 | 説明 |
|------|------|------|
| `value` | Number | 進捗値。範囲 `0.0` 〜 `1.0`。Qt では `value * 100` で整数パーセントに変換 |
| `show_text` | Boolean | パーセンテージテキストの表示有無 |

### 4.11 `props` 定義 — Separator

```json
{ "orientation": "horizontal" }
```

| キー | 型 | 値 | 説明 |
|------|------|------|------|
| `orientation` | String | `"horizontal"` \| `"vertical"` | 線の方向 |

---

## 5. `type` 一覧 — 図形

図形はランタイム側で **カスタム描画** (GTK: Cairo / Qt: QPainter) によりレンダリングする。

| type | 説明 | GTK (Cairo) | Qt (QPainter) |
|------|------|-------------|---------------|
| `Line` | 直線 | `cairo_move_to` + `cairo_line_to` | `drawLine` |
| `Rect` | 矩形 | `cairo_rectangle` | `drawRect` / `drawRoundedRect` |
| `Ellipse` | 楕円 / 円 | `cairo_arc` + `cairo_scale` | `drawEllipse` |
| `Triangle` | 三角形 | `cairo_line_to` (3点) | `drawPolygon` (3点) |
| `Diamond` | ひし形 | `cairo_line_to` (4点) | `drawPolygon` (4点) |
| `Arrow` | 矢印線 | Line + 三角マーカー | `drawLine` + `drawPolygon` |
| `Star` | 星形 | `cairo_line_to` (頂点リスト) | `drawPolygon` |

### 5.1 `props` 定義 — Line

```json
{ "stroke_color": "#ECEFF4", "stroke_width": 2, "direction": "horizontal" }
```

| キー | 型 | 説明 |
|------|------|------|
| `stroke_color` | String | 線の色 (`#RRGGBB`) |
| `stroke_width` | Number | 線の太さ (px) |
| `direction` | String | `"horizontal"` \| `"vertical"` \| `"diagonal-se"` \| `"diagonal-ne"` |

**始点・終点の計算** (`geometry` の `x, y, width(w), height(h)` から):

| direction | 始点 (x1, y1) | 終点 (x2, y2) |
|-----------|---------------|---------------|
| `horizontal` | `(x, y+h/2)` | `(x+w, y+h/2)` |
| `vertical` | `(x+w/2, y)` | `(x+w/2, y+h)` |
| `diagonal-se` | `(x, y)` | `(x+w, y+h)` |
| `diagonal-ne` | `(x, y+h)` | `(x+w, y)` |

### 5.2 `props` 定義 — Rect

```json
{ "fill_color": "transparent", "stroke_color": "#ECEFF4", "stroke_width": 2, "border_radius": 0 }
```

| キー | 型 | 説明 |
|------|------|------|
| `fill_color` | String | 塗り色。`#RRGGBB` または `"transparent"` (塗りなし) |
| `stroke_color` | String | 枠線色 (`#RRGGBB`) |
| `stroke_width` | Number | 枠線の太さ (px)。`0` で枠線なし |
| `border_radius` | Number | 角丸の半径 (px)。`0` で直角 |

### 5.3 `props` 定義 — Ellipse

```json
{ "fill_color": "transparent", "stroke_color": "#ECEFF4", "stroke_width": 2 }
```

| キー | 型 | 説明 |
|------|------|------|
| `fill_color` | String | 塗り色 |
| `stroke_color` | String | 枠線色 |
| `stroke_width` | Number | 枠線の太さ (px) |

**描画座標**: `geometry` の矩形に内接する楕円。

```
cx = x + w/2,  cy = y + h/2
rx = w/2,      ry = h/2
```

### 5.4 `props` 定義 — Triangle

```json
{ "fill_color": "transparent", "stroke_color": "#ECEFF4", "stroke_width": 2, "direction": "up" }
```

| キー | 型 | 説明 |
|------|------|------|
| `fill_color` | String | 塗り色 |
| `stroke_color` | String | 枠線色 |
| `stroke_width` | Number | 枠線の太さ (px) |
| `direction` | String | 頂点の方向。`"up"` \| `"down"` \| `"left"` \| `"right"` |

**頂点座標** (`geometry` 内の相対座標):

| direction | 頂点 1 | 頂点 2 | 頂点 3 |
|-----------|--------|--------|--------|
| `up` | `(w/2, 0)` | `(w, h)` | `(0, h)` |
| `down` | `(0, 0)` | `(w, 0)` | `(w/2, h)` |
| `left` | `(w, 0)` | `(w, h)` | `(0, h/2)` |
| `right` | `(0, 0)` | `(w, h/2)` | `(0, h)` |

### 5.5 `props` 定義 — Diamond

```json
{ "fill_color": "transparent", "stroke_color": "#ECEFF4", "stroke_width": 2 }
```

| キー | 型 | 説明 |
|------|------|------|
| `fill_color` | String | 塗り色 |
| `stroke_color` | String | 枠線色 |
| `stroke_width` | Number | 枠線の太さ (px) |

**頂点座標** (`geometry` 内の相対座標):

```
(w/2, 0)  →  (w, h/2)  →  (w/2, h)  →  (0, h/2)
```

### 5.6 `props` 定義 — Arrow

```json
{ "stroke_color": "#ECEFF4", "stroke_width": 2, "direction": "right" }
```

| キー | 型 | 説明 |
|------|------|------|
| `stroke_color` | String | 線と矢じりの色 (`#RRGGBB`) |
| `stroke_width` | Number | 線の太さ (px) |
| `direction` | String | 矢印の方向。`"right"` \| `"left"` \| `"up"` \| `"down"` |

**描画**: Line と同じ始点・終点計算に加え、終点に三角形の矢じりマーカーを付加する。
矢じりの向きは `direction` に従う。

| direction | 線の始点 | 線の終点 (矢じり位置) |
|-----------|---------|---------------------|
| `right` | `(x, y+h/2)` | `(x+w, y+h/2)` |
| `left` | `(x+w, y+h/2)` | `(x, y+h/2)` |
| `up` | `(x+w/2, y+h)` | `(x+w/2, y)` |
| `down` | `(x+w/2, y)` | `(x+w/2, y+h)` |

### 5.7 `props` 定義 — Star

```json
{ "fill_color": "transparent", "stroke_color": "#ECEFF4", "stroke_width": 2, "points": 5 }
```

| キー | 型 | 説明 |
|------|------|------|
| `fill_color` | String | 塗り色 |
| `stroke_color` | String | 枠線色 |
| `stroke_width` | Number | 枠線の太さ (px) |
| `points` | Integer | 頂点数 (3 〜 20) |

**頂点計算アルゴリズム**:

```
cx = x + w/2
cy = y + h/2
R  = min(w, h) / 2        // 外周半径
r  = R * 0.4              // 内周半径

for i in 0 .. (points * 2 - 1):
    angle  = PI * i / points - PI / 2
    radius = R  if (i % 2 == 0)  else r
    vx = cx + radius * cos(angle)
    vy = cy + radius * sin(angle)
```

`points * 2` 個の頂点を外周・内周交互に配置し、多角形として描画する。

---

## 6. `events` — シグナルマッピング

ウィジェットのユーザー操作とアプリケーション側ハンドラ名を紐付ける。
図形タイプでは `events` は空オブジェクト `{}` となる。

```json
"events": {
  "clicked": "on_button_submit"
}
```

- **キー**: シグナル名 (下表参照)
- **値**: アプリケーション側のコールバック / ハンドラ名
- 値が空文字 `""` の場合はハンドラ未接続を意味する

### 6.1 シグナル一覧

| type | キー | 発火条件 | GTK シグナル | Qt シグナル |
|------|------|---------|-------------|------------|
| `Button` | `clicked` | ボタン押下時 | `"clicked"` | `QPushButton::clicked` |
| `Checkbox` | `toggled` | チェック状態変化時 | `"toggled"` | `QCheckBox::stateChanged` |
| `Switch` | `toggled` | ON/OFF 切替時 | `"state-set"` | カスタム |
| `Combo` | `changed` | 選択変更時 | `"changed"` | `QComboBox::currentIndexChanged` |
| `Slider` | `value_changed` | 値変更時 | `"value-changed"` | `QSlider::valueChanged` |
| `Spin` | `value_changed` | 値変更時 | `"value-changed"` | `QSpinBox::valueChanged` |
| `Entry` | `activate` | Enter キー押下時 | `"activate"` | `QLineEdit::returnPressed` |

イベントを持たないタイプ (`Label`, `Image`, `Progress`, `Separator`, 全図形) では `events` は `{}` となる。

---

## 7. 色フォーマット

JSON 内の全色値は以下のいずれか:

| 形式 | 例 | 説明 |
|------|------|------|
| `#RRGGBB` | `"#2E3440"` | 6桁 hex。各チャネル 00-FF |
| `"transparent"` | `"transparent"` | 透明 (塗りなし / 背景なし) |

**パース例 (C)**:

```c
int parse_color(const char *hex, double *r, double *g, double *b) {
    if (strcmp(hex, "transparent") == 0) return 0; // 透明
    unsigned int val;
    sscanf(hex + 1, "%06x", &val);
    *r = ((val >> 16) & 0xFF) / 255.0;
    *g = ((val >> 8)  & 0xFF) / 255.0;
    *b = ( val        & 0xFF) / 255.0;
    return 1;
}
```

**パース例 (C++ / Qt)**:

```cpp
QColor parseColor(const QString &s) {
    if (s == "transparent") return Qt::transparent;
    return QColor(s);  // QColor は "#RRGGBB" をそのまま受け付ける
}
```

---

## 8. 完全な出力例

```json
{
  "window": {
    "title": "My Application",
    "width": 1920,
    "height": 1080,
    "background_color": "#2E3440"
  },
  "widgets": [
    {
      "id": "rect_1",
      "type": "Rect",
      "geometry": { "x": 30, "y": 20, "width": 400, "height": 300 },
      "style": { "background_color": "transparent", "color": "transparent" },
      "props": { "fill_color": "#3B4252", "stroke_color": "#4C566A", "stroke_width": 1, "border_radius": 12 },
      "events": {}
    },
    {
      "id": "label_2",
      "type": "Label",
      "geometry": { "x": 50, "y": 30, "width": 200, "height": 30 },
      "style": { "background_color": "transparent", "color": "#ECEFF4" },
      "props": { "label": "Settings", "font_size": 20 },
      "events": {}
    },
    {
      "id": "entry_3",
      "type": "Entry",
      "geometry": { "x": 50, "y": 80, "width": 300, "height": 32 },
      "style": { "background_color": "#434C5E", "color": "#ECEFF4" },
      "props": { "placeholder": "Username", "text": "" },
      "events": { "activate": "on_username_submit" }
    },
    {
      "id": "slider_4",
      "type": "Slider",
      "geometry": { "x": 50, "y": 130, "width": 300, "height": 28 },
      "style": { "background_color": "#ECEFF4", "color": "#2E3440" },
      "props": { "min": 0, "max": 100, "value": 75, "step": 1 },
      "events": { "value_changed": "on_brightness_change" }
    },
    {
      "id": "checkbox_5",
      "type": "Checkbox",
      "geometry": { "x": 50, "y": 180, "width": 200, "height": 28 },
      "style": { "background_color": "transparent", "color": "#ECEFF4" },
      "props": { "label": "Enable notifications", "checked": true },
      "events": { "toggled": "on_notify_toggle" }
    },
    {
      "id": "combo_6",
      "type": "Combo",
      "geometry": { "x": 50, "y": 220, "width": 200, "height": 32 },
      "style": { "background_color": "#434C5E", "color": "#ECEFF4" },
      "props": { "items": "English,Japanese,Chinese", "active_index": 1 },
      "events": { "changed": "on_language_change" }
    },
    {
      "id": "button_7",
      "type": "Button",
      "geometry": { "x": 50, "y": 275, "width": 120, "height": 36 },
      "style": { "background_color": "#88C0D0", "color": "#2E3440" },
      "props": { "label": "Save", "icon_name": "" },
      "events": { "clicked": "on_save" }
    },
    {
      "id": "button_8",
      "type": "Button",
      "geometry": { "x": 185, "y": 275, "width": 120, "height": 36 },
      "style": { "background_color": "#BF616A", "color": "#ECEFF4" },
      "props": { "label": "Cancel", "icon_name": "" },
      "events": { "clicked": "on_cancel" }
    },
    {
      "id": "separator_9",
      "type": "Separator",
      "geometry": { "x": 450, "y": 20, "width": 2, "height": 300 },
      "style": { "background_color": "#ECEFF4", "color": "#2E3440" },
      "props": { "orientation": "vertical" },
      "events": {}
    },
    {
      "id": "star_10",
      "type": "Star",
      "geometry": { "x": 500, "y": 50, "width": 80, "height": 80 },
      "style": { "background_color": "transparent", "color": "transparent" },
      "props": { "fill_color": "#EBCB8B", "stroke_color": "#D08770", "stroke_width": 1, "points": 5 },
      "events": {}
    },
    {
      "id": "arrow_11",
      "type": "Arrow",
      "geometry": { "x": 500, "y": 160, "width": 150, "height": 2 },
      "style": { "background_color": "transparent", "color": "transparent" },
      "props": { "stroke_color": "#A3BE8C", "stroke_width": 2, "direction": "right" },
      "events": {}
    },
    {
      "id": "progress_12",
      "type": "Progress",
      "geometry": { "x": 500, "y": 200, "width": 200, "height": 20 },
      "style": { "background_color": "#ECEFF4", "color": "#2E3440" },
      "props": { "value": 0.72, "show_text": true },
      "events": {}
    }
  ]
}
```

---

## 9. ランタイム実装手順 (概要)

1. JSON ファイルを読み込み、パースする。
2. `window` オブジェクトからウィンドウを生成し、タイトル・サイズ・背景色を設定する。
3. `widgets` 配列を **インデックス順** (0, 1, 2, ...) にイテレートする。
4. 各要素の `type` で分岐:
   - **ウィジェット** (Button 〜 Separator): 対応するネイティブウィジェットを生成 → `geometry` で配置 → `style` で外観設定 → `props` で固有設定 → `events` でシグナル接続。
   - **図形** (Line 〜 Star): `geometry` + `props` を用いてカスタム描画レイヤー上にレンダリングする。
5. 配列順 = Z-order。後の要素が前の要素の上に重なる。
