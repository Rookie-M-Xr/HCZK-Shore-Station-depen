#pragma once

#include "map/core/MapSdkTypes.h"
#include "sdk/EncViewerSdkGlobal.h"

#include <QPointF>
#include <QSize>
#include <QSizeF>
#include <optional>

class MapWidget;

/**
 * @brief 提供面向 SDK 的地图坐标与距离换算接口。
 *
 * @note 屏幕坐标及长度均为控件局部逻辑像素，不是桌面坐标或物理像素。
 * @note 控件相关方法仅限 GUI 线程；空指针选择第一个已注册控件，多地图场景建议显式指定。
 * @note optional 方法在无控件、非 GUI 线程、无效视口/输入或转换失败时返回 nullopt；合法零值仍有值。
 * @note 距离和方位采用球面近似；纯经纬度方法不依赖控件，可在后台线程使用。
 */
class ENCVIEWER_SDK_EXPORT MapTransform
{
public:
    /**
     * @brief 将 WGS84 经纬度转换为指定地图控件的屏幕坐标。
     * @param lonLat WGS84 经纬度。
     * @param mapWidget 地图控件；为空时使用第一个已注册控件。
     * @return 屏幕坐标；失败时返回 nullopt。
     */
    static std::optional<QPointF> lonLatToScreen(const QPointF &lonLat, const MapWidget *mapWidget = nullptr);

    /**
     * @brief 将屏幕坐标转换为指定地图控件中的 WGS84 经纬度。
     * @param screenPoint 屏幕坐标。
     * @param mapWidget 地图控件；为空时使用第一个已注册控件。
     * @return WGS84 经纬度；失败时返回 nullopt。
     */
    static std::optional<QPointF> screenToLonLat(const QPointF &screenPoint, const MapWidget *mapWidget = nullptr);

    /**
     * @brief 计算两个 WGS84 经纬度点之间的近似地表距离。
     * @return 距离，单位为米。
     */
    static double distanceMeters(const QPointF &firstLonLat, const QPointF &secondLonLat);

    /**
     * @brief 计算从起点指向终点的初始方位角。
     * @return 正北为 0 度、顺时针递增的角度。
     */
    static double bearingDegrees(const QPointF &startLonLat, const QPointF &endLonLat);

    /**
     * @brief 根据起点、距离和方位角计算 WGS84 终点。
     * @param startLonLat 起点经纬度。
     * @param distanceMetersValue 距离，单位为米。
     * @param bearingDegreesValue 方位角，正北为 0 度并顺时针递增。
     */
    static QPointF destinationPoint(const QPointF &startLonLat,
                                    double distanceMetersValue,
                                    double bearingDegreesValue);

    /**
     * @brief 同时计算两个 WGS84 点之间的距离和初始方位角。
     */
    static MapDistanceBearing distanceAndBearing(const QPointF &startLonLat, const QPointF &endLonLat);

    /**
     * @brief 计算地理长度在指定地图控件中的屏幕像素长度。
     * @param startLonLat 起点经纬度。
     * @param distanceMetersValue 非负有限地理长度，单位为米。
     * @param bearingDegreesValue 地理方位角。
     * @param mapWidget 地图控件；为空时使用第一个已注册控件。
     */
    static std::optional<double> screenLengthForMeters(const QPointF &startLonLat,
                                        double distanceMetersValue,
                                        double bearingDegreesValue,
                                        const MapWidget *mapWidget = nullptr);

    /**
     * @brief 计算屏幕长度在指定地图控件中对应的地理长度。
     * @param screenStart 屏幕起点。
     * @param pixelLength 非负有限屏幕长度，单位为逻辑像素。
     * @param screenBearingDegrees 屏幕方向，正上方为 0 度并顺时针递增。
     * @param mapWidget 地图控件；为空时使用第一个已注册控件。
     */
    static std::optional<double> metersForScreenLength(const QPointF &screenStart,
                                        double pixelLength,
                                        double screenBearingDegrees = 90.0,
                                        const MapWidget *mapWidget = nullptr);

    /**
     * @brief 返回指定屏幕位置横向和纵向每像素对应的实际长度。
     * @param screenPoint 屏幕采样位置。
     * @param mapWidget 地图控件；为空时使用第一个已注册控件。
     * @return width 为横向米/像素，height 为纵向米/像素。
     */
    static std::optional<QSizeF> metersPerPixel(const QPointF &screenPoint, const MapWidget *mapWidget = nullptr);

    /**
     * @brief 返回当前视口中心点横向每像素代表的米数。
     * @param mapWidget 地图控件；为空时使用第一个已注册控件。
     */
    static std::optional<double> metersPerPixel(const MapWidget *mapWidget = nullptr);

    /**
     * @brief 返回当前视口中心点横向每米代表的像素数。
     * @param mapWidget 地图控件；为空时使用第一个已注册控件。
     */
    static std::optional<double> pixelsPerMeter(const MapWidget *mapWidget = nullptr);

private:
    MapTransform() = delete;

    /**
     * @brief 计算指定横向屏幕像素长度对应的近似地表距离。
     * @param screenStart 屏幕起点。
     * @param pixelLength 横向屏幕长度，单位为像素。
     * @param state 地图视图状态。
     * @param viewportSize 视口尺寸。
     * @return 地表距离，单位为米。
     */
    static double horizontalDistanceMeters(const QPointF &screenStart,
                                           double pixelLength,
                                           const MapViewState &state,
                                           const QSize &viewportSize);

    /**
     * @brief 返回当前视口中心点横向和纵向每像素对应的近似长度。
     * @note 返回值可在当前视口内快速复用，远离视口中心时存在纬度比例误差。
     */
    static QSizeF approximateMetersPerPixel(const MapWidget *mapWidget = nullptr);

    /**
     * @brief 使用视口中心固定比例估算屏幕长度对应的地理长度。
     */
    static double approximateMetersForScreenLength(double pixelLength,
                                                   double screenBearingDegrees = 90.0,
                                                   const MapWidget *mapWidget = nullptr);

    /**
     * @brief 使用视口中心固定比例估算地理长度对应的屏幕长度。
     */
    static double approximateScreenLengthForMeters(double distanceMetersValue,
                                                   double screenBearingDegrees = 90.0,
                                                   const MapWidget *mapWidget = nullptr);
};
