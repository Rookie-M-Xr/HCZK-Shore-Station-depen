#pragma once

#include "map/item/MapItemTypes.h"
#include "sdk/EncViewerSdkGlobal.h"

#include <QObject>
#include <QRectF>
#include <QSharedPointer>

class MapRenderContext;

/**
 * @brief 地图业务对象抽象基类。
 *
 * 一个物标同时保存业务数据并实现其唯一绘制方式，ID 在进程内自动分配且不复用。
 * 对象不可复制或赋值；共享指针可以复制。加入管理器后，更新、选择、绘制和命中
 * 必须在 GUI 线程执行。使用共享指针管理生命周期，不得设置 QObject parent。
 */
class ENCVIEWER_SDK_EXPORT MapItem : public QObject
{
    Q_OBJECT
public:
    /** @brief 析构地图业务对象。 */
    ~MapItem() override;
    MapItem(const MapItem &) = delete;
    MapItem &operator=(const MapItem &) = delete;

    /** @brief 返回物标是否选中。 */
    bool isSelected() const noexcept { return m_selected; }

    /** @brief 设置本对象选中状态，已注册对象自动通知管理器重绘；不取消其他对象的选择。 */
    void setSelected(bool selected);

    /** @brief 在物标下层按实际图形绘制扩边高亮；draw 应在 geometryOnly 阶段跳过文字和背景。 */
    virtual void drawSelection(MapRenderContext &context) const;

    /** @brief 返回地图对象运行时类型。 */
    virtual MapItemType type() const noexcept = 0;

    /** @brief 返回基类自动生成的非零 ID；进程内唯一且不复用，不作为持久化业务编号。 */
    MapItemId id() const noexcept { return m_id; }

    /** @brief 返回 WGS84 经纬度范围。 */
    virtual QRectF geographicBounds() const = 0;

    /** @brief 返回对象是否参与绘制和命中测试。 */
    bool isVisible() const noexcept;

    /** @brief 设置是否参与绘制和命中测试；状态变化时自动通知管理器更新和重绘。
     * @param visible true 显示，false 隐藏；不改变选中状态。 */
    void setVisible(bool visible);

    /** @brief 返回绘制顺序，数值越大越靠近上层。 */
    virtual int drawOrder() const noexcept = 0;

    /** @brief 绘制物标，方法必须恢复自己修改的 QPainter 状态。 */
    virtual void draw(MapRenderContext &context) const = 0;

    /** @brief 判断视口局部逻辑像素坐标是否命中物标主体，例如符号、节点或线段。
     * @note 不检测删除按钮等交互控件，默认返回 false。 */
    virtual bool containsScreenPoint(const QPointF &screenPoint,
                         const MapRenderContext &context) const;

    /** @brief 在当前地图工具上方绘制交互控件，例如删除按钮。
     * @note 默认不绘制；不用于绘制物标主体。 */
    virtual void drawInteractionControls(MapRenderContext &context) const;

    /** @brief 判断视口局部逻辑像素坐标是否命中物标交互控件。
     * @note 默认返回 false；返回 true 仅表示命中，不执行删除等操作。 */
    virtual bool containsInteractionControl(const QPointF &screenPoint,
                                   const MapRenderContext &context) const;

protected:
    /** @brief 自动分配进程内唯一 ID，不接受外部 ID 或 QObject parent。 */
    explicit MapItem(bool visible = true);
    /** @brief 派生类修改业务数据后调用，已注册对象自动触发 itemUpdated 和重绘。 */
    void notifyChanged();

private:
    friend class MapItemManager;
    friend class ShipRangeVectorItem;
    friend class ShipSectorItem;
    friend class TrackItem;
    Q_SIGNAL void changed();
    Q_SIGNAL void selectedChanged();
    const MapItemId m_id;
    bool m_selected = false;
    bool m_visible = true; ///< 所有子类共用的可见状态。
};

/** @brief 可更新地图对象共享指针。 */
using MapItemPtr = QSharedPointer<MapItem>;
