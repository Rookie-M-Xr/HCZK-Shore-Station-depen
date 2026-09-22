#pragma once

#include "map/core/MapSdkTypes.h"
#include "sdk/EncViewerSdkGlobal.h"

#include <QObject>
#include <QHash>
#include "map/item/MapItemTypes.h"
#include <Qt>

class MapWidget;
class QContextMenuEvent;
class QMouseEvent;
class QMenu;
class QPainter;
class QPointF;

/**
 * @brief 地图交互工具基类。
 */
class ENCVIEWER_SDK_EXPORT MapTool : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造绑定到指定地图控件的工具。
     */
    explicit MapTool(MapWidget *mapWidget)
        : QObject(nullptr)
        , m_mapWidget(mapWidget)
    {
    }

    /**
     * @brief 析构地图交互工具。
     */
    virtual ~MapTool() = default;

    /**
     * @brief 退出当前工具，丢弃未完成操作并返回平移工具。
     * @note 仅在 GUI 线程调用。延迟到事件循环切换，由地图调用 reset() 清理派生类状态，
     *       非当前工具、平移工具及重复退出请求不产生作用。
     */
    void exit();

    /** @brief 设置指定物标类型的选择权限，覆盖默认权限。 */
    void setItemTypeSelectable(MapItemType type, bool selectable)
    { m_selectableItemTypes.insert(type, selectable); }

    /** @brief 返回指定物标类型是否允许选中；无效类型始终不可选。 */
    bool allowsItemSelection(MapItemType type) const
    { return type != MapItemTypes::Invalid && m_selectableItemTypes.value(type, m_allItemTypesSelectable); }

    /** @brief 设置所有类型（含未来自定义类型）的默认权限，并清空逐类型覆盖。 */
    void setAllItemTypesSelectable(bool selectable)
    { m_allItemTypesSelectable = selectable; m_selectableItemTypes.clear(); }

    /** @brief 返回是否开启了任何类型的物标选择。 */
    bool hasSelectableItemTypes() const
    { return m_allItemTypesSelectable || m_selectableItemTypes.values().contains(true); }

    /**
     * @brief 返回工具类型。
     */
    virtual MapToolType type() const noexcept = 0;

protected:
    /**
     * @brief 为工具自有菜单追加统一的退出项；PanTool 不添加。
     * @param menu 已添加业务动作的菜单，每个菜单仅调用一次。
     * @note 在派生类构造阶段或之后调用，不绑定快捷键。
     */
    void appendExitAction(QMenu &menu);

    /**
     * @brief 返回工具激活时使用的鼠标光标。
     */
    virtual Qt::CursorShape cursorShape() const = 0;

    /**
     * @brief 重置工具内部交互状态。
     */
    virtual void reset() = 0;

    /**
     * @brief 处理鼠标按下事件，返回是否已处理。
     */
    virtual bool mousePressEvent(QMouseEvent *event) = 0;

    /**
     * @brief 处理鼠标移动事件，返回是否已处理。
     */
    virtual bool mouseMoveEvent(QMouseEvent *event) = 0;

    /**
     * @brief 处理鼠标释放事件，返回是否已处理。
     */
    virtual bool mouseReleaseEvent(QMouseEvent *event) = 0;

    /**
     * @brief 处理鼠标双击事件，返回是否已经处理。
     */
    virtual bool mouseDoubleClickEvent(QMouseEvent *event);

    /**
     * @brief 处理当前工具的右键菜单请求。
     * @return 当前工具已接管菜单事件时返回 true。
     */
    virtual bool contextMenuEvent(QContextMenuEvent *event);

    /**
     * @brief 取消当前鼠标按键形成的临时手势。
     *
     * 该操作只清理按下、拖动和平移状态，不重置工具业务数据。
     */
    virtual void cancelPointerGesture();

    /**
     * @brief 使用外部输入的 WGS84 坐标执行一次当前工具的点位操作。
     * @return 当前工具接受该坐标时返回 true。
     */
    virtual bool inputCoordinate(const QPointF &lonLat);

    /**
     * @brief 返回工具是否正在执行交互。
     */
    virtual bool isBusy() const = 0;

    /**
     * @brief 绘制工具叠加内容。
     * @param painter 当前地图绘制器。
     */
    virtual void draw(QPainter &painter) const = 0;

    /**
     * @brief 返回工具是否正在拖拽地图。
     */
    virtual bool isDraggingMap() const
    {
        return false;
    }

protected:
    /** @brief 当前输入是否来自触屏，供派生工具调整命中区域。 */
    bool isTouchInput() const noexcept { return m_touchInput; }
    /** @brief 当前单指已超过拖动阈值，不应执行仅限点击的控制按钮动作。 */
    bool isTouchDrag() const noexcept { return m_touchInput && m_touchDrag; }
    /** @brief 触屏控制点的命中半径至少为 22 个逻辑像素。 */
    qreal pointerHitRadius(qreal mouseRadius) const noexcept
    { return m_touchInput ? qMax(qreal(22), mouseRadius) : mouseRadius; }

    MapWidget *m_mapWidget = nullptr; ///< 工具所属地图控件。

private:
    friend class MapWidget;
    friend class MapToolTestAccess;
    friend class FeatureInfoWidget;
    bool m_touchInput = false;
    bool m_exitPending = false; ///< 防止重复执行退出清理。
    bool m_touchDrag = false;
    bool m_allItemTypesSelectable = false; ///< 工具默认禁用通用选择。
    QHash<MapItemType, bool> m_selectableItemTypes; ///< 逐类型选择权限覆盖。

private:
    /**
     * @brief 地图工具收到鼠标双击事件。
     */
    Q_SIGNAL void mapDoubleClicked(const QPointF &screenPos);
};
