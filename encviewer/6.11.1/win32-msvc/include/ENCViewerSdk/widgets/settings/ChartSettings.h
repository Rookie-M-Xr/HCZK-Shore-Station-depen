#pragma once
#include "sdk/EncViewerSdkGlobal.h"
#include "enc/model/EncCoordinateReferenceSystem.h"
#include "map/component/ChartOrientationIndicatorTypes.h"
#include "map/core/MapSdkTypes.h"
#include "s52/S52SdkTypes.h"
#include <QMap>
class MapWidget;

/**
 * @brief 全局海图显示设置。仅 GUI 线程修改，修改后显式 applyTo，不自动持久化。
 * 水深值限定为 [0, 12000] 米；浅水分界、安全等深线、深水分界始终非递减。
 */
class ENCVIEWER_SDK_EXPORT ChartSettings final
{
public:
    /** @brief 返回进程内唯一的设置入口。 */
    static ChartSettings &instance();
    /** @brief 应用到地图并刷新，空指针忽略；MSAA 受地图 GL 上下文及硬件支持限制。 */
    void applyTo(MapWidget *map) const;
    /** @brief 恢复全部默认设置，保留样式注册项；不自动保存或刷新。 */
    void resetToDefaults();
    /** @brief 返回全局绘制 CRS。 */
    EncCoordinateReferenceSystem coordinateReferenceSystem() const;
    /** @brief 设置全局绘制 CRS，无效值忽略；影响全局海图数据。 */
    void setCoordinateReferenceSystem(const EncCoordinateReferenceSystem &crs);
    /** @brief 返回色表。 */
    S52ColorTableMode colorTable() const;
    /** @brief 设置色表；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setColorTable(S52ColorTableMode value);
    /** @brief 返回显示类别。 */
    S52DisplayCategory displayCategory() const;
    /** @brief 设置显示类别；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setDisplayCategory(S52DisplayCategory value);
    /** @brief 返回安全水深（米）。 */
    double safetyDepth() const;
    /** @brief 设置安全水深（米）；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setSafetyDepth(double value);
    /** @brief 返回安全等深线（米）。 */
    double safetyContour() const;
    /** @brief 设置安全等深线（米）；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setSafetyContour(double value);
    /** @brief 返回浅水分界（米）。 */
    double shallowDepth() const;
    /** @brief 设置浅水分界（米）；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setShallowDepth(double value);
    /** @brief 返回深水分界（米）。 */
    double deepDepth() const;
    /** @brief 设置深水分界（米）；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setDeepDepth(double value);
    /** @brief 返回双色水域。 */
    bool twoShades() const;
    /** @brief 设置双色水域；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setTwoShades(bool value);
    /** @brief 返回点符号样式。 */
    SymbolStyle symbolStyle() const;
    /** @brief 设置点符号样式；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setSymbolStyle(SymbolStyle value);
    /** @brief 返回区域边界样式。 */
    BoundaryStyle boundaryStyle() const;
    /** @brief 设置区域边界样式；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setBoundaryStyle(BoundaryStyle value);
    /** @brief 返回对象文字显示。 */
    bool showText() const;
    /** @brief 设置对象文字显示；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setShowText(bool value);
    /** @brief 返回名称显示模式。 */
    NameDisplayMode nameDisplayMode() const;
    /** @brief 设置名称显示模式；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setNameDisplayMode(NameDisplayMode value);
    /** @brief 返回文字字号（6–24 point）。 */
    int chartTextPointSize() const;
    /** @brief 设置文字字号（6–24 point）；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setChartTextPointSize(int value);
    /** @brief 返回MSAA 采样数（0–16）。 */
    int msaaSamples() const;
    /** @brief 设置MSAA 采样数（0–16）；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setMsaaSamples(int value);
    /** @brief 返回比例尺叠加。 */
    bool showScaleOverlay() const;
    /** @brief 设置比例尺叠加；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setShowScaleOverlay(bool value);
    /** @brief 返回鼠标坐标叠加。 */
    bool showMouseLonLatOverlay() const;
    /** @brief 设置鼠标坐标叠加；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setShowMouseLonLatOverlay(bool value);
    /** @brief 返回坐标系叠加。 */
    bool showProjectionOverlay() const;
    /** @brief 设置坐标系叠加；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setShowProjectionOverlay(bool value);
    /** @brief 返回方向指示器显示。 */
    bool orientationIndicatorVisible() const;
    /** @brief 设置方向指示器显示；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setOrientationIndicatorVisible(bool value);
    /** @brief 返回方向指示器位置。 */
    ChartOrientationIndicatorPosition orientationIndicatorPosition() const;
    /** @brief 设置方向指示器位置；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setOrientationIndicatorPosition(ChartOrientationIndicatorPosition value);
    /** @brief 返回方向指示器大小（80–320 逻辑像素）。 */
    int orientationIndicatorSize() const;
    /** @brief 设置方向指示器大小（80–320 逻辑像素）；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setOrientationIndicatorSize(int value);
    /** @brief 返回缩放锚点。 */
    MapZoomAnchor zoomAnchor() const;
    /** @brief 设置缩放锚点；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setZoomAnchor(MapZoomAnchor value);
    /** @brief 返回抗锯齿。 */
    bool antialiasingEnabled() const;
    /** @brief 设置抗锯齿；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setAntialiasingEnabled(bool value);
    /** @brief 返回Cell 范围显示。 */
    bool cellCoverageOverlayVisible() const;
    /** @brief 设置Cell 范围显示；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setCellCoverageOverlayVisible(bool value);
    /** @brief 返回基础图层强制显示。 */
    bool forceBaseLayersVisible() const;
    /** @brief 设置基础图层强制显示；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setForceBaseLayersVisible(bool value);
    /** @brief 返回图层可见性快照。 */
    QMap<QString, bool> layerVisibility() const;
    /** @brief 设置图层可见性快照；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setLayerVisibility(const QMap<QString, bool> & value);
    /** @brief 返回Cell 可见性快照。 */
    QMap<qint64, bool> cellVisibility() const;
    /** @brief 设置Cell 可见性快照；数值限制在允许范围，非法枚举和非有限水深忽略。 */
    void setCellVisibility(const QMap<qint64, bool> & value);
private:
    ChartSettings() = default;
    ChartSettings(const ChartSettings &) = delete;
    ChartSettings &operator=(const ChartSettings &) = delete;
};
