#pragma once

#include "target/TargetDataTypes.h"

/**
 * @brief AIS 目标数据。
 * @note 纯值类型；目标 MMSI 位于 staticInfo.mmsi，来源船 MMSI 为 ownShipMMSI。
 * 本类不做目标融合、风险计算、数据有效性判断或物标更新。
 */

class AisData
{
public:
    /** @brief AIS 航行状态，取值对应 AIS message 1/2/3 的 Navigation Status。 */
enum class AisNavStatus : uint8_t
{
    UnderWayUsingEngine       = 0,  ///< 使用发动机航行。
    AtAnchor                  = 1,  ///< 锚泊。
    NotUnderCommand           = 2,  ///< 失控。
    RestrictedManoeuvrability = 3,  ///< 操纵受限。
    ConstrainedByDraught      = 4,  ///< 受吃水限制。
    Moored                    = 5,  ///< 系泊。
    Aground                   = 6,  ///< 搁浅。
    EngagedInFishing          = 7,  ///< 从事捕捞。
    UnderWaySailing           = 8,  ///< 帆行。
    ReservedHSC               = 9,  ///< 高速船保留值。
    ReservedWIG               = 10, ///< 地效翼船保留值。
    TowingAstern              = 11, ///< 拖带作业。
    PushingOrTowingAlongside  = 12, ///< 顶推或并靠拖带。
    Reserved                  = 13, ///< 保留值。
    AisSartActive             = 14, ///< AIS-SART 激活。
    Undefined                 = 15  ///< 未定义或不可用。
};
    DynamicInfoData dynamicInfo; ///< 动态信息。
    StaticInfoData staticInfo; ///< 静态信息。
    quint32 ownShipMMSI = 0; ///< 所属／发现船舶 MMSI，不等同于目标 MMSI。
    AisNavStatus navStatus = AisNavStatus::Undefined; ///< 航行状态；默认表示未定义或未收到。
    float dangerValue = 0.0f; ///< 危险度，通常为 0～1，数值越大风险越高。
    float dcpa = 0.0f; ///< 最近会遇距离，海里。
    float tcpa = 0.0f; ///< 最近会遇时间，分钟。
};
