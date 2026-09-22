#pragma once

#include "map/core/MapSdkTypes.h"
#include "sdk/EncViewerSdkGlobal.h"

#include <QPointF>
#include <QSize>

class MapWidget;
class QPainter;

/**
 * @brief MapItem 绘制阶段共享的地图状态快照及绘制器访问入口。
 * @note 地图状态只读，QPainter 可修改；修改画笔、变换或裁剪后应恢复原状态。
 * @note 不持有绘制器和控件所有权。上下文及 painter() 引用仅限本次绘制期间使用，不得保存到异步任务。
 * @note 坐标转换只使用本上下文的视图快照及视口尺寸，不读取关联控件的实时状态。
 */
class ENCVIEWER_SDK_EXPORT MapRenderContext
{
public:
    /**
     * @brief 使用当前绘制器、视图快照和视口构造上下文。
     * @param painter 本次绘制的有效绘制器，其生命周期必须覆盖上下文使用期间。
     * @param viewState 有效 CRS、有限中心和朝向、正比例的地图视图快照。
     * @param viewportSize 正尺寸视口，单位为逻辑像素。
     * @param mapWidget 可为空的关联控件；仅供访问，不参与坐标转换，不转移所有权。
     */
    MapRenderContext(QPainter &painter,
                     const MapViewState &viewState,
                     const QSize &viewportSize,
                     const MapWidget *mapWidget);

    /** @brief 返回当前绘制器。 */
    QPainter &painter() const;

    /** @brief 返回本帧地图视图快照。 */
    const MapViewState &viewState() const;

    /** @brief 返回本次绘制的视口逻辑像素尺寸。 */
    QSize viewportSize() const;

    /** @brief 返回关联地图控件，可为空；访问控件必须在 GUI 线程进行。 */
    const MapWidget *mapWidget() const;

    /** @brief 返回视口顶部对应的真方位角。 */
    double viewBearingDegrees() const;

    /** @brief 按快照将有效 WGS84 经纬度转换为视口局部逻辑像素坐标。 */
    QPointF lonLatToScreen(const QPointF &lonLat) const;

    /** @brief 按快照将有限的视口局部逻辑像素坐标转换为 WGS84 经纬度。 */
    QPointF screenToLonLat(const QPointF &screenPoint) const;

    /** @brief 是否仅绘制图形，用于选中轮廓；此阶段应跳过文字及文字背景。 */
    bool geometryOnly() const noexcept { return m_geometryOnly; }
private:
    friend class MapItem;
    /** @brief 仅由 MapItem 控制选中轮廓绘制阶段；普通绘制默认关闭。 */
    void setGeometryOnly(bool enabled) noexcept { m_geometryOnly = enabled; }

    bool m_geometryOnly = false; ///< 仅生成图形轮廓，不绘制文字。
    QPainter *m_painter = nullptr;       ///< 当前绘制器，不持有所有权。
    MapViewState m_viewState;            ///< 本帧地图视图快照。
    QSize m_viewportSize;                ///< 本帧视口尺寸。
    const MapWidget *m_mapWidget = nullptr; ///< 所属地图控件，不持有所有权。
};
