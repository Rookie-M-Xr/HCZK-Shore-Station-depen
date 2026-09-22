#pragma once

#include "target/TargetDataTypes.h"
#include <optional>

/**
 * @brief 船舶基础数据。
 * @note 仅保留动态、静态、在线和基础状态。纯值类型，可复制、移动；
 * 不含设备指针、控制逻辑、文件、轨迹、数据管理器依赖或界面信号。
 */
class ShipData
{
public:
    /** @brief 船舶基础运行状态。 */
    enum class Status : int
    {
        Unknown = -1, ///< 未报告。
        Wait = 0, ///< 待机。
        Straight = 1, ///< 直航。
        TurningLeft = 2, ///< 左转。
        TurningRight = 3, ///< 右转。
        Error = 4, ///< 故障。
        Stop = 5, ///< 紧急停车。
        FixedPosition = 6, ///< 定点位控。
        Avoid = 7 ///< 避碰。
    };

    DynamicInfoData dynamicInfo; ///< 动态信息。
    StaticInfoData staticInfo; ///< 静态信息，包含 MMSI。
    bool online = false; ///< 在线状态快照；本类不依赖通信设备或推算超时。
    Status status = Status::Unknown; ///< 船舶基础运行状态；默认表示未报告。
    std::optional<quint8> gnssStatus; ///< GNSS 状态码；空表示未报告。
};
