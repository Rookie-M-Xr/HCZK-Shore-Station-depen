#pragma once

#include "enc/model/EncCoordinateReferenceSystem.h"

#include <QList>
#include <QRectF>
#include <QString>
#include <QtGlobal>
#include <memory>

struct MapViewStateRuntime;
class MapViewStateAccess;

/** @brief 保存当前地图视图中心、缩放和默认范围。 */
struct MapViewState
{
    double centerLon = 0.0;                                  ///< WGS84 视图中心经度，单位为度。
    double centerLat = 0.0;                                  ///< WGS84 视图中心纬度，单位为度。
    double mapUnitsPerPixel = 40075016.68557849 / 1024.0;     ///< 每像素地图单位数；单位取决于 CRS，不固定为米。
    double viewBearingDegrees = 0.0;                          ///< 视口顶部对应的真方位角；0 表示北向上。
    EncCoordinateReferenceSystem coordinateReferenceSystem = EncCoordinateReferenceSystem::epsg3857(); ///< 当前全局绘制 CRS。
    QRectF fullExtent = QRectF(-180.0, -90.0, 360.0, 180.0); ///< WGS84 地图完整范围，单位为度。
private:
    friend class MapViewStateAccess;
    // 内部快照共享只读数据，内部写入先分离，不向调用方暴露运行时字段。
    std::shared_ptr<MapViewStateRuntime> m_runtime;
};

/** @brief 两个 WGS84 点之间的距离和初始方位角。 */
struct MapDistanceBearing
{
    double distanceMeters = 0.0; ///< 地表距离，单位为米。
    double bearingDegrees = 0.0; ///< 初始方位角，正北为 0 度并顺时针递增。
};

/** @brief 滚轮缩放锚点策略。 */
enum class MapZoomAnchor
{
    ViewCenter,   ///< 以视口中心作为缩放锚点。
    MousePosition ///< 以鼠标位置作为缩放锚点。
};

/**
 * @brief 地图工具唯一的运行时类型，内置取值见 MapToolTypes。
 * @note 自定义工具使用 Custom + n 并由调用方保证类型值不冲突；通过 MapWidget::setTool() 注入实例。
 */
using MapToolType = qint32;

/** @brief 地图工具类型常量。 */
namespace MapToolTypes
{
/** @brief 平移地图工具。 */
inline constexpr MapToolType Pan = 0;
/** @brief 框选查询工具。 */
inline constexpr MapToolType QueryBox = 1;
/** @brief 两点测量工具。 */
inline constexpr MapToolType Measure = 3;
/** @brief 电子方位线工具。 */
inline constexpr MapToolType Ebl = 4;
/** @brief 航线创建工具运行时类型。 */
inline constexpr MapToolType RouteCreate = 5;
/** @brief 航线编辑工具运行时类型。 */
inline constexpr MapToolType RouteEdit = 6;
/** @brief 自定义地图工具起始值，自定义工具使用 Custom + n。 */
inline constexpr MapToolType Custom = 10000;
}
