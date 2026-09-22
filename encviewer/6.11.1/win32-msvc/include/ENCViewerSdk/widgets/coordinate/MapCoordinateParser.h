#pragma once

#include "sdk/EncViewerSdkGlobal.h"

#include <QPointF>
#include <QString>

#include <optional>

/** @brief WGS84 坐标轴类型。 */
enum class MapCoordinateAxis
{
    Longitude,
    Latitude
};

/** @brief 解析 WGS84 十进制度、度分和度分秒文本。 */
class ENCVIEWER_SDK_EXPORT MapCoordinateParser final
{
public:
    MapCoordinateParser() = delete;
    /**
     * @brief 解析一个经度或纬度文本。
     * @param text 支持十进制度、度分、度分秒及末尾 N/E/S/W 后缀，不支持科学计数法。
     * 分量使用空格、冒号或度分秒符号分隔；仅最后一个分量允许小数，负号仅位于开头。
     * @param axis 坐标轴类型，用于范围和方向校验。
     * @param errorMessage 可选的中文错误信息输出，成功时清空。
     * @return 经度 [-180, 180] 或纬度 [-90, 90]；无效输入返回 nullopt。
     */
    static std::optional<double> parse(const QString &text,
                                       MapCoordinateAxis axis,
                                       QString *errorMessage = nullptr);

    /**
     * @brief 解析经度和纬度文本。
     * @param longitudeText 经度文本。
     * @param latitudeText 纬度文本。
     * @param errorMessage 可选的中文错误信息输出，成功时清空。
     * @return x 为经度、y 为纬度的 WGS84 坐标；任一坐标无效则返回 nullopt。
     */
    static std::optional<QPointF> parsePair(const QString &longitudeText,
                                            const QString &latitudeText,
                                            QString *errorMessage = nullptr);
};
