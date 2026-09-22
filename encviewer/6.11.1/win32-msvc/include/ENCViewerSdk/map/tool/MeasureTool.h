#pragma once

#include <optional>

#include "map/tool/MapTool.h"
#include <memory>

class PanTool;
#include "map/item/MeasureItem.h"

#include <QMenu>
#include <QPoint>
#include <QPointF>
#include <QVector>

class QKeyEvent;
class QAction;
class QContextMenuEvent;
class QTimer;

/**
 * @brief 多点折线测量工具。
 *
 * 左键单击增加测量点，拖动已有测量点修改位置，拖动空白区域平移地图。
 * Enter 完成当前测量；再次单击开始下一组测量，已有结果继续保留。
 * Backspace/Delete 删除当前测量点，Escape 取消当前一组测量。
 */
class ENCVIEWER_SDK_EXPORT MeasureTool final : public MapTool
{
    Q_OBJECT

private:
    friend class MapWidget;
    friend class MainWindow;
    friend class MapToolTestAccess;
    /** @brief 测量工具主状态。 */
    enum class State
    {
        Measuring, ///< 正在增加或编辑测量点。
        Finished   ///< 测量已经完成并保留显示结果。
    };

public:
    /** @brief 构造测量工具。 */
    explicit MeasureTool(MapWidget *mapWidget);
    /** @brief 析构工具；由 MapWidget 接管所有权。 */
    ~MeasureTool() override;

private:

    /** @brief 返回测量工具类型。 */
    MapToolType type() const noexcept override;
    /** @brief 返回十字光标。 */
    Qt::CursorShape cursorShape() const override;
    /** @brief 取消当前草稿或解除当前结果的编辑状态，已完成 Item 保留。 */
    void reset() override;
    /** @brief 开始测量点选择、节点拖动或地图平移判定。 */
    bool mousePressEvent(QMouseEvent *event) override;
    /** @brief 更新预览、拖动测量点或平移地图。 */
    bool mouseMoveEvent(QMouseEvent *event) override;
    /** @brief 确认新增测量点或结束当前拖动。 */
    bool mouseReleaseEvent(QMouseEvent *event) override;
    /** @brief 右键双击完成当前测量。 */
    bool mouseDoubleClickEvent(QMouseEvent *event) override;
    /** @brief 显示测量工具自己的右键菜单。 */
    bool contextMenuEvent(QContextMenuEvent *event) override;
    /** @brief 取消右键菜单出现前残留的测量鼠标手势。 */
    void cancelPointerGesture() override;
    /** @brief 使用外部输入的 WGS84 坐标新增测量点。 */
    bool inputCoordinate(const QPointF &lonLat) override;
    /** @brief 处理 Enter、Escape、Delete 和 Backspace。 */
    bool keyPressEvent(QKeyEvent *event);
    /** @brief 返回是否存在未完成测量或正在执行鼠标手势。 */
    bool isBusy() const override;
    /** @brief 绘制测量折线、节点、预览和距离方位文字。 */
    void draw(QPainter &painter) const override;
    /** @brief 返回当前是否正在拖拽地图。 */
    bool isDraggingMap() const override;

    /** @brief 返回当前测量状态。 */
    State state() const noexcept;
    /** @brief 返回全部已确认测量点的 WGS84 经纬度。 */
    const QVector<QPointF> &points() const noexcept;
    /** @brief 返回全部已确认航段的总距离，单位为米。 */
    double totalDistanceMeters() const;
    /** @brief 返回当前显示的测量结果数量，包含正在创建的一组。 */
    qsizetype measurementCount() const noexcept;

private:
    /** @brief 从所属 MapWidget 接收测量快捷键。 */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    enum class MouseState
    {
        Idle,
        PendingMapAction,
        PendingPoint,
        Panning,
        DraggingPoint
    };

    int pointAt(const QPointF &screenPoint) const;
    void updateHoverAndPreview(const QPointF &screenPoint);
    void removeSelectedPoint();
    void beginMeasurement(const QPointF &lonLat);
    void replacePoints(QVector<QPointF> points);
    bool publishActiveItem();
    void cancelCurrentMeasurement();
    void clearMouseState();
    /** @brief 工具自行获取 WGS84 输入并处理结果；取消不改变当前数据。 */
    void showCoordinateInput(std::optional<QPointF> initialCoordinate = std::nullopt);
    void showCoordinateInputMenu(const QPoint &globalPos, bool useInitialCoordinate);

    State m_state = State::Measuring;               ///< 测量主状态。
    MouseState m_mouseState = MouseState::Idle;     ///< 当前鼠标手势状态。
    QSharedPointer<MeasureItem> m_activeItem; ///< 当前创建或编辑的测量 Item。
    QAction *m_coordinateInputAction = nullptr;    ///< 测量工具坐标输入动作。
    QTimer *m_contextMenuTimer = nullptr;           ///< 测量工具右键菜单双击判定定时器。
    QMenu m_contextMenu;                            ///< 测量工具持有的右键菜单。
    QAction *m_menuCoordinateAction = nullptr;      ///< 菜单坐标输入动作。
    QPoint m_contextMenuPosition;                   ///< 待显示测量菜单的位置。
    bool m_useInitialContextCoordinate = false;     ///< 菜单坐标输入是否使用右键位置。
    bool m_ignoreNextContextMenu = false;           ///< 右键双击完成后忽略补发菜单。
    bool m_suppressNextLeftPress = false;           ///< 菜单外部左键关闭后忽略重投递的按下事件。
    bool m_activePublished = false;                ///< 当前 Item 是否已经交给管理器。
    QPointF m_previewLonLat;                        ///< 末段预览终点经纬度。
    bool m_hasPreview = false;                      ///< 是否存在末段预览。
    QPoint m_pressMousePos;                         ///< 左键按下位置。
    int m_pendingPoint = -1;                        ///< 待选择或拖动测量点。
    int m_selectedPoint = -1;                       ///< 当前选中测量点。
    int m_hoverPoint = -1;                          ///< 当前悬停测量点。
    std::unique_ptr<PanTool> m_panTool;                              ///< 复用地图已有平移实现。
};
