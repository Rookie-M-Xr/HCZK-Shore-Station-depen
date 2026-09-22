#pragma once

#include "map/tool/MapTool.h"
#include "map/component/ChartOrientationIndicatorTypes.h"
#include "map/core/MapSdkTypes.h"
#include "map/item/MapItem.h"
#include "sdk/EncViewerSdkGlobal.h"

#include <QColor>
#include <QEvent>
#include <QImage>
#include <QList>
#include <QMap>
#include <QOpenGLWidget>
#include <QRectF>
#include <QSize>
#include <QStringList>
#include <QVector>
#include <memory>
#include <atomic>
#include <optional>

class EncChartLayer;
class QAction;
class MapItemManager;
class QPainter;
class QContextMenuEvent;
class QPaintEvent;
class QMouseEvent;
class QPointF;
class QResizeEvent;
class QSize;
class QTimer;
class QWheelEvent;
class QDragEnterEvent;
class QDropEvent;
class ChartSettingsStorage;
class MapCanonicalViewRegion;
struct S52RenderContext;
enum class MapProjection;
enum class MapDrawnObjectSummarySort;
struct MapDrawnObjectClassSummary;

struct ViewportPixmapCacheSnapshot;

/**
 * @brief 地图显示控件，集中处理海图绘制、视口变换、交互和视口缓存。
 */
class ENCVIEWER_SDK_EXPORT MapWidget : public QOpenGLWidget
{
    Q_OBJECT

signals:
    /**
     * @brief 当前地图交互工具已更换。
     */
    void toolChanged(MapTool *tool);

    /**
     * @brief 地图视口状态发生变化。
     */
    void chartViewportChanged();

    /** @brief 视口顶部真方位角发生变化。 */
    void viewBearingChanged(double degrees);

    /** @brief 通知拖入的本地 ENC 路径，不代表已经导入或导入完成。 */
    void encFilesDropped(const QStringList &paths);

    /**
     * @brief 航线创建成功；结果在 GUI 事件循环中排队通知，取消或失败不通知。
     * @param routeId 新航线的进程内唯一 ID。
     * @param points 创建完成时的坐标快照，按航点顺序排列，WGS84：x 经度、y 纬度。
     * @note 后续编辑/删除不影响快照；只表示内存保存成功，业务持久化由宿主处理。
     */
    void routeCreate(MapItemId routeId, const QVector<QPointF> &points);

    /**
     * @brief 编辑保存成功，整批结果在 GUI 事件循环中一次排队通知；取消或失败不通知。
     * @param createdRoutes 本轮新增且保留的航线，键为 ID，值为保存时按航点顺序排列的坐标快照。
     * @param updatedRoutes 本轮修改且保留的原航线，键为 ID，值为保存时的坐标快照。
     * @param deletedRouteIds 本轮确认删除的原航线 ID；不包含新增后又删除的临时航线。
     * @note 坐标为 WGS84：x 经度、y 纬度。三类 ID 互斥；空保存可返回三个空容器。
     *       后续修改不影响快照；持久化由宿主处理。地图销毁会丢弃未送达通知。
     */
    void routeEdit(const QMap<MapItemId, QVector<QPointF>> &createdRoutes,
                   const QMap<MapItemId, QVector<QPointF>> &updatedRoutes,
                   const QVector<MapItemId> &deletedRouteIds);

public:
    /** @brief 原生 GL 内容插入的固定绘制阶段。 */
    enum class GlRenderStage {
        AfterMap,        ///< 海图主体之上，覆盖层和业务物标之下。
        AfterItems,      ///< Item/Tool 之上，信息组件之下。
        AfterComponents  ///< 最上层，可能覆盖比例尺等信息组件。
    };

    // 生命周期

    /**
     * @brief 构造地图控件，自动创建海图层、应用设置并监听 Cell 更新。
     * @param parent 可选 QWidget 父对象；为空时可独立 show，否则可嵌入布局。
     * @note 须先创建 QApplication 并成功初始化 EncSdk；只在 GUI 线程使用。
     *       不需要 ChartUiController 或 MainWindow 才能显示海图。
     */
    explicit MapWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构地图控件。
     */
    ~MapWidget() override;


    // 视图状态与导航

    /**
     * @brief 返回当前地图视图状态。
     */
    const MapViewState &viewState() const;

