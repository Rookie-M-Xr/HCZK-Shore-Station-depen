#pragma once

#include "map/item/MapDynamicItem.h"

#include <QDateTime>
#include <QPointF>
#include <QVector>
#include <QPointer>

/** @brief 轨迹中的一个带时间位置点。 */
struct ENCVIEWER_SDK_EXPORT TrackPoint
{
    QPointF position; ///< WGS84 经纬度，x 为经度，y 为纬度。
    QDateTime timestampUtc; ///< 位置采样时间，使用 UTC。
    bool startsNewSegment = false; ///< true 表示本点开始新段，不与前一点连线。
};

/** @brief 动态轨迹地图对象，保存并绘制按时间排列的轨迹点。 */
class ENCVIEWER_SDK_EXPORT TrackItem final : public MapDynamicItem
{
public:
    /** @brief 返回借用的所属物标；独立轨迹或物标销毁后返回 nullptr，不延长物标生命周期。 */
    MapItem *target() const noexcept;
    /** @brief 追加一个 UTC 轨迹点，不自动排序；调用方保证时间不早于末点。 */
    void appendPoint(TrackPoint point);
    /** @brief 更新完整轨迹点序列，调用方保证时间升序，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setPoints(QVector<TrackPoint> points);

    /** @brief 构造轨迹对象；轨迹点应按时间升序排列。 */
    TrackItem(QVector<TrackPoint> points, bool visible = true);

    /** @brief 返回内置轨迹对象类型。 */
    MapItemType type() const noexcept override;
    /** @brief 返回全部轨迹点。 */
    const QVector<TrackPoint> &points() const noexcept;
    /** @brief 返回轨迹 WGS84 经纬度范围。 */
    QRectF geographicBounds() const override;
    /** @brief 返回轨迹绘制顺序。 */
    int drawOrder() const noexcept override;
    /** @brief 绘制轨迹折线。 */
    void draw(MapRenderContext &context) const override;
    /** @brief 判断屏幕点是否命中轨迹点或线段。 */
    bool containsScreenPoint(const QPointF &screenPoint,
                 const MapRenderContext &context) const override;

private:
    friend class ShipItem;
    friend class AisItem;
    friend class ArpaItem;
    void bindTarget(MapItem *target, QString styleKey);
    void recordMotion(const MapTargetMotion &motion);
    bool displayEnabled() const;
    QPointer<MapItem> m_target;
    bool m_attached = false;
    bool m_recordOnlyWhenVisible = false;
    bool m_pendingSegment = false;
    quint64 m_showRevision = 0;
    QString m_styleKey = QStringLiteral("item.track");
    QVector<TrackPoint> m_points; ///< 按时间升序排列的轨迹点。
};
