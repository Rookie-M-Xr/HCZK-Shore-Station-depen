#pragma once

#include "map/item/MapRubberItem.h"

#include <QVector>

/** @brief 多点折线测量对象，保存测量点并绘制测量结果。 */
class ENCVIEWER_SDK_EXPORT MeasureItem final : public MapRubberItem
{
public:
    /** @brief 更新完整测量点序列，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setPoints(QVector<QPointF> points);

    /** @brief 构造测量对象。 */
    MeasureItem(QVector<QPointF> points, bool visible = true);

    /** @brief 返回内置测量对象类型。 */
    MapItemType type() const noexcept override;

    /** @brief 返回全部测量点的 WGS84 经纬度。 */
    const QVector<QPointF> &points() const noexcept;

    /** @brief 返回全部测量段的总距离，单位为米。 */
    double totalDistanceMeters() const;

    /** @brief 返回测量对象 WGS84 经纬度范围。 */
    QRectF geographicBounds() const override;

    /** @brief 返回删除按钮跟随的末测量点。 */
    QPointF deleteAnchor() const override;

    /** @brief 返回测量物标绘制顺序。 */
    int drawOrder() const noexcept override;

    /** @brief 绘制测量折线、节点、分段数据和总距离。 */
    void draw(MapRenderContext &context) const override;

    /** @brief 判断屏幕点是否命中测量节点或线段。 */
    bool containsScreenPoint(const QPointF &screenPoint,
                 const MapRenderContext &context) const override;

private:
    QVector<QPointF> m_points; ///< 按用户点击顺序排列的测量点。
};