    /**
     * @brief 设置当前地图视图状态并刷新显示。
     */
    void setViewState(const MapViewState &state);

    /** @brief 返回视口顶部对应的真方位角；0 表示北向上。 */
    double viewBearingDegrees() const;

    /** @brief 设置视口顶部对应的真方位角。 */
    void setViewBearingDegrees(double degrees);

    /** @brief 在当前真方位角基础上旋转指定角度。 */
    void rotateViewBy(double deltaDegrees);

    /** @brief 将视口恢复为北向上。 */
    void resetViewBearing();

    /**
     * @brief 将地图中心移动到指定经纬度。
     */
    void centerOn(double lon, double lat);

    /**
     * @brief 缩放到当前海图图层全范围。
     */
    void fitToFullExtent();

    /**
     * @brief 移动并缩放到指定 WGS84 经纬度包围盒。
     * @param minLongitude 最小经度，单位为度。
     * @param minLatitude 最小纬度，单位为度。
     * @param maxLongitude 最大经度，单位为度。
     * @param maxLatitude 最大纬度，单位为度。
     * @return 包围盒和当前视口有效并完成导航时返回 true。
     */
    bool zoomToLonLatBoundingBox(double minLongitude,
                                 double minLatitude,
                                 double maxLongitude,
                                 double maxLatitude);

    /**
     * @brief 缩放到指定 cell 的覆盖范围。
     * @return 找到有效 Cell 并完成缩放时返回 true。
     */
    bool zoomToCell(qint64 cellId);

    /**
     * @brief 以固定步长放大地图。
     */
    void zoomIn();

    /**
     * @brief 以固定步长缩小地图。
     */
    void zoomOut();

    /**
     * @brief 按指定倍率缩放地图。
     * @param factor 有限正数；小于 1 放大，大于 1 缩小，等于 1 不改变比例尺。
     */
    void zoomBy(double factor);

    /**
     * @brief 设置滚轮缩放锚点策略。
     */
    void setZoomAnchor(MapZoomAnchor anchor);

    /**
     * @brief 返回当前滚轮缩放锚点策略。
     */
    MapZoomAnchor zoomAnchor() const;

    /** @brief 返回当前进程唯一的全局绘制 CRS。 */
    EncCoordinateReferenceSystem coordinateReferenceSystem() const;

    /** @brief 设置当前进程唯一的全局绘制 CRS 并异步重投影可见 Cell。 */
    bool setCoordinateReferenceSystem(const EncCoordinateReferenceSystem &crs,
                                      QString *errorMessage = nullptr);

    // 状态叠加层与基础绘制参数

    /**
     * @brief 设置地图比例尺叠加层显示状态。
     */
    void setScaleOverlayVisible(bool visible);

    /**
     * @brief 设置鼠标经纬度叠加层显示状态。
     */
    void setMouseLonLatOverlayVisible(bool visible);

    /**
     * @brief 设置当前坐标系叠加层显示状态。
     */
    void setProjectionOverlayVisible(bool visible);

    /** @brief 设置海图方向指示器显示状态。 */
    void setOrientationIndicatorVisible(bool visible);

    /** @brief 设置海图方向指示器停靠位置。 */
    void setOrientationIndicatorPosition(ChartOrientationIndicatorPosition position);

    /** @brief 设置海图方向指示器直径，单位为逻辑像素。 */
    void setOrientationIndicatorSize(int size);

    /** @brief 设置海图方向指示器资源颜色模式。 */
    void setOrientationIndicatorColorMode(ChartOrientationIndicatorColorMode mode);

    /**
     * @brief 设置地图绘制抗锯齿状态。
     */
    void setAntialiasingEnabled(bool enabled);

    /**
     * @brief 返回地图绘制抗锯齿状态。
     */
    bool isAntialiasingEnabled() const;

    /**
     * @brief 设置 OpenGL MSAA 样本数，0 表示关闭。
     *
     * 应在控件首次显示、OpenGL context 创建前调用；运行中修改会在 context 下次创建时生效。
     */
    void setMsaaSamples(int samples);

    /** @brief 返回期望使用的 OpenGL MSAA 样本数。 */
    int msaaSamples() const;

    /**
     * @brief 设置水域背景色。
     */
    void setWaterBackgroundColor(const QColor &color);

    /**
     * @brief 返回水域背景色。
     */
    QColor waterBackgroundColor() const;

