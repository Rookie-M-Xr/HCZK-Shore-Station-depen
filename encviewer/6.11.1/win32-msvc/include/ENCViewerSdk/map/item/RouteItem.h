#pragma once

#include "map/item/MapStaticItem.h"

#include <QColor>
#include <QPointF>
#include <QString>
#include <QVector>

/** @brief 单个航点的基础数据。 */
struct ENCVIEWER_SDK_EXPORT RouteWaypoint
{
    QPointF position; ///< WGS84 经纬度，x 为经度，y 为纬度。
    QString name;     ///< 可选航点名称。
};

/** @brief 航线地图对象，保存航线数据并绘制电子海图航线。 */
class ENCVIEWER_SDK_EXPORT RouteItem final : public MapStaticItem
{
public:
    /** @brief 更新航线名称，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setName(QString name);
    /** @brief 更新完整航点序列，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setWaypoints(QVector<RouteWaypoint> waypoints);

    /** @brief 构造航线对象。 */
    RouteItem(QString name,
              QVector<RouteWaypoint> waypoints,
              bool visible = true);

    /** @brief 返回内置航线对象类型。 */
    MapItemType type() const noexcept override;


    /** @brief 返回航线名称。 */
    QString name() const;

    /** @brief 返回全部航点。 */
    const QVector<RouteWaypoint> &waypoints() const;

    /** @brief 返回航线 WGS84 经纬度范围。 */
    QRectF geographicBounds() const override;


    /** @brief 返回航线绘制顺序。 */
    int drawOrder() const noexcept override;

    /** @brief 使用航线物标颜色绘制航线和带编号航点。 */
    void draw(MapRenderContext &context) const override;

    /** @brief 判断屏幕点是否命中航点或航段。 */
    bool containsScreenPoint(const QPointF &screenPoint,
                 const MapRenderContext &context) const override;

private:
    friend class CreateRouteTool;
    friend class EditRouteTool;
    /** @brief 供航线工具使用其独立配色绘制预览。 */
    void draw(MapRenderContext &context, const QColor &routeColor) const;

    QString m_name;                       ///< 航线名称。
    QVector<RouteWaypoint> m_waypoints;   ///< 按航行顺序排列的航点。
};
