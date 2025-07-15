#pragma once

class NonCopyable {
public:
    NonCopyable() = default; // デフォルトコンストラクタ

    // コピーコンストラクタを削除
    NonCopyable(const NonCopyable&) = delete;

    // 代入演算子も削除（コピーを完全に禁止する場合）
    NonCopyable& operator=(const NonCopyable&) = delete;
};