    // 交互工具

    /**
     * @brief 返回当前交互工具的真实运行时类型，包括自定义类型值。
     * @return 当前工具的 type()；没有工具时返回 MapToolTypes::Pan。
     */
    MapToolType interactionTool() const;

    /**
     * @brief 按运行时类型切换内置交互工具。
     * @param tool MapToolTypes 中的内置工具类型；未知或自定义类型不会创建工具或替换当前工具。
     * @note 自定义工具通过 setTool() 注入；重复选择当前航线创建或编辑工具会重新启动相应操作。
     */
    void setInteractionTool(MapToolType tool);

    /**
     * @brief 设置当前地图交互工具并接管其生命周期。
     * @param tool 待激活的地图工具。
     * @note 工具应绑定当前 MapWidget；内置工具安装后自动就绪，无需额外启动。
     *       支持单指点选/拖动及双指平移、缩放和旋转，触屏操作栏提供完成和取消入口。
     */
    void setTool(std::unique_ptr<MapTool> tool);

    /**
     * @brief 返回当前地图交互工具的借用指针，切换工具后失效。
     */
    MapTool *tool() const;

    /**
     * @brief 返回地图业务对象管理器。
     * @note 已注册 Item 的增删及属性变化自动通知控件重绘。
     */
    MapItemManager &itemManager();

    /** @brief 返回只读地图业务对象管理器。 */
    const MapItemManager &itemManager() const;

    /**
     * @brief 查询屏幕坐标处命中的地图业务对象。
     * @param screenPoint 控件内的屏幕坐标。
     * @return 按绘制顺序从上到下返回首个命中对象，未命中时返回空指针。
     * @note 调用方无需构造 MapRenderContext，控件会使用当前视图状态完成命中测试。
     */
    MapItemPtr itemAt(const QPointF &screenPoint) const;

    // ENC 图层显示参数

    /**
     * @brief 批量设置对象图层可见性并刷新地图。
     * @param visibility 键为大写 acronym:primitive，如 SOUNDG:1、COALNE:2、DEPARE:3；
     *        数字 1/2/3 分别表示 Point/Line/Area。仅更新传入键，未传入键保留原状态。
     */
    void setChartLayerVisibility(const QMap<QString, bool> &visibility);

    /**
     * @brief 设置 ENC cell 可见性并刷新地图。
     */
    void setChartCellVisible(qint64 cellId, bool visible);

    /**
     * @brief 批量设置 ENC cell 可见性并刷新地图。
     * @param visibility Cell ID 到显示状态的增量映射；未传入的已设置 Cell 保留原状态。
     */
    void setChartCellVisibility(const QMap<qint64, bool> &visibility);

    /**
     * @brief 设置是否强制显示基础面图层并刷新地图。
     */
    void setChartForceBaseLayersVisible(bool enabled);

    /**
     * @brief 设置 cell 覆盖范围叠加层显示状态并刷新地图。
     */
    void setChartCellCoverageOverlayVisible(bool visible);

protected:
    // 绘制扩展接口

    /**
     * @brief 绘制完整地图帧。
     * @note 顺序为 map → GL(AfterMap) → cover → custom → item/tool → GL(AfterItems)
     *       → components → GL(AfterComponents)。高级派生类可重写并选择调用基类。
     */
    void paintGL() override;

    /**
     * @brief 在固定阶段叠加原生 GL 内容，默认不绘制。
     * @param stage 当前固定绘制阶段，按枚举顺序每帧调用。
     * @note 仅在 GUI 线程、有效 GL context 和 native painting 区间内调用。
     *       无需 makeCurrent 或自行调用 beginNativePainting；viewport 使用物理像素。
     *       必须恢复自行修改的 GL 状态，不得切换 context、交换缓冲或缓存 framebuffer ID。
     *       SDK 恢复 framebuffer/viewport 后继续 QPainter 绘制；GL 资源需随 context 重建。
     */
    virtual void paintGlStage(GlRenderStage stage);


    /**
     * @brief 绘制 SDK 或业务扩展内容，默认不绘制。
     * @note 绘制在 cover 的上层、item/tool 的下层。
     * @param painter 当前控件绘制器。
     */
    virtual void paintCustom(QPainter &painter);

