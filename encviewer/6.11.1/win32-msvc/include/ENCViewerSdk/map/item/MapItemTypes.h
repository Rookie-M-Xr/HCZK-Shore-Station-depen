#pragma once

#include <QtGlobal>

#include "sdk/EncViewerSdkGlobal.h"

#include <QDateTime>
#include <QPointF>

/** @brief 地图对象全局唯一标识。 */
using MapItemId = quint64;

/** @brief 地图对象 ID 常量。 */
namespace MapItemIds
{
/** @brief 无效地图对象 ID。 */
inline constexpr MapItemId Invalid = 0;
}

/** @brief 地图对象运行时类型。 */
using MapItemType = qint32;

/** @brief 地图对象类型常量。 */
namespace MapItemTypes
{
/** @brief 无效地图对象类型。 */
inline constexpr MapItemType Invalid = -1;
/** @brief 内置地图对象类型起始值。 */
inline constexpr MapItemType Builtin = 0;
/** @brief 内置航线对象。 */
inline constexpr MapItemType Route = Builtin;
/** @brief 内置测量对象。 */
inline constexpr MapItemType Measure = Builtin + 1;
/** @brief 内置电子方位线对象。 */
inline constexpr MapItemType Ebl = Builtin + 2;
/** @brief AIS 目标物标。 */
inline constexpr MapItemType Ais = Builtin + 3;
/** @brief ARPA 雷达目标物标。 */
inline constexpr MapItemType Arpa = Builtin + 4;
/** @brief 小艇、中型船和大船物标。 */
inline constexpr MapItemType Ship = Builtin + 5;
/** @brief 本船物标，保证绘制在其他船舶目标上层。 */
inline constexpr MapItemType OwnShip = Builtin + 6;
/** @brief 动态轨迹物标。 */
inline constexpr MapItemType Track = Builtin + 7;
/** @brief 依附船舶的航向航程矢量线。 */
inline constexpr MapItemType ShipRangeVector = Builtin + 8;
/** @brief 依附船舶的艏向／迹向扇形。 */
inline constexpr MapItemType ShipSector = Builtin + 9;
/** @brief 自定义地图对象类型起始值，自定义对象使用 Custom + n。 */
inline constexpr MapItemType Custom = 10000;
}

/** @brief AIS/ARPA 目标风险等级。 */
enum class MapTargetRiskLevel : quint8
{
    Low,
    Medium,
    High
};

/** @brief 目标在线状态。 */
enum class ShipOnlineState : quint8
{
    Offline,
    Online
};

/** @brief 目标控制状态。 */
enum class ShipControlState : quint8
{
    Uncontrolled,
    Controlled
};

/** @brief 目标的基础运动数据，角度均为真方位。 */
struct ENCVIEWER_SDK_EXPORT MapTargetMotion
{
    QPointF position;                 ///< WGS84 经纬度。
    double headingDegrees = -1.0;      ///< 真艏向；负值表示无效，此时使用 COG。
    double cogDegrees = -1.0;          ///< 对地航向；负值表示无效。
    double sogKnots = 0.0;             ///< 对地航速，单位节。
    QDateTime updateTimeUtc;           ///< 数据更新时间。
};
