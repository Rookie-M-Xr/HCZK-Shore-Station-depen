#pragma once
#include "sdk/EncViewerSdkGlobal.h"
#include <QWidget>
class MapWidget;
class LayerControlWidget;

/** @brief 组合五个独立设置页的标签窗口；仅 GUI 线程使用，支持直接 show 和嵌入。 */
class ENCVIEWER_SDK_EXPORT ChartSettingsWidget final : public QWidget
{
    Q_OBJECT
signals:
    /** @brief 设置已应用并持久化。 */
    void settingsApplied();

public:
    /** @brief 绑定地图、可选图层控件及宿主，不持有地图所有权。 */
    explicit ChartSettingsWidget(MapWidget *map = nullptr, LayerControlWidget *layers = nullptr, QWidget *parent = nullptr);
    /** @brief 将当前设置应用到绑定的地图和图层控件。 */
    void applyCurrentSettings();
};
