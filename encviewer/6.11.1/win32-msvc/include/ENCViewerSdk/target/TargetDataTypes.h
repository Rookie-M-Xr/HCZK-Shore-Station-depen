#pragma once

#include <QString>
#include <QtGlobal>


/**
 * @brief 船舶与 AIS 共用的动态数据首稿，按九自由度组织。
 * @note 纯值类型，不含 QObject、信号、绘制或记录逻辑。默认零值不代表有效观测，
 * 不代表已收到有效观测；数据有效性及协议无效值归一化规则待后续接入时确定。
 */



struct DynamicInfoData
{
    /** @brief WGS84 经度，单位度。 */
    double longitude = 0.0; ///< 经度，单位度。
    /** @brief WGS84 纬度，单位度。 */
    double latitude = 0.0; ///< 纬度，单位度。
    /** @brief 船舶高度，单位米。 */
    float altitude = 0.0; ///< 高度，单位米。

    float roll = 0.0; ///< 横滚角，单位度。
    float pitch = 0.0; ///< 俯仰角，单位度。
    float yaw = 0.0; ///< 艏向／偏航角，单位度。

    float rollRate = 0.0; ///< 横滚角速度，单位度／分钟。
    float pitchRate = 0.0; ///< 俯仰角速度，单位度／分钟。
    float yawRate = 0.0; ///< 偏航角速度，单位度／分钟。

    float cog = 0.0; ///< 对地航向，度；与姿态 yaw 独立。
    float ctw = 0.0; ///< 对水航向，度。
    float stw = 0.0; ///< 对水航速，节。
    float sog = 0.0; ///< 对地速度，单位节。
    float nmileWater = 0.0f; ///< 累计对水里程，海里；
    float nmileGround = 0.0f; ///< 累计对地里程，海里；
    qint64 time = 0; ///< 数据更新时间，UTC 秒。
};

/** @brief 船舶与 AIS 共用的静态数据。 */
struct StaticInfoData
{
    quint32 mmsi = 0; ///< 目标船舶 MMSI；0 表示未设置。
    QString name; ///< 船名；空字符串表示未设置。
    QString imo; ///< 船舶永久识别码；具体采用 IMO 号还是呼号由接入协议确定。

    float length = 0.0f; ///< 船长，米。
    float width = 0.0f; ///< 船宽，米。
    float height = 0.0f; ///< 主甲板到水面的高度，单位米。
    float draft = 0.0f; ///< 吃水，米。

    char aisType = 0; ///< AIS 船舶类型代码。
    QString destLocation; ///< 目的地。
    qint64 arriveTime = 0; ///< 预计到达时间；单位和时区由接入协议约定。
    float ridus = 500.0f; ///< 转弯半径，单位米。
    float routeWidth = 926.0f; ///< 标准航道宽度，单位米。
};