    /**
     * @brief 绘制位于地图、业务要素和交互工具之上的控件层。
     *
     * 默认绘制比例尺、坐标信息和性能信息。派生类重写时应在自定义控件绘制完成后
     * 调用基类实现，以保证内置控件始终位于最顶层。
     * @note 位于 QPainter 阶段的最上层；之后仍可叠加 GL(AfterComponents)。
     * @param painter 当前控件绘制器。
     */
    virtual void paintComponents(QPainter &painter);

    // 事件处理

private:
    void cancelTouchGesture();
    void initializeTouchControls();
    void showTouchMenu(const QPoint &position);
    void sendTouchKey(int key);
    void dispatchTouchMouse(QEvent::Type type, const QPointF &position);

    void paintFrame(QPainter &painter);
    void invokeGlStage(QPainter &painter, GlRenderStage stage);

    /**
     * @brief 返回当前是否已创建 ENC 海图图层。
     */
    bool hasChartLayer() const;

    /**
     * @brief 创建新的 ENC 海图图层并替换当前图层。
     */
    void rebuildChartLayer();

    /**
     * @brief 登记 ready Cell，合并到下一次定时视口刷新。
     */
    void handleChartCellReady(qint64 cellId);
    /** @brief 按最新视口和 LOD 合并刷新待处理 Cell；导入结束时立即调用。 */
    void flushChartCellReady();

    /**
     * @brief 删除 cell 后清理当前绘制结果并刷新视口。
     */
    void removeChartCells(const QList<qint64> &cellIds);

    /**
     * @brief 移除指定 Cell 的进程内绘制缓存，不删除磁盘缓存文件。
     */
    void removeMemoryCellCache(qint64 cellId);

    /**
     * @brief 清空全部进程内绘制缓存，不删除磁盘缓存文件。
     */
    void clearMemoryCellCaches();

    /**
     * @brief 请求当前 ENC 图层加载指定视口内的可见对象。
     */
    void requestChartVisibleFeatures(const QRectF &viewRect);

    /**
     * @brief 返回当前地图投影。
     */
    MapProjection projection() const;

    /**
     * @brief 设置当前地图投影并刷新视图。
     */
    void setProjection(MapProjection projection);

    /**
     * @brief 返回 FPS 叠加层是否可见。
     */
    bool isFpsVisible() const;

    /**
     * @brief 设置 FPS 叠加层显示状态。
     */
    void setFpsVisible(bool visible);

    /**
     * @brief 设置视口 pixmap 缓存最大外圈数。
     */
    void setViewportPixmapCacheMaxRings(int rings);

    /**
     * @brief 返回视口 pixmap 缓存最大外圈数。
     */
    int viewportPixmapCacheMaxRings() const;

    /**
     * @brief 设置视口 pixmap 缓存最多保留的瓦片数量，0 表示按外圈数自动计算。
     */
    void setViewportPixmapCacheMaxTileCount(int count);

    /**
     * @brief 返回视口 pixmap 缓存最多保留的瓦片数量。
     */
    int viewportPixmapCacheMaxTileCount() const;

    /**
     * @brief 设置视口 pixmap 缓存后台线程数。
     */
    void setViewportPixmapCacheThreadCount(int count);

    /**
     * @brief 返回视口 pixmap 缓存后台线程数。
     */
    int viewportPixmapCacheThreadCount() const;

    /**
     * @brief 设置视口 pixmap 缓存重建延迟时间。
     */
    void setViewportPixmapCacheBuildDelayMs(int delayMs);

    /**
     * @brief 返回视口 pixmap 缓存重建延迟时间。
     */
    int viewportPixmapCacheBuildDelayMs() const;

    /**
     * @brief 将当前视口 pixmap 缓存快照异步保存为调试图。
     */
    void saveViewportPixmapCacheDebugImage();

    /**
     * @brief 返回当前内存中视口 Pixmap 瓦片及构建状态的只读快照。
     */
    ViewportPixmapCacheSnapshot viewportPixmapCacheSnapshot() const;

    /**
     * @brief 设置远离视口 cell 缓存自动卸载策略。
     */
    void setChartAutoUnloadDistantCellCache(bool enabled, double viewportMarginFactor);

    /**
     * @brief 返回当前视口已绘制对象统计。
     */
    QList<MapDrawnObjectClassSummary> currentDrawnObjectSummaries(MapDrawnObjectSummarySort sortMode) const;

