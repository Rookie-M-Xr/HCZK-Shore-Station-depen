#pragma once

#include "map/item/MapItem.h"
#include "sdk/EncViewerSdkGlobal.h"

#include <QList>
#include <QMap>
#include <QObject>

class MapRenderContext;
class MapTool;

/** @brief 批量清空轨迹的所属物标分类。 */
enum class TrackTargetGroup { Ship, Ais, Arpa, All };

/**
 * @brief 管理、绘制和命中测试 MapItem 的轻量容器。
 *
 * 所有注册、增删、绘制和命中操作均应在 MapWidget 所在线程执行。
 */
class ENCVIEWER_SDK_EXPORT MapItemManager : public QObject
{
    Q_OBJECT

signals:
    /** @brief 注册对象的选中状态或选中对象集合变化后发出，不属于业务数据更新。 */
    void selectionChanged();
    /** @brief 地图对象新增完成。 */
    void itemAdded(const MapItemPtr &item);

    /** @brief 已有地图对象更新完成。 */
    void itemUpdated(const MapItemPtr &item);

    /** @brief 地图对象删除完成。 */
    void itemRemoved(const MapItemPtr &item);

public:
    /** @brief 构造空地图对象管理器。 */
    explicit MapItemManager(QObject *parent = nullptr);

    /** @brief 析构地图对象管理器。 */
    ~MapItemManager() override;

    MapItemManager(const MapItemManager &) = delete;
    MapItemManager &operator=(const MapItemManager &) = delete;

    /** @brief 添加新对象并监听其更新；重复添加返回 false，不替换已有对象。
     * @note 船舶自动注册扇形、矢量线、轨迹，AIS／ARPA 自动注册轨迹；计数和通知包含附属对象。 */
    bool add(const MapItemPtr &item);

    /** @brief 按全局唯一 ID 删除地图对象；移除船舶时同步移除其航程矢量线。 */
    bool removeItem(MapItemId id);

    /** @brief 清空指定类型的全部地图对象。 */
    void clearItems(MapItemType type);

    /** @brief 清空全部地图对象。 */
    void clear();

    /** @brief 清空当前管理器中指定分类物标的轨迹点，默认清空三类全部轨迹。
     * @param group Ship 包含全部 ShipItem 子类；All 包含船舶、AIS、ARPA，不含独立 TrackItem。
     * @note 仅 GUI 线程；保留物标、轨迹对象、ID、显隐和样式，不补入当前位置。
     * 下一次位置更新按对应记录规则追加。 */
    void clearTracks(TrackTargetGroup group = TrackTargetGroup::All);

    /** @brief 返回全部地图对象数量。 */
    qsizetype itemCount() const;

    /** @brief 返回指定类型的地图对象数量。 */
    qsizetype itemCount(MapItemType type) const;

    /** @brief 按全局唯一 ID 返回地图对象。 */
    MapItemPtr item(MapItemId id) const;

    /** @brief 单选指定 ID，0 清空选择；无效非零 ID 不改变选择。 */
    void selectItem(MapItemId id);

    /** @brief 返回指定类型全部地图对象，按 ID 稳定排序。 */
    QList<MapItemPtr> items(MapItemType type) const;

private:
    friend class MapWidget;
    /** @brief 内部按绘制顺序命中当前工具允许选择的物标。 */
    MapItemPtr selectableItemAt(const QPointF &point, const MapRenderContext &context,
                               const MapTool &tool) const;

    /** @brief 按绘制顺序绘制全部可见地图对象。 */
    void draw(MapRenderContext &context) const;

    /** @brief 在 Item 和 Tool 上层绘制全部物标控制层。 */
    void drawTopOverlays(MapRenderContext &context) const;

    /** @brief 返回屏幕点命中的最上层临时物标删除按钮对应 ID。 */
    MapItemId rubberDeleteButtonAt(const QPointF &screenPoint,
                                   const MapRenderContext &context) const;

    /**
     * @brief 按绘制顺序从上到下返回首个命中对象。
     * @note 仅供 MapWidget 调用；SDK 调用使用 MapWidget::itemAt()。
     */
    MapItemPtr itemAt(const QPointF &screenPoint,
                      const MapRenderContext &context) const;

private:
    QList<MapItemPtr> orderedItems() const;

    bool m_selecting = false; ///< 管理器单选时合并本管理器的选择通知。
    QMap<MapItemId, MapItemPtr> m_items; ///< 全局 ID 到地图对象。
};
