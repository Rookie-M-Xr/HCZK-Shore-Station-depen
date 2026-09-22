#pragma once

#include "map/item/MapRubberItem.h"

/** @brief 电子方位线对象，保存控制点并绘制距离圆和方位线。 */
class ENCVIEWER_SDK_EXPORT EblItem final : public MapRubberItem
{
public:
    /** @brief 更新圆心 WGS84 经纬度，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setCenter(QPointF center);
    /** @brief 更新半径控制点 WGS84 经纬度，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setRadiusPoint(QPointF radiusPoint);

    /** @brief 构造电子方位线对象。 */
    EblItem(const QPointF &center,
            const QPointF &radiusPoint,
            bool visible = true);

    /** @brief 返回内置电子方位线对象类型。 */
    MapItemType type() const noexcept override;

    /** @brief 返回距离圆圆心的 WGS84 经纬度。 */
    QPointF center() const noexcept;

    /** @brief 返回确定半径和方向的 WGS84 经纬度控制点。 */
    QPointF radiusPoint() const noexcept;

    /** @brief 返回圆心到控制点的距离，单位为米。 */
    double rangeMeters() const;

    /** @brief 返回圆心到控制点的真方位角。 */
    double bearingDegrees() const;

    /** @brief 返回电子方位线关键点的 WGS84 经纬度范围。 */
    QRectF geographicBounds() const override;

    /** @brief 返回删除按钮跟随的半径控制点。 */
    QPointF deleteAnchor() const override;

    /** @brief 返回电子方位线绘制顺序。 */
    int drawOrder() const noexcept override;

    /** @brief 绘制距离圆、半径线、控制点和距离方位文字。 */
    void draw(MapRenderContext &context) const override;

    /** @brief 判断屏幕点是否命中控制点、半径线或距离圆。 */
    bool containsScreenPoint(const QPointF &screenPoint,
                 const MapRenderContext &context) const override;

private:
    QPointF m_center;      ///< 用户指定的距离圆圆心。
    QPointF m_radiusPoint; ///< 用户指定的半径和方向控制点。
};