    /** @brief 清空视口 pixmap 缓存，使设置变更立即反映到下一帧。 */
    void invalidateViewportPixmapCache();

    /**
     * @brief 使用指定绘制器绘制地图主体。
     * @note 位于完整绘制顺序的最底层。
     * @param painter 当前控件绘制器。
     */
    void paintMap(QPainter &painter);

    /**
     * @brief 绘制地图动态覆盖层。
     * @note 绘制在 map 的上层、custom 的下层。
     * @param painter 当前控件绘制器。
     */
    void paintCover(QPainter &painter);

    /**
     * @brief 绘制全部 MapItem 和当前 MapTool。
     * @note 绘制在 custom 的上层、components 的下层；顺序为 Item、Tool、删除按钮。
     * @param painter 当前控件绘制器。
     */
    void paintItemAndTool(QPainter &painter);

    /** @brief 统一处理触摸手势，其他事件交由 QWidget。 */
    bool event(QEvent *event) override;

    /** @brief 处理鼠标按下并启动平移、框选或点选流程。 */
    void mousePressEvent(QMouseEvent *event) override;

    /** @brief 处理鼠标移动、拖拽、框选和经纬度反馈。 */
    void mouseMoveEvent(QMouseEvent *event) override;

    /** @brief 处理鼠标释放并结束平移或框选流程。 */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /** @brief 将鼠标双击事件分发给当前地图工具。 */
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    /** @brief 将地图右键菜单请求转发给当前工具。 */
    void contextMenuEvent(QContextMenuEvent *event) override;

    /** @brief 处理滚轮缩放。 */
    void wheelEvent(QWheelEvent *event) override;

    /** @brief 鼠标离开地图时清理 hover 状态。 */
    void leaveEvent(QEvent *event) override;

    /** @brief 视口尺寸变化后刷新地图状态和缓存。 */
    void resizeEvent(QResizeEvent *event) override;

    /** @brief 接受包含本地文件或目录的拖放进入事件。 */
    void dragEnterEvent(QDragEnterEvent *event) override;

    /** @brief 提取拖入的本地路径并交给主窗口导入。 */
    void dropEvent(QDropEvent *event) override;

    struct ViewportPixmapTile;
    struct ViewportPixmapBuildRequest;
    class Private;

    // ENC 图层生命周期

    /** @brief 替换当前 ENC 海图图层并重置相关缓存。 */
    void setChartLayer(const std::shared_ptr<EncChartLayer> &layer);
    /** @brief 设置 S-52 渲染上下文并刷新地图。 */
    void setChartS52RenderContext(const S52RenderContext &context);
    /** @brief 重建导入前释放图层和视口 pixmap 内存缓存。 */
    void clearChartCachesForRebuild();
    /** @brief 响应全局绘制 CRS 切换并刷新当前地图视图。 */
    void handleCoordinateReferenceSystemChanged(const EncCoordinateReferenceSystem &crs);

    // 绘制辅助函数

    /** @brief 绘制比例尺、鼠标经纬度和当前坐标系叠加层。 */
    void drawMapInfoOverlay(QPainter &painter);
    /** @brief 在同一矩形中绘制 FPS 和内存性能信息。 */
    void drawPerformanceOverlay(QPainter &painter);

    // 视口变化与查询调度

    /** @brief 围绕指定屏幕锚点缩放地图。 */
    void zoomByAt(double factor, const QPointF &anchorScreenPos);
    /** @brief 记录视图变化并失效不再适用的缓存。 */
    void noteViewChanged(bool scaleChanged);
    /** @brief 延迟调度缩放后的视口查询。 */
    void scheduleZoomViewportQuery(int delayOverrideMs = -1);
    /** @brief 延迟调度平移后的视口查询。 */
    void schedulePanViewportQuery();
    /** @brief 立即提交待处理视口查询。 */
    void submitViewportQueryNow();
    /** @brief 请求图层加载视口对象并发出视口变化通知。 */
    void requestChartVisibleFeaturesAndNotify(const QRectF &viewRect);
    /** @brief 请求图层加载规范区域集合内的对象并发出视口变化通知。 */
    void requestChartVisibleRegionAndNotify(const MapCanonicalViewRegion &viewRegion);

    // 地图工具与对象命中

