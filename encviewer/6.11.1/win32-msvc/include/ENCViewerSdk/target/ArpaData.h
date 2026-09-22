#pragma once

#include <QString>
#include <QtGlobal>

/**
 * @brief ARPA 目标数据。
 * @note 只保存数据，不持有物标、轨迹或 QObject；可复制、移动。
 * 保留单位码和输入数值，不在数据类中转换相对／真方位或距离单位。
 * 默认零值不表示观测有效；有效性规则待接入设计。
 */
class ArpaData
{
public:
    quint32 ownShipMMSI = 0; ///< 所属／发现船舶 MMSI。
    quint32 id = 0; ///< 雷达目标编号；不是 MapItemId。
    QString name; ///< 目标名称。
    char bearingTR = 'R'; ///< 方位参考，T 表示真方位，R 表示相对方位。
    char courseTR = 'R'; ///< 航向参考，T 表示真航向，R 表示相对航向。
    char units = 'N'; ///< 速度和距离单位码：K、N 或 S。
    char status = 'Q'; ///< 目标状态：L 丢失、Q 待确认、T 跟踪。
    char acqType = 'M'; ///< 捕获类型：A 自动、M 手动、R 通告。
    float dis = 0.0f; ///< 目标距离；单位由 units 约定。
    float bearing = 0.0f; ///< 方位角，度；相对／真方位由 bearingTR 指定。
    float spd = 0.0f; ///< 目标速度；单位由 units 约定。
    float course = 0.0f; ///< 航向角，度；相对／真航向由 courseTR 指定。
    float dcpa = 0.0f; ///< 最近会遇距离；单位由接入协议约定。
    float tcpa = 0.0f; ///< 最近会遇时间，分钟。
    qint64 time = 0; ///< 数据时间，UTC 秒。
    double lng = 0.0; ///< 目标经度，度；不在本类中由距离／方位推算。
    double lat = 0.0; ///< 目标纬度，度。
    float dangerValue = 0.0f; ///< 危险度，通常为 0～1，数值越大风险越高。
};
