#pragma once
#include "sdk/EncViewerSdkGlobal.h"
#include <QWidget>
class MapWidget;
class LayerControlWidget;

/** @brief 交互设置页，可独立 show 或嵌入容器；仅 GUI 线程使用。 */
class ENCVIEWER_SDK_EXPORT ChartDisplaySettingsWidget final : public QWidget
{
    Q_OBJECT
signals:
    /** @brief 本页设置已应用并持久化。 */
    void settingsApplied();

public:
    /** @brief 绑定地图及可选图层控件；parent 为空时独立显示，地图销毁后不再操作地图。 */
    explicit ChartDisplaySettingsWidget(MapWidget *map = nullptr, LayerControlWidget *layers = nullptr, QWidget *parent = nullptr);
};

/** @brief 显示设置页，可独立 show 或嵌入容器；仅 GUI 线程使用。 */
class ENCVIEWER_SDK_EXPORT ChartS52SettingsWidget final : public QWidget
{
    Q_OBJECT
signals:
    /** @brief 本页设置已应用并持久化。 */
    void settingsApplied();

public:
    /** @brief 绑定地图及可选图层控件；parent 为空时独立显示，地图销毁后不再操作地图。 */
    explicit ChartS52SettingsWidget(MapWidget *map = nullptr, LayerControlWidget *layers = nullptr, QWidget *parent = nullptr);
};

/** @brief 工具与物标样式设置页，可独立 show 或嵌入容器；仅 GUI 线程使用。 */
class ENCVIEWER_SDK_EXPORT ChartStyleSettingsWidget final : public QWidget
{
    Q_OBJECT
signals:
    /** @brief 本页设置已应用并持久化。 */
    void settingsApplied();

public:
    /** @brief 绑定地图及可选图层控件；parent 为空时独立显示，地图销毁后不再操作地图。 */
    explicit ChartStyleSettingsWidget(MapWidget *map = nullptr, LayerControlWidget *layers = nullptr, QWidget *parent = nullptr);
};

/** @brief LOD 与比例尺设置页，可独立 show 或嵌入容器；仅 GUI 线程使用。 */
class ENCVIEWER_SDK_EXPORT ChartLodSettingsWidget final : public QWidget
{
    Q_OBJECT
signals:
    /** @brief 本页设置已应用并持久化。 */
    void settingsApplied();

public:
    /** @brief 绑定地图及可选图层控件；parent 为空时独立显示，地图销毁后不再操作地图。 */
    explicit ChartLodSettingsWidget(MapWidget *map = nullptr, LayerControlWidget *layers = nullptr, QWidget *parent = nullptr);
};

/** @brief 缓存与调试设置页，可独立 show 或嵌入容器；仅 GUI 线程使用。 */
class ENCVIEWER_SDK_EXPORT ChartCacheSettingsWidget final : public QWidget
{
    Q_OBJECT
signals:
    /** @brief 本页设置已应用并持久化。 */
    void settingsApplied();

public:
    /** @brief 绑定地图及可选图层控件；parent 为空时独立显示，地图销毁后不再操作地图。 */
    explicit ChartCacheSettingsWidget(MapWidget *map = nullptr, LayerControlWidget *layers = nullptr, QWidget *parent = nullptr);
};
