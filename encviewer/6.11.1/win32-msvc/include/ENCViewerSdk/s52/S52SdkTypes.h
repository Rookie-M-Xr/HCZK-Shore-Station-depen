#pragma once

#include <QtGlobal>

/**
 * @brief S-52 显示类别。
 */
enum class S52DisplayCategory
{
    Base,     ///< 基础显示类别。
    Standard, ///< 标准显示类别。
    All,      ///< 全部显示类别。
    Custom    ///< 自定义显示类别。
};

/**
 * @brief S-52 点符号风格。
 */
enum class SymbolStyle
{
    Simplified, ///< 简化符号。
    Paper       ///< 纸海图符号。
};

/**
 * @brief S-52 区域边界风格。
 */
enum class BoundaryStyle
{
    Plain,     ///< 普通边界。
    Symbolized ///< 符号化边界。
};

/**
 * @brief ENC 对象名称标注字段。
 */
enum class NameDisplayMode
{
    National,    ///< 本国语言名称。
    International ///< 国际交换名称。
};

/**
 * @brief S-52 色表模式。
 */
enum class S52ColorTableMode : quint8
{
    DayBright,    ///< 日间明亮色表。
    DayBlackback, ///< 日间黑底色表。
    DayWhiteback, ///< 日间白底色表。
    Dusk,         ///< 黄昏色表。
    Night         ///< 夜间色表。
};
