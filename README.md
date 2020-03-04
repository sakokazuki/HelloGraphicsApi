# Hello Graphics API
グラフィックス関連のプログラミングの勉強用リポジトリ  
複数のAPIに入門することで理解が深まるのではないかという仮定でとりあえずテクスチャ貼るくらいまでをやる

## Setup
Visual Studio 2017/Cmake, nugetが必要/x64 only/
1. clone repogitory
1. execute build.bat
1. move build/
1. open HelloGraphicsApi.sln
1. nugetでライブラリインストール
    - ツール/Nugetパッケージマネージャー/管理
    - 参照
    - "glfw"をVulkan_Projectにインストール
    - "glm"をVulkan_Projectにインストール
1. 任意のプロジェクトをスタートアップに設定
1. 実行

## Contents
- DirectX12
- Vulkan (wip)
- OpenGL (will)
- DirectX11 (will)

## 参考
参考というかDirectX12とvulkanは本を読みながら進めたのでコード部分は以下のリポジトリと99%一緒  
- https://github.com/techlabxe/d3d12_book_1
- https://github.com/techlabxe/vulkan_book_1