#pragma once

#include "map/item/MapDynamicItem.h"
#include <QPointer>

class ShipItem;
class QPainterPath;

/** @brief 扇形中心线使用的船舶方向。 */
enum class ShipSectorDirection : quint8
{
    Heading,          ///< 真艏向。
    CourseOverGround  ///< 对地航向 COG。
};

/** @brief 依附船舶的地理扇形；与船舶在同一 GUI 线程使用。
 * @note 不持有船舶所有权；自身或船舶隐藏、船舶销毁、选定方向无效时不绘制且不命中。
 * ShipItem 自动附带并注册一个扇形，固定半径不依赖航速。 */
class ENCVIEWER_SDK_EXPORT ShipSectorItem : public MapDynamicItem
{
public:
    /** @brief 绑定船舶，nullptr 表示暂不显示；参数默认跟随扇形样式设置。 */
    explicit ShipSectorItem(ShipItem *ship = nullptr, bool visible = true);
    /** @brief 更换借用的船舶；断开旧连接并通知重绘，不接受其他线程的对象。 */
    void setShip(ShipItem *ship);
    /** @brief 返回借用的船舶，销毁后返回 nullptr。 */
    ShipItem *ship() const noexcept;
    /** @brief 设置单船扇形的固定半径和总张角，之后不再跟随全局尺寸。
     * @param radiusMeters 半径，单位米，范围 (0, 1000000]；海里值应乘以 1852。
     * @param openingAngleDegrees 总张角，范围 (0, 360]，中心线两侧各占一半。
     * @return 有限且在范围内时更新并返回 true；无效时保持原参数并返回 false。 */
    bool setRangeParameters(double radiusMeters, double openingAngleDegrees);
    /** @brief 设置单船中心线方向来源；非法枚举忽略，所选方向无效时隐藏而不回退。 */
    void setDirectionMode(ShipSectorDirection mode);
    /** @brief 清除单船参数覆盖，重新跟随全局半径、总张角和方向；不改变可见性。 */
    void resetParameters();
    /** @brief 返回有效半径，单位米。 */
    double radiusMeters() const noexcept;
    /** @brief 返回有效总张角，单位度。 */
    double openingAngleDegrees() const noexcept;
    /** @brief 返回有效方向来源。 */
    ShipSectorDirection directionMode() const noexcept;
    /** @brief 返回船舶扇形物标类型。 */
    MapItemType type() const noexcept override;
    /** @brief 返回包含扇形的保守 WGS84 范围；跨日期线时覆盖全部经度。 */
    QRectF geographicBounds() const override;
    /** @brief 扇形绘制在航程矢量线及船舶符号下层。 */
    int drawOrder() const noexcept override;
    /** @brief 按地理距离绘制扇形，轮廓和填充使用当前白／昏／夜配色。 */
    void draw(MapRenderContext &context) const override;
    /** @brief 检查扇形内部及边界命中；隐藏或无效数据时返回 false。 */
    bool containsScreenPoint(const QPointF &point, const MapRenderContext &context) const override;

private:
    bool parameters(QPointF &position, double &direction) const;
    QPainterPath screenPath(const MapRenderContext &context) const;
    QPointer<ShipItem> m_ship;
    QMetaObject::Connection m_changeConnection;
    QMetaObject::Connection m_destroyConnection;
    double m_radiusMeters = 0.0; ///< 0 表示跟随全局设置。
    double m_openingAngleDegrees = 0.0;
    int m_directionMode = -1; ///< -1 表示跟随全局设置。
};
