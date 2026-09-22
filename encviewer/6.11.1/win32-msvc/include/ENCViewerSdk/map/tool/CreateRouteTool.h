#pragma once

#include <optional>

#include "map/tool/MapTool.h"
#include "map/item/RouteItem.h"

#include <QMenu>
#include <QPoint>

class QAction;
class QKeyEvent;
class QTimer;

/** @brief 航线创建工具，只负责创建新的航线物标。 */
class ENCVIEWER_SDK_EXPORT CreateRouteTool final : public MapTool
{
    Q_OBJECT

private:
    friend class MapWidget;
    friend class MainWindow;
    friend class MapToolTestAccess;
    /** @brief 航线创建状态。 */
    enum class State
    {
        ReadyToCreate, ///< 等待创建下一条航线。
        Creating      ///< 正在创建航线。   
    };

public:
    /** @brief 构造绑定到指定地图控件的航线创建工具。 */
    explicit CreateRouteTool(MapWidget *mapWidget);
    /** @brief 析构工具；由 MapWidget 接管所有权。 */
    ~CreateRouteTool() override = default;

private:

    /** @brief 返回航线创建工具运行时类型。 */
    MapToolType type() const noexcept override;
    /** @brief 返回航线创建工具使用的十字光标。 */
    Qt::CursorShape cursorShape() const override;
    /** @brief 放弃当前草稿并清理鼠标状态。 */
    void reset() override;
    /** @brief 处理新增航点或地图平移的按下事件。 */
    bool mousePressEvent(QMouseEvent *event) override;
    /** @brief 处理航点预览或地图平移。 */
    bool mouseMoveEvent(QMouseEvent *event) override;
    /** @brief 确认航点或结束地图平移。 */
    bool mouseReleaseEvent(QMouseEvent *event) override;
    /** @brief 右键双击完成当前航线。 */
    bool mouseDoubleClickEvent(QMouseEvent *event) override;
    /** @brief 显示航线创建工具的右键菜单。 */
    bool contextMenuEvent(QContextMenuEvent *event) override;
    /** @brief 清理右键菜单出现前残留的鼠标手势。 */
    void cancelPointerGesture() override;
    /** @brief 使用外部输入的 WGS84 坐标新增航点。 */
    bool inputCoordinate(const QPointF &lonLat) override;
    /** @brief 返回是否存在正在执行的鼠标手势。 */
    bool isBusy() const override;
    /** @brief 绘制创建中的航线和预览航段。 */
    void draw(QPainter &painter) const override;
    /** @brief 返回当前是否正在拖拽地图。 */
    bool isDraggingMap() const override;

    /** @brief 开始创建自动分配 ID 的航线；名称为空时按生成的 ID 命名。 */
    bool startCreating(const QString &routeName = {});
    /** @brief 完成当前航线并保持创建工具激活。 */
    bool finish();
    /** @brief 放弃当前航线草稿。 */
    void cancel();
    /** @brief 放弃当前航线草稿并等待创建下一条航线。 */
    void discardCurrentRoute();
    /** @brief 返回当前创建状态。 */
    State state() const noexcept;
    /** @brief 返回当前活动航线 ID。 */
    MapItemId activeRouteId() const noexcept;

private:
    /** @brief 新航线创建完成。 */
    Q_SIGNAL void routeCreated(MapItemId routeId);
    /** @brief 创建中的航线数据已经改变。 */
    Q_SIGNAL void routeChanged(MapItemId routeId);
    /** @brief 航线创建被取消。 */
    Q_SIGNAL void routeCanceled(MapItemId routeId);

private:
    /** @brief 从所属 MapWidget 接收航线创建快捷键。 */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    enum class MouseState
    {
        Idle,
        PendingMapAction,
        Panning
    };

    bool keyPressEvent(QKeyEvent *event);
    bool commitActiveRoute();
    void clearActiveRoute();
    void publishWaypoints(QVector<RouteWaypoint> waypoints);
    void addWaypoint(const QPointF &lonLat);
    QVector<QPointF> activeScreenPoints() const;
    void updatePreview(const QPointF &screenPoint);
    void beginPanning();
    void panTo(const QPoint &screenPosition);
    void endPanning();
    void clearMouseState();
    /** @brief 工具自行获取 WGS84 输入并处理结果；取消不改变当前数据。 */
    void showCoordinateInput(std::optional<QPointF> initialCoordinate = std::nullopt);
    void showCoordinateInputMenu(const QPoint &globalPos, bool useInitialCoordinate);

    State m_state = State::ReadyToCreate;         ///< 当前创建状态。
    MouseState m_mouseState = MouseState::Idle;   ///< 当前鼠标手势状态。
    QAction *m_coordinateInputAction = nullptr;   ///< 坐标输入动作。
    QTimer *m_contextMenuTimer = nullptr;          ///< 右键菜单双击判定定时器。
    QMenu m_contextMenu;                           ///< 航线创建工具持有的右键菜单。
    QAction *m_finishAction = nullptr;             ///< 完成当前航线动作。
    QAction *m_discardAction = nullptr;            ///< 放弃当前航线动作。
    QAction *m_menuCoordinateAction = nullptr;     ///< 菜单坐标输入动作。
    QPoint m_contextMenuPosition;                  ///< 待显示菜单的位置。
    bool m_useInitialContextCoordinate = false;    ///< 菜单坐标输入是否使用右键位置。
    bool m_ignoreNextContextMenu = false;          ///< 右键双击后忽略补发菜单。
    bool m_suppressNextLeftPress = false;          ///< 菜单外部左键关闭后忽略重投递的按下事件。
    QSharedPointer<RouteItem> m_activeRoute; ///< 当前创建中的航线原对象。
    QPoint m_pressMousePos;                        ///< 左键按下位置。
    QPoint m_lastMousePos;                         ///< 平移上一次位置。
    QPointF m_previewScreenPos;                    ///< 当前预览屏幕位置。
    int m_selectedWaypoint = -1;                   ///< 当前选中航点。
};
