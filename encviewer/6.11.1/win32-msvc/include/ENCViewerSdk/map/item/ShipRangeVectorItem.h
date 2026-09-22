#pragma once

#include "map/item/MapDynamicItem.h"
#include <QPointer>

class ShipItem;

/** @brief 依附船舶状态的航向航程矢量线，须与船舶在同一 GUI 线程使用。
 * @note 不持有船舶所有权；船舶隐藏或销毁后不绘制。ShipItem 自动附带并注册一条。 */
class ENCVIEWER_SDK_EXPORT ShipRangeVectorItem : public MapDynamicItem
{
public:
    /** @brief 绑定船舶；nullptr 表示暂不显示，预测时长跟随样式设置（初始 6 分钟），每分钟一个刻度。 */
    explicit ShipRangeVectorItem(ShipItem *ship = nullptr, bool visible = true);
    /** @brief 更换借用的船舶，自动断开旧绑定并通知重绘。 */
    void setShip(ShipItem *ship);
    /** @brief 返回借用的船舶，销毁后返回 nullptr。 */
    ShipItem *ship() const noexcept;
    /** @brief 设置单船预测秒数和刻度间隔秒数，不再跟随全局预测时长；均须为正且刻度不超过 3600 个。
     * @return 有效时更新并返回 true；无效时保持原设置并返回 false。 */
    bool setTimeParameters(int durationSeconds, int tickIntervalSeconds);
    /** @brief 返回预测总时长，单位秒。 */
    int durationSeconds() const noexcept;
    /** @brief 返回时间刻度间隔，单位秒。 */
    int tickIntervalSeconds() const noexcept { return m_tickIntervalSeconds; }
    /** @brief 返回航程矢量物标类型。 */
    MapItemType type() const noexcept override;
    /** @brief 返回起终点的 WGS84 包围范围；无有效运动数据时返回空范围。 */
    QRectF geographicBounds() const override;
    /** @brief 绘制在船舶符号下层。 */
    int drawOrder() const noexcept override;
    /** @brief 按真实预计距离绘制矢量与时间刻度，样式独立于船舶。 */
    void draw(MapRenderContext &context) const override;
    /** @brief 检查矢量线主体命中；隐藏或失去船舶时返回 false。 */
    bool containsScreenPoint(const QPointF &point, const MapRenderContext &context) const override;

private:
    bool motionParameters(QPointF &position, double &direction, double &metersPerSecond) const;
    QPointer<ShipItem> m_ship;
    QMetaObject::Connection m_changeConnection;
    QMetaObject::Connection m_destroyConnection;
    int m_durationSeconds = 0; ///< 0 表示跟随全局预测时长设置。
    int m_tickIntervalSeconds = 60;
};
