#pragma once

#include <optional>

#include "map/tool/MapTool.h"
#include "map/item/RouteItem.h"

#include <QMap>
#include <QMenu>
#include <QPoint>
#include <QSet>
#include <QVector>
#include <optional>

class QAction;
class QKeyEvent;
class QTimer;

/** @brief 航线编辑工具，只负责选择和修改已有航线物标。 */
class ENCVIEWER_SDK_EXPORT EditRouteTool final : public MapTool
{
    Q_OBJECT

private:
    friend class MapWidget;
    friend class MainWindow;
    friend class MapToolTestAccess;
    /** @brief 航线编辑状态。 */
    enum class State
    {
        Idle,           ///< 编辑已经结束。
        Editing,        ///< 正在选择或编辑航线。
        ExtendingFront, ///< 从首航点向前延伸。
        ExtendingBack   ///< 从尾航点向后延伸。
    };

public:
    /** @brief 构造绑定到指定地图控件的航线编辑工具。 */
    explicit EditRouteTool(MapWidget *mapWidget);
    /** @brief 析构工具；由 MapWidget 接管所有权。 */
    ~EditRouteTool() override = default;

private:

    /** @brief 返回航线编辑工具运行时类型。 */
    MapToolType type() const noexcept override;
    /** @brief 返回航线编辑工具使用的十字光标。 */
    Qt::CursorShape cursorShape() const override;
    /** @brief 撤销未保存修改并结束编辑会话。 */
    void reset() override;
    /** @brief 处理航线选择、航点和控制柄按下事件。 */
    bool mousePressEvent(QMouseEvent *event) override;
    /** @brief 处理航点拖动、悬停或地图平移。 */
    bool mouseMoveEvent(QMouseEvent *event) override;
    /** @brief 处理选择、插入、延伸或结束拖动。 */
    bool mouseReleaseEvent(QMouseEvent *event) override;
    /** @brief 右键双击保存当前会话的全部航线修改。 */
    bool mouseDoubleClickEvent(QMouseEvent *event) override;
    /** @brief 显示航线编辑工具的右键菜单。 */
    bool contextMenuEvent(QContextMenuEvent *event) override;
    /** @brief 清理右键菜单出现前残留的鼠标手势。 */
    void cancelPointerGesture() override;
    /** @brief 使用外部输入的 WGS84 坐标移动选中航点或延伸航线。 */
    bool inputCoordinate(const QPointF &lonLat) override;
    /** @brief 返回是否存在正在执行的鼠标手势。 */
    bool isBusy() const override;
    /** @brief 绘制编辑航线、悬停提示和控制柄。 */
    void draw(QPainter &painter) const override;
    /** @brief 返回当前是否正在拖拽地图。 */
    bool isDraggingMap() const override;

    /** @brief 进入编辑模式；ID 为 0 时等待用户点击航线。 */
    bool startEditing(MapItemId routeId = MapItemIds::Invalid);
    /** @brief 保存当前会话的全部航线修改，但不退出工具。 */
    bool finish();
    /** @brief 撤销上次保存后的全部航线修改，但不退出工具。 */
    void cancel();
    /** @brief 返回当前编辑状态。 */
    State state() const noexcept;
    /** @brief 返回当前活动航线 ID。 */
    MapItemId activeRouteId() const noexcept;

private:
    /** @brief 航线修改已经保存。 */
    Q_SIGNAL void routeChanged(MapItemId routeId);
    /** @brief 单条航线修改已经保存。 */
    Q_SIGNAL void routeEditingFinished(MapItemId routeId);
    /** @brief 单条航线的未保存修改已经撤销。 */
    Q_SIGNAL void routeCanceled(MapItemId routeId);

private:
    /** @brief 从所属 MapWidget 接收航线编辑快捷键。 */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    enum class MouseState
    {
        Idle,
        PendingMapAction,
        PendingWaypoint,
        PendingSegment,
        PendingHandle,
        Panning,
        DraggingWaypoint
    };

    enum class ExtensionHandle
    {
        None,
        Front,
        Back
    };

    enum class HintAction
    {
        InsertSegment,
        SplitWaypoint,
        SplitSegment,
        MergeRoute
    };

    struct ActionHint
    {
        HintAction action = HintAction::InsertSegment;
        QPointF position;
        QPointF anchorPosition;
        int segmentIndex = -1;
        int waypointIndex = -1;
        MapItemId routeId = MapItemIds::Invalid;
        ExtensionHandle routeEnd = ExtensionHandle::None;
    };

    struct SegmentHit
    {
        int index = -1;
        QPointF closestPoint;
    };

