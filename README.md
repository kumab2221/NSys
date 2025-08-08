# NSys

NSysは、ImGuiを使用したC++アプリケーション開発プロジェクトです。

## プロジェクト構成

```
NSys/
├── NSys/                    # メインアプリケーション
│   ├── external/imgui/      # ImGuiライブラリ（サブモジュール）
│   ├── ImGuiWindowManager.cpp
│   ├── ImGuiWindowManager.h
│   └── main.cpp
├── PluginTest/              # プラグインテスト用プロジェクト
│   ├── external/imgui/      # ImGuiライブラリ（サブモジュール）
│   ├── dllmain.cpp
│   ├── framework.h
│   └── README.md
├── CSVNodeEditor/           # ノードプログラミング型CSV解析ツール
│   ├── external/            # 外部ライブラリ（サブモジュール）
│   │   ├── imgui/          # ImGuiライブラリ
│   │   ├── imnodes/        # ImNodesライブラリ
│   │   └── implot/         # ImPlotライブラリ
│   ├── CSVNodeEditor.h     # メインクラス
│   ├── CSVNodeEditor.cpp   # メイン実装
│   ├── NodeEditor.h        # ノードエディタ
│   ├── NodeEditor.cpp      # ノードエディタ実装
│   ├── CSVData.h           # CSVデータ処理
│   ├── CSVData.cpp         # CSVデータ処理実装
│   ├── NodeTypes.h         # ノードタイプ定義
│   ├── NodeTypes.cpp       # ノードタイプ実装
│   ├── dllmain.cpp         # DLLエントリーポイント
│   ├── framework.h         # 共通ヘッダー
│   └── README.md           # プラグイン説明
└── README.md
```

## 前提条件

- Visual Studio 2019以降
- Git
- CMake（必要に応じて）

## プラグイン

### PluginTest
基本的なプラグイン機能のテスト用プロジェクトです。ImGuiコンテキストを共有してメインアプリケーションと連携します。

### CSVNodeEditor
ノードプログラミング型CSV解析ツールです。以下の機能を提供します：

- **ノードベースのデータ処理フロー構築**
- **CSVファイルの読み込み・保存**
- **複数タブでの並列編集**
- **Dockingウィンドウ対応**
- **各種データ処理ノード**（フィルター、ソート、集計、結合等）

詳細は [CSVNodeEditor/README.md](CSVNodeEditor/README.md) を参照してください。

## セットアップ手順

### 1. リポジトリのクローン

```bash
git clone <repository-url>
cd NSys
```

### 2. サブモジュールの初期化

**重要**: このプロジェクトはImGuiライブラリをサブモジュールとして使用しています。以下の手順で正しく初期化してください。

#### 方法1: 推奨方法（新規クローン時）

```bash
# リポジトリをクローンする際にサブモジュールも同時に初期化
git clone --recursive <repository-url>
cd NSys
```

#### 方法2: 既存のリポジトリでサブモジュールを初期化

```bash
# サブモジュールの初期化と更新
git submodule update --init --recursive
```

#### 方法3: サブモジュールが正しく動作しない場合の対処法

もし上記の方法でサブモジュールが正しく初期化されない場合（空のディレクトリが作成される場合）：

```bash
# 1. 空のimguiディレクトリを削除
Remove-Item -Recurse -Force "NSys/external/imgui" -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force "PluginTest/external/imgui" -ErrorAction SilentlyContinue

# 2. サブモジュールを再追加
git submodule add -b docking https://github.com/ocornut/imgui.git NSys/external/imgui
git submodule add -b docking https://github.com/ocornut/imgui.git PluginTest/external/imgui

# 3. サブモジュールの状態を確認
git submodule status
```

### 3. 自動セットアップ（推奨）

プロジェクトのセットアップを自動化するスクリプトを提供しています：

#### Windows用バッチスクリプト
```cmd
setup.bat
```

#### PowerShellスクリプト（より詳細なログ出力）
```powershell
.\setup.ps1
```

これらのスクリプトは以下を自動的に実行します：
- Gitのインストール確認
- サブモジュールの初期化
- ImGuiファイルの存在確認
- 問題がある場合の自動修復

### 4. プロジェクトのビルド

1. `NSys.sln` をVisual Studioで開く
2. 必要な依存関係が解決されていることを確認
3. ビルド設定を選択（Debug/Release）
4. ビルドを実行

## サブモジュールの管理

### サブモジュールの更新

```bash
# すべてのサブモジュールを最新版に更新
git submodule update --remote --recursive

# 特定のサブモジュールのみ更新
git submodule update --remote NSys/external/imgui
```

### サブモジュールの状態確認

```bash
# サブモジュールの状態を表示
git submodule status

# サブモジュールの詳細情報を表示
git submodule foreach 'git status'
```

## トラブルシューティング

### サブモジュールが空の場合

1. `git submodule status` を実行してサブモジュールの状態を確認
2. 空のディレクトリが表示される場合は、上記の「方法3」を実行

### ビルドエラーが発生する場合

1. ImGuiファイルが正しく配置されているか確認
2. プロジェクトのインクルードパスが正しく設定されているか確認
3. 必要な依存関係がインストールされているか確認

## ライセンス

このプロジェクトは適切なライセンスの下で提供されています。詳細は `LICENSE.txt` を参照してください。

## 貢献

プロジェクトへの貢献を歓迎します。プルリクエストやイシューの報告をお気軽にお願いします。

## サポート

問題が発生した場合は、以下の手順で対処してください：

1. このREADMEのトラブルシューティングセクションを確認
2. 既存のイシューを検索
3. 新しいイシューを作成（詳細な情報を含めてください）