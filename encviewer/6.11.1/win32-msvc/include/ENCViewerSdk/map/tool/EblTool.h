#pragma once

#include <optional>

#include "map/tool/MapTool.h"
#include <memory>

class PanTool;
#include "map/item/EblItem.h"

#include <QMenu>
#include <QPoint>
#include <QPointF>

class QAction;
class QKeyEvent;
class QTimer;

/**
 * @brief 用户选点电子方位线工具。
 *
 * 第一次单击设置圆心，移动鼠标调整半径和方向，第二次单击锁定。
 * 每组绘制距离圆和圆心到控制点的半径线；再次单击可开始下一组方位线。
 * 锁定后可拖动最新结果的末端控制柄；圆心不依赖本船或其他目标。
 */
class ENCVIEWER_SDK_EXPORT EblTool final : public MapTool
{
    Q_OBJECT

private:
    friend class MapWidget;
    friend class MainWindow;
    friend class MapToolTestAccess;
    /** @brief 电子方位线主状态。 */
    enum class State
    {
        AwaitingStart, ///< 等待用户指定基准点。
        Adjusting,     ///< 方位线跟随鼠标调整。
        Locked         ///< 方位线已经锁定。
    };

public:
    /** @brief 构造电子方位线工具。 */
    explicit EblTool(MapWidget *mapWidget);
    /** @brief 析构工具；由 MapWidget 接管所有权。 */
    ~EblTool() override;

private:

    /** @brief 返回电子方位线工具类型。 */
    MapToolType type() const noexcept override;
    /** @brief 返回十字光标。 */
    Qt::CursorShape cursorShape() const override;
    /** @brief 取消当前草稿或解除当前结果的编辑状态，已完成 Item 保留。 */
    void reset() override;
    /** @brief 开始基准点选择、控制柄拖动或地图平移判定。 */
    bool mousePressEvent(QMouseEvent *event) override;
    /** @brief 更新方位线、拖动控制柄或平移地图。 */
    bool mouseMoveEvent(QMouseEvent *event) override;
    /** @brief 确认方位线或结束当前拖动。 */
    bool mouseReleaseEvent(QMouseEvent *event) override;
    /** @brief 右键双击完成当前电子方位线。 */
    bool mouseDoubleClickEvent(QMouseEvent *event) override;
    /** @brief 显示电子方位线工具自己的右键菜单。 */
    bool contextMenuEvent(QContextMenuEvent *event) override;
    /** @brief 取消右键菜单出现前残留的方位线鼠标手势。 */
    void cancelPointerGesture() override;
    /** @brief 使用外部输入的 WGS84 坐标设置圆心或半径点。 */
    bool inputCoordinate(const QPointF &lonLat) override;
    /** @brief 处理 Enter 和 Escape。 */
    bool keyPressEvent(QKeyEvent *event);
    /** @brief 返回是否正在设置方位线或执行鼠标手势。 */
    bool isBusy() const override;
    /** @brief 绘制距离圆、半径线、控制柄和方位距离文字。 */
    void draw(QPainter &painter) const override;
    /** @brief 返回当前是否正在拖拽地图。 */
    bool isDraggingMap() const override;

    /** @brief 返回当前电子方位线状态。 */
    State state() const noexcept;
    /** @brief 返回基准点 WGS84 经纬度。 */
    QPointF startPoint() const noexcept;
    /** @brief 返回方向控制点 WGS84 经纬度。 */
    QPointF endPoint() const noexcept;
    /** @brief 返回当前真方位角，正北为 0 度并顺时针递增。 */
    double bearingDegrees() const;
    /** @brief 返回基准点到控制点的距离，单位为米。 */
    double rangeMeters() const;
    /** @brief 返回当前显示的电子方位线数量，包含正在调整的一组。 */
    qsizetype eblCount() const noexcept;

private:
    /** @brief 从所属 MapWidget 接收电子方位线快捷键。 */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    enum class MouseState
    {
        Idle,
        PendingMapAction,
        PendingHandle,
        Panning,
        DraggingHandle
    };

    bool isHandleAt(const QPointF &screenPoint) const;
    void beginEbl(const QPointF &lonLat);
    void replaceActiveItem(const QPointF &radiusPoint);
    bool publishActiveItem();
    void cancelCurrentEbl();
    void clearMouseState();
    /** @brief 工具自行获取 WGS84 输入并处理结果；取消不改变当前数据。 */
    void showCoordinateInput(std::optional<QPointF> initialCoordinate = std::nullopt);
    void showCoordinateInputMenu(const QPoint &globalPos, bool useInitialCoordinate);

    State m_state = State::AwaitingStart;           ///< 电子方位线主状态。
    MouseState m_mouseState = MouseState::Idle;     ///< 当前鼠标手势状态。
    QSharedPointer<EblItem> m_activeItem;      ///< 当前创建或编辑的电子方位线 Item。
    QAction *m_coordinateInputAction = nullptr;      ///< 电子方位线坐标输入动作。
    QTimer *m_contextMenuTimer = nullptr;             ///< 电子方位线右键菜单双击判定定时器。
    QMenu m_contextMenu;                              ///< 电子方位线工具持有的右键菜单。
    QAction *m_menuCoordinateAction = nullptr;        ///< 菜单坐标输入动作。
    QPoint m_contextMenuPosition;                     ///< 待显示电子方位线菜单的位置。
    bool m_useInitialContextCoordinate = false;       ///< 菜单坐标输入是否使用右键位置。
    bool m_ignoreNextContextMenu = false;             ///< 右键双击完成后忽略补发菜单。
    bool m_suppressNextLeftPress = false;             ///< 菜单外部左键关闭后忽略重投递的按下事件。
    bool m_activePublished = false;                 ///< 当前 Item 是否已经交给管理器。
    QPointF m_originalEndLonLat;                    ///< 取消控制柄调整时恢复的位置。
    QPoint m_pressMousePos;                         ///< 左键按下位置。
    bool m_hoverHandle = false;                     ///< 鼠标是否悬停末端控制柄。
    bool m_hasOriginalEnd = false;                  ///< 是否保存了控制柄拖动前的位置。
    std::unique_ptr<PanTool> m_panTool;                              ///< 复用地图已有平移实现。
};
