#pragma once

#include "EncViewerSdkGlobal.h"
#include "widgets/settings/ChartSettingsPages.h"

#include <QObject>
#include <QString>
#include <Qt>

#include <memory>

class CellListWidget;
class FeatureInfoWidget;
class LayerControlWidget;
class TextLogWidget;
class ViewportObjectWidget;
class QWidget;
class MapWidget;

/**
 * @brief 为一个地图控件创建并管理配套海图界面组件。
 *
 * 业务界面均为可独立展示或嵌入的 QWidget，停靠容器由宿主自行管理。
 * @note 全部接口在 GUI 线程调用。返回指针均为借用，宿主/地图销毁或 closeAllUi 后失效。
 * closeAllUi 后可重新创建 UI，但不会重新初始化 SDK。
 */
class ENCVIEWER_SDK_EXPORT ChartUiController final : public QObject
{
    Q_OBJECT

signals:
    /** @brief 界面修改了需要持久化的海图设置。 */
    void settingsChanged();

    /** @brief 界面请求向宿主显示状态消息。 */
    void messageRequested(const QString &message);

public:
    /**
     * @brief 构造绑定指定地图控件的界面控制器。
     * @param mapWidget 界面组件操作的地图控件。
     * @param parent QObject 生命周期父对象。
     */
    explicit ChartUiController(MapWidget *mapWidget, QObject *parent = nullptr);

    /** @brief 销毁控制器管理的全部界面组件。 */
    ~ChartUiController() override;

    /**
     * @brief 初始化全部业务 Widget，但不创建 Dock 容器。
     * @param parent Widget 的初始父控件；为空时使用地图所在窗口。
     */
    void initializeWidgets(QWidget *parent = nullptr);

    /** @brief 返回 Cell 列表 Widget，未初始化时返回空。 */
    CellListWidget *cellListWidget() const;

    /** @brief 返回图层控制 Widget，未初始化时返回空。 */
    LayerControlWidget *layerControlWidget() const;

private:
    /** @brief 返回对象属性 Widget，未初始化时返回空。 */
    FeatureInfoWidget *featureInfoWidget() const;

    /** @brief 返回当前视口对象统计 Widget，未初始化时返回空。 */
    ViewportObjectWidget *viewportObjectWidget() const;

public:
    /** @brief 返回导入日志 Widget，未初始化时返回空。 */
    TextLogWidget *importLogWidget() const;

    /** @brief 返回运行日志 Widget，未初始化时返回空。 */
    TextLogWidget *runtimeLogWidget() const;

    /**
     * @brief 显示海图文件控件，重复调用复用已有实例。
     * @param parent 非空时嵌入该宿主；为空时显示为独立工具窗口。
     * @return 借用的 QWidget 指针；地图已销毁时返回空。
     */
    QWidget *showCellList(QWidget *parent = nullptr);

    /**
     * @brief 显示图层控件，重复调用复用已有实例。
     * @param parent 非空时嵌入该宿主；为空时显示为独立工具窗口。
     * @return 借用的 QWidget 指针；地图已销毁时返回空。
     */
    QWidget *showLayerControl(QWidget *parent = nullptr);

    /**
     * @brief 显示对象属性控件，重复调用复用已有实例。
     * @param parent 非空时嵌入该宿主；为空时显示为独立工具窗口。
     * @return 借用的 QWidget 指针；地图已销毁时返回空。
     */
    QWidget *showFeatureInfo(QWidget *parent = nullptr);

    /**
     * @brief 显示当前视口绘制对象控件，重复调用复用已有实例。
     * @param parent 非空时嵌入该宿主；为空时显示为独立工具窗口。
     * @return 借用的 QWidget 指针；地图已销毁时返回空。
     */
    QWidget *showViewportObject(QWidget *parent = nullptr);

    /**
     * @brief 显示Import Log控件，重复调用复用已有实例。
     * @param parent 非空时嵌入该宿主；为空时显示为独立工具窗口。
     * @return 借用的 QWidget 指针；地图已销毁时返回空。
     */
    QWidget *showImportLog(QWidget *parent = nullptr);

    /**
     * @brief 显示Runtime Log控件，重复调用复用已有实例。
     * @param parent 非空时嵌入该宿主；为空时显示为独立工具窗口。
     * @return 借用的 QWidget 指针；地图已销毁时返回空。
     */
    QWidget *showRuntimeLog(QWidget *parent = nullptr);

    /**
     * @brief 显示即改即用的海图设置 Widget。
     * @param parent 设置 Widget 的宿主；为空时使用地图控件所在窗口。
     */
    QWidget *showChartSettings(QWidget *parent = nullptr);

    /** @brief 独立显示“交互”页；返回借用指针，closeAllUi 后失效。 */
    ChartDisplaySettingsWidget *showDisplaySettings(QWidget *parent = nullptr);
    /** @brief 独立显示“显示”页；返回借用指针，closeAllUi 后失效。 */
    ChartS52SettingsWidget *showS52Settings(QWidget *parent = nullptr);
    /** @brief 独立显示工具与物标样式页；返回借用指针，closeAllUi 后失效。 */
    ChartStyleSettingsWidget *showStyleSettings(QWidget *parent = nullptr);
    /** @brief 独立显示 LOD 与比例尺页；返回借用指针，closeAllUi 后失效。 */
    ChartLodSettingsWidget *showLodSettings(QWidget *parent = nullptr);
    /** @brief 独立显示缓存与调试页；返回借用指针，closeAllUi 后失效。 */
    ChartCacheSettingsWidget *showCacheSettings(QWidget *parent = nullptr);

    /**
     * @brief 打开比例尺隐藏图层编辑 Dialog。
     * @return 用户确认并保存修改时返回 true。
     */
    bool editScaleHiddenLayers(QWidget *parent = nullptr);

    /** @brief 返回内部设置 Widget 的 Qt 基类指针。 */
    QWidget *settingsWidget() const;

    /** @brief 从缓存和当前设置刷新 Cell、图层及地图可见性。 */
    void refresh();

    /** @brief 向导入日志追加消息。 */
    void appendImportLogMessage(const QString &message);

    /** @brief 向运行日志追加消息。 */
    void appendRuntimeLogMessage(const QString &message);

    /** @brief 隐藏控制器管理的全部业务控件与设置页，不隐藏宿主容器。 */
    void hideAllWidgets();

    /** @brief 关闭并销毁控制器管理的全部界面组件。 */
    void closeAllUi();

private:
    class Private;
    std::unique_ptr<Private> d; ///< 界面组件及内部业务连接。
};