    bool keyPressEvent(QKeyEvent *event);
    QSharedPointer<RouteItem> routeById(MapItemId routeId) const;
    QSharedPointer<RouteItem> routeAt(const QPointF &screenPoint) const;
    bool saveChanges();
    void restoreUnsavedChanges();
    void rememberOriginalRoute(const QSharedPointer<RouteItem> &route);
    bool selectRoute(MapItemId routeId);
    void clearActiveRoute();
    void publishWaypoints(QVector<RouteWaypoint> waypoints);
    void addWaypoint(const QPointF &lonLat);
    void insertWaypoint(int segmentIndex, const QPointF &lonLat);
    void moveWaypoint(int waypointIndex, const QPointF &lonLat);
    void deleteSelectedWaypoint();
    bool executeActionHint(int hintIndex);
    bool splitAtWaypoint(int waypointIndex);
    bool splitAtSegment(int segmentIndex, const QPointF &screenPoint);
    bool mergeWithRoute(MapItemId routeId, ExtensionHandle routeEnd);
    void updateActionHints(const QPointF &screenPoint);
    int actionHintAt(const QPointF &screenPoint) const;
    bool isNearActionHintPath(const ActionHint &hint, const QPointF &screenPoint) const;
    QSharedPointer<RouteItem> routeEndpointAt(const QPointF &screenPoint,
                                                     ExtensionHandle *routeEnd) const;
    void clearActionHints();
    void drawActionHint(QPainter &painter, const ActionHint &hint) const;
    QVector<QPointF> activeScreenPoints() const;
    int waypointAt(const QPointF &screenPoint) const;
    SegmentHit segmentAt(const QPointF &screenPoint) const;
    ExtensionHandle handleAt(const QPointF &screenPoint) const;
    std::optional<QPointF> extensionHandlePosition(ExtensionHandle handle) const;
    void updateHover(const QPointF &screenPoint);
    void updatePreview(const QPointF &screenPoint);
    void beginPanning();
    void panTo(const QPoint &screenPosition);
    void endPanning();
    void clearMouseState();
    /** @brief 工具自行获取 WGS84 输入并处理结果；取消不改变当前数据。 */
    void showCoordinateInput(std::optional<QPointF> initialCoordinate = std::nullopt);
    void showCoordinateInputMenu(const QPoint &globalPos, bool useInitialCoordinate);

    State m_state = State::Idle;                             ///< 当前编辑状态。
    MouseState m_mouseState = MouseState::Idle;              ///< 当前鼠标手势状态。
    QAction *m_coordinateInputAction = nullptr;              ///< 坐标输入动作。
    QTimer *m_contextMenuTimer = nullptr;                     ///< 右键菜单双击判定定时器。
    QMenu m_contextMenu;                                      ///< 航线编辑工具持有的右键菜单。
    QAction *m_finishAction = nullptr;                        ///< 保存全部修改动作。
    QAction *m_cancelAction = nullptr;                        ///< 撤销全部未保存修改动作。
    QAction *m_menuCoordinateAction = nullptr;                ///< 菜单坐标输入动作。
    QPoint m_contextMenuPosition;                             ///< 待显示菜单的位置。
    bool m_useInitialContextCoordinate = false;               ///< 菜单坐标输入是否使用右键位置。
    bool m_ignoreNextContextMenu = false;                     ///< 右键双击后忽略补发菜单。
    bool m_suppressNextLeftPress = false;                     ///< 菜单外部左键关闭后忽略重投递的按下事件。
    QSharedPointer<RouteItem> m_activeRoute;            ///< 当前活动航线原对象。
    struct RouteSnapshot {
        QSharedPointer<RouteItem> item;
        QVector<RouteWaypoint> waypoints;
    };
    QMap<MapItemId, RouteSnapshot> m_originalRoutes; ///< 各航线未保存修改前的快照。
    QMap<MapItemId, QSharedPointer<RouteItem>> m_removedRoutes;  ///< 本次编辑中已移除的航线原对象。
    QSet<MapItemId> m_createdRoutes;                         ///< 本次编辑中新增的航线 ID。
    QPoint m_pressMousePos;                                   ///< 左键按下位置。
    QPoint m_lastMousePos;                                    ///< 平移上一次位置。
    QPointF m_previewScreenPos;                               ///< 当前鼠标屏幕位置。
    MapItemId m_pendingRouteId = MapItemIds::Invalid;         ///< 待选择航线 ID。
    int m_pendingWaypoint = -1;                               ///< 待选择或拖动航点。
    int m_pendingSegment = -1;                                ///< 待插入航段。
    ExtensionHandle m_pendingHandle = ExtensionHandle::None;  ///< 待激活首尾控制柄。
    int m_selectedWaypoint = -1;                              ///< 当前选中航点。
    int m_hoverWaypoint = -1;                                 ///< 当前悬停航点。
    SegmentHit m_hoverSegment;                                ///< 当前悬停航段。
    QVector<ActionHint> m_actionHints;                         ///< 当前可点击的拆分、插入或合并提示。
};
