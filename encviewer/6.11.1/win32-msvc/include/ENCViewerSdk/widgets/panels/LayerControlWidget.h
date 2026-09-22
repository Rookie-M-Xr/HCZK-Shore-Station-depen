#pragma once

#include "sdk/EncViewerSdkGlobal.h"
#include "s52/S52SdkTypes.h"

#include <QMap>
#include <QSet>
#include <QWidget>

class QCheckBox;
class MapWidget;
struct EncObjectClassInfo;
struct EncFeature;
enum class EncPrimitive;
class QLineEdit;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;

/**
 * @brief 管理 ENC 图层树、搜索过滤和图层可见性的独立控件。
 */
class ENCVIEWER_SDK_EXPORT LayerControlWidget : public QWidget
{
    Q_OBJECT

public:
    /** @brief 自动加载图层列表；mapWidget 为借用，可为空，parent 管理控件生命周期。 */
    explicit LayerControlWidget(MapWidget *mapWidget = nullptr, QWidget *parent = nullptr);

private:
    friend class ChartUiController;
    friend class EncChartWorkspaceController;
    friend class AppUserSettings;
    friend class ChartSettingsPage;
    friend class SdkWidgetBoundaryTestAccess;

    /** @brief 重建图层树。 */
    void setObjectClassInfos(const QList<EncObjectClassInfo> &infos,
                             const QMap<QString, bool> &savedVisibility,
                             S52DisplayCategory displayCategory);

    /** @brief 返回当前图层可见性。 */
    QMap<QString, bool> layerVisibility() const;

    /** @brief 同步并返回可应用到地图的图层可见性。 */
    QMap<QString, bool> syncLayerChecks();

    /** @brief 按 S-52 显示类别套用预设。 */
    int applyDisplayCategoryPreset(S52DisplayCategory displayCategory);

    /** @brief 设置所有非强制图层可见性。 */
    void setAllLayersVisible(bool visible);

    /** @brief 设置指定几何类型图层可见性。 */
    void setPrimitiveLayersVisible(EncPrimitive prim, bool visible);

    /** @brief 刷新图层显示文本。 */
    void refreshDisplayTexts();

    /** @brief 设置图层名称显示组成。 */
    void setLayerNameDisplayOptions(bool acronym, bool objectClass, bool chineseName);

    /** @brief 设置基础面图层是否强制可见。 */
    void setForceBaseLayersVisible(bool enabled);

private:
    /** @brief 图层可见性发生变化。 */
    Q_SIGNAL void layerVisibilityChanged(const QMap<QString, bool> &visibility);

    /** @brief 用户开始自定义图层显示类别。 */
    Q_SIGNAL void customDisplayCategoryRequested();

    /** @brief 图层对象查询完成。 */
    Q_SIGNAL void featuresReady(const QList<EncFeature> &features, const QString &emptyMessage);

    /** @brief 请求向宿主显示状态消息。 */
    Q_SIGNAL void messageRequested(const QString &message);

private slots:
    /** @brief 响应图层树勾选变化。 */
    void handleItemChanged(QTreeWidgetItem *item, int column);

    /** @brief 应用总复选框状态。 */
    void applyMasterCheckState(Qt::CheckState state);

    /** @brief 过滤图层树。 */
    void applyFilter();

    /** @brief 显示图层右键菜单。 */
    void showContextMenu(const QPoint &pos);

private:
    /** @brief 更新分组和总复选框状态。 */
    void updateMasterChecks();

    /** @brief 更新总复选框状态。 */
    void updateMasterCheckState();

    /** @brief 更新点、线、面图层复选框状态。 */
    void updatePrimitiveCheckStates();

    /** @brief 折叠或展开全部 Acronym 节点。 */
    void setAllGroupsExpanded(bool expanded);

    /** @brief 返回图层显示文本。 */
    QString layerDisplayText(const QString &objClass) const;

    /** @brief 返回 Acronym 节点的说明文本。 */
    QString objectClassToolTip(const QString &objClass,
                               const QList<EncObjectClassInfo> &primitiveInfos) const;

    /** @brief 返回 Primitive 节点的说明文本。 */
    QString primitiveToolTip(const QString &objClass,
                             const EncObjectClassInfo &info) const;

    /** @brief 查询指定图层的对象。 */
    void listLayerFeatures(const QString &objClass, EncPrimitive prim, const QString &displayText);

    QLineEdit *m_searchEdit = nullptr;             ///< 图层搜索输入框。
    QCheckBox *m_masterCheck = nullptr;            ///< 图层总复选框。
    QCheckBox *m_pointCheck = nullptr;              ///< 点图层总复选框。
    QCheckBox *m_lineCheck = nullptr;               ///< 线图层总复选框。
    QCheckBox *m_areaCheck = nullptr;               ///< 面图层总复选框。
    QTabWidget *m_levelTabs = nullptr;             ///< S-57 显示级别标签页。
    QList<QTreeWidget *> m_levelTrees;             ///< 各显示级别的 Acronym 树。
    QList<QTreeWidgetItem *> m_layerItems;         ///< Primitive 叶子节点。
    QList<QTreeWidgetItem *> m_groupItems;         ///< Acronym 分组节点。
    QMap<QString, bool> m_visibility;              ///< 图层 key 到可见性的映射。
    QSet<QString> m_lastPartialLayerKeys;          ///< 半选状态恢复集合。
    bool m_updating = false;                       ///< 是否正在程序化更新。
    bool m_searchActive = false;                   ///< 图层搜索是否激活。
    QMap<QTreeWidget *, int> m_searchRestoreScrollValues; ///< 各标签页搜索前的滚动位置。
    bool m_showLayerNameAcronym = true;            ///< 是否显示 Acronym。
    bool m_showLayerNameObjectClass = true;        ///< 是否显示 ObjectClass。
    bool m_showLayerNameChinese = true;            ///< 是否显示中文名。
    bool m_forceBaseLayersVisible = true;          ///< 是否强制基础面图层可见。
};