    /** @brief 返回是否存在正在进行的地图工具交互。 */
    bool isInteractionBusy() const;
    /** @brief 返回当前是否正在拖拽地图。 */
    bool isDraggingMap() const;
    /** @brief 返回屏幕点命中的临时物标删除按钮对应 ID。 */
    MapItemId rubberDeleteButtonAt(const QPointF &screenPoint) const;
    /** @brief 切换屏幕点命中的 cell 覆盖遮罩状态。 */
    bool toggleCellCoverageMaskAtScreenPoint(const QPointF &screenPos);
    /** @brief 返回屏幕点命中的 cell ID。 */
    std::optional<qint64> cellCoverageLabelAtScreenPoint(const QPointF &screenPos) const;
    /** @brief 更新屏幕点命中的扇形灯 hover 状态。 */
    bool updateHoveredSectorLightAtScreenPoint(const QPointF &screenPos);

    // 视口 pixmap 缓存

    /** @brief 标记当前视口缓存为脏并安排后台重绘。 */
    void refreshViewportPixmapCache();
    /** @brief 清空视口 pixmap 缓存。 */
    void clearViewportPixmapCache();
    /** @brief 取消旧代次构建，但保留真实在途计数直到任务完成。 */
    void cancelViewportPixmapCacheRequests();
    /** @brief 取消正在进行的视口 pixmap 后台构建，不清空已生效缓存。 */
    void cancelViewportPixmapCacheBuild();
    /** @brief 重建视口 pixmap 缓存覆盖路径。 */
    void rebuildViewportPixmapCacheCoveragePath();
    /** @brief 将当前视口缺失的 pixmap 缓存任务加入 FIFO 队列。 */
    int enqueueViewportPixmapCacheBuildRequests();
    /** @brief 从 FIFO 队列启动后台 pixmap 缓存任务。 */
    void drainViewportPixmapCacheBuildQueue();
    /** @brief ENC 查询或预取完成后重新调度等待数据的瓦片。 */
    void retryWaitingViewportPixmapCacheBuilds();
    /** @brief 预取失败后按有限次数重新提交当前目标覆盖区域。 */
    void retryWaitingViewportPixmapCacheDataRequest(bool staleRequest);
    /** @brief 合入一个生成完成的视口 pixmap 缓存。 */
    void mergeViewportPixmapCacheTile(const QString &key, ViewportPixmapTile &&tile);
    /** @brief 标记当前视口内已缓存 tile 的绘制内容需要后台刷新。 */
    void markCurrentViewportPixmapCacheDirty();
    /** @brief 清除非当前目标瓦片，并按 Euclidean 格距从远到近执行数量淘汰。 */
    void enforceViewportPixmapCacheMaxTileCount();
    /** @brief 安排视口 pixmap 缓存后台构建。 */
    void scheduleViewportPixmapCacheBuild();
    /** @brief 启动视口 pixmap 缓存后台构建。 */
    void startViewportPixmapCacheBuild();
    /** @brief 绘制命中的视口 pixmap 缓存。 */
    bool drawViewportPixmapCache(QPainter &painter, const MapViewState &state);
    /** @brief 在后台渲染一个视口缓存图像。 */
    static QImage renderViewportCacheImage(const std::shared_ptr<EncChartLayer> &layer,
                                    const MapViewState &state,
                                    const QSize &viewportSize,
                                    qreal devicePixelRatio,
                                    const QColor &backgroundColor,
                                    bool antialiasingEnabled,
                                    const QPointF &renderOffsetPixels,
                                    bool backgroundOnly,
                                    int ring,
                                    int tileX,
                                    int tileY,
                                    const std::shared_ptr<std::atomic_bool> &cancelled);

    std::unique_ptr<Private> d; ///< 地图图层、交互和视口缓存私有实现。
    friend class ChartSettingsStorage;
    friend class MainWindow;
    friend class EncChartWorkspaceController;
    friend class ChartSettingsWidget;
    friend class ChartSettingsPage;
    friend class ChartUiController;
    friend class CellListWidget;
    friend class FeatureInfoWidget;
    friend class ViewportObjectWidget;
    friend class MemoryCellCacheDialog;
    friend class ViewportPixmapCacheDialog;
    friend class MapWidgetTestAccess;
    friend class PanTool;
    friend class QueryBoxTool;
    friend class CreateRouteTool;
    friend class EditRouteTool;
};
