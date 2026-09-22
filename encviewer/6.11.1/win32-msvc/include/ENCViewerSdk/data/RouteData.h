#pragma once

#include <QDateTime>
#include <QPointF>
#include <QString>
#include <QUuid>
#include <QVector>

/** @brief 持久化航点；ID 与顺序独立，编辑时保留已有 ID。 */
struct RouteWaypointData
{
    QUuid id; ///< 空 ID 在保存时分配。
    QPointF position; ///< WGS84 经纬度，x 经度、y 纬度，单位度。
    QString name; ///< 可选航点名称。
    double speedKnots = 0.0; ///< 从当前航点驶向下一个航点的航速，单位节；末航点值保留但通常不使用。
};

/** @brief 可按值编辑的航线副本；只有保存成功后才替换正式数据。 */
struct RouteData
{
    QUuid id; ///< 稳定业务 ID，不是 MapItemId；空值在首次保存时分配。
    QString name; ///< 航线名称。
    QVector<RouteWaypointData> waypoints; ///< 按航行顺序排列，保存至少需要两个航点。
    QDateTime createdAtUtc; ///< 创建时间，由管理器维护。
    QDateTime updatedAtUtc; ///< 最后保存时间，由管理器维护。
};
