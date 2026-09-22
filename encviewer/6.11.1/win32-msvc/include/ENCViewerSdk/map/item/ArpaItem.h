#pragma once

#include "map/item/MapDynamicItem.h"

#include <QString>

class TrackItem;

/** @brief ARPA 雷达目标，保存航迹号及关联 MMSI。 */
class ENCVIEWER_SDK_EXPORT ArpaItem final : public MapDynamicItem
{
public:
    /** @brief 返回随物标注册的独立轨迹；可通过 setVisible 单独关闭，仅轨迹自身及分类显示开关均开启时自动记录；重新开启后另起一段。 */
    QSharedPointer<TrackItem> trackItem() const noexcept;
    /** @brief 更新关联 AIS MMSI，0 表示未关联，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setAssociatedMmsi(quint32 associatedMmsi);
    /** @brief 更新运动数据，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setMotion(MapTargetMotion motion);
    /** @brief 更新风险等级，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setRiskLevel(MapTargetRiskLevel risk);
    /** @brief 更新目标名称，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setName(QString name);

    /** @brief 构造 ARPA 目标。associatedMmsi 为 0 表示尚未关联 AIS。 */
    ArpaItem(quint32 trackNumber,
             quint32 observerMmsi,
             quint32 associatedMmsi,
             MapTargetMotion motion,
             MapTargetRiskLevel risk = MapTargetRiskLevel::Low,
             QString name = {},
             bool visible = true);
    /** @brief 返回 ARPA 物标类型。 */ MapItemType type() const noexcept override;
    /** @brief 返回目标地理范围。 */ QRectF geographicBounds() const override;
    /** @brief 返回雷达航迹号。 */ quint32 trackNumber() const noexcept;
    /** @brief 返回发现者 MMSI。 */ quint32 observerMmsi() const noexcept;
    /** @brief 返回关联 AIS MMSI。 */ quint32 associatedMmsi() const noexcept;
    /** @brief 返回位置、航向和航速。 */ const MapTargetMotion &motion() const noexcept;
    /** @brief 返回风险等级。 */ MapTargetRiskLevel riskLevel() const noexcept;
    /** @brief 返回目标名称。 */ const QString &name() const noexcept;

    /** @brief 返回 ARPA 目标绘制顺序。 */ int drawOrder() const noexcept override;
    /** @brief 绘制 ARPA 符号、航向速度矢量和文字。 */
    void draw(MapRenderContext &context) const override;
    /** @brief 判断屏幕点是否命中 ARPA 符号或速度矢量。 */
    bool containsScreenPoint(const QPointF &screenPoint,
                 const MapRenderContext &context) const override;

private:
    QSharedPointer<TrackItem> m_trackItem; ///< 自动附带的轨迹。
    quint32 m_trackNumber = 0;
    quint32 m_observerMmsi = 0;
    quint32 m_associatedMmsi = 0;
    MapTargetMotion m_motion;
    MapTargetRiskLevel m_risk = MapTargetRiskLevel::Low;
    QString m_name;
};
