# PluginTest

NSysプラグインアーキテクチャのテスト用プロジェクトです。

## プロジェクト構成

```
PluginTest/
├── external/
│   └── imgui/          # ImGuiライブラリ（サブモジュール）
├── dllmain.cpp         # プラグインのメインエントリーポイント
├── framework.h         # 共通ヘッダーファイル
├── PluginTest.vcxproj  # Visual Studio プロジェクトファイル
└── README.md           # このファイル
```

## 機能

- ImGuiコンテキストを共有してメインアプリケーションと連携
- シンプルなウィンドウとコントロールの表示
- プラグインの表示/非表示切り替え機能

## プラグインインターフェース

### エクスポート関数

```cpp
extern "C" __declspec(dllexport) void run(ImGuiContext* shared_context, const void* inputs, void* outputs)
```

- `shared_context`: メインアプリケーションから共有されるImGuiコンテキスト
- `inputs`: 入力データ（現在は未使用）
- `outputs`: 出力データ（現在は未使用）

### 実装例

```cpp
extern "C" __declspec(dllexport) void run(ImGuiContext* shared_context, const void* inputs, void* outputs)
{
    // ImGuiのコンテキストをセット
    ImGui::SetCurrentContext(shared_context);

    // プラグインのUIを描画
    static bool show_window = true;
    static float slider_value = 0.0f;

    ImGui::Begin("DLL Window");
    ImGui::Text("Hello from DLL!");
    ImGui::SliderFloat("Slider", &slider_value, 0.0f, 1.0f);
    ImGui::Checkbox("Show Window", &show_window);
    ImGui::End();
}
```

## ビルド方法

1. Visual Studioで`PluginTest.vcxproj`を開く
2. 設定をRelease x64に変更
3. ビルドを実行
4. 生成されたDLLを`Plugins/PluginTest/`フォルダに配置

## プラグインの配置

メインアプリケーションは以下の構造でプラグインを探します：

```
NSys.exe
└── Plugins/
    └── PluginTest/
        └── PluginTest.dll
```

## 参考資料

- [ImGui Documentation](https://github.com/ocornut/imgui)
- [NSys メインプロジェクト](../NSys/README.md)
