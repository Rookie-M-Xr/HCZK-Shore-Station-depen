#pragma once

#include "sdk/EncViewerSdkGlobal.h"

#include <QMap>
#include <QPointer>
#include <QSet>
#include <QWidget>

class QCheckBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QTextBrowser;
class MapWidget;
struct EncFeature;
struct EncCellInfo;

/**
 * @brief 管理 ENC Cell 列表、可见性和右键菜单的独立控件。
 */
class ENCVIEWER_SDK_EXPORT CellListWidget : public QWidget
{
    Q_OBJECT

public:
    /** @brief 自动加载并订阅 Cell 列表；mapWidget 为借用，可为空，parent 管理控件生命周期。仅在 GUI 线程使用。 */
    explicit CellListWidget(MapWidget *mapWidget, QWidget *parent = nullptr);

private:
    friend class ChartUiController;
    friend class EncChartWorkspaceController;
    friend class AppUserSettings;
    friend class ChartSettingsPage;
    friend class SdkWidgetBoundaryTestAccess;

    /** @brief 使用 Cell 元数据和已保存可见性重建列表。 */
    void setCells(const QList<EncCellInfo> &cells, const QMap<qint64, bool> &savedVisibility);

    /** @brief 新增或更新一个已进入 ready 状态的 Cell。 */
    void upsertCell(const EncCellInfo &cell);

    /** @brief 返回当前 Cell 可见性。 */
    QMap<qint64, bool> cellVisibility() const;

    /** @brief 返回当前选中的 Cell ID。 */
    QList<qint64> selectedCellIds() const;

    /** @brief 移除指定 Cell 的本地可见性记录。 */
    void removeCells(const QList<qint64> &cellIds);

    /** @brief 设置指定 Cell 的可见性。 */
    void setCellVisible(qint64 cellId, bool visible);

    /** @brief 处理地图中指定 Cell 的右键菜单。 */
    void showChartCellContextMenu(qint64 cellId, const QPoint &globalPos);

private:
    /** @brief 用户修改了 Cell 可见性。 */
    Q_SIGNAL void cellVisibilityChanged(const QMap<qint64, bool> &visibility);

    /** @brief Cell 对象查询完成。 */
    Q_SIGNAL void featuresReady(const QList<EncFeature> &features, const QString &emptyMessage);

    /** @brief 请求向宿主显示状态消息。 */
    Q_SIGNAL void messageRequested(const QString &message);

    /** @brief 已移除一个或多个 Cell。 */
    Q_SIGNAL void cellsRemoved();

private slots:
    /** @brief 应用总复选框状态。 */
    void applyMasterCheckState(Qt::CheckState state);

    /** @brief 响应单个 Cell 勾选变化。 */
    void handleItemChanged(QListWidgetItem *item);

    /** @brief 显示 Cell 右键菜单。 */
    void showContextMenu(const QPoint &pos);

    /** @brief 响应后台批量移除完成。 */
    void handleCellsRemovalFinished(const QList<qint64> &cellIds,
                                    bool success,
                                    const QString &errorMessage);

private:
    /** @brief 根据当前列表更新总复选框。 */
    void updateMasterCheckState();

    /** @brief 更新已加载海图数量显示。 */
    void updateLoadedCount();

    /** @brief 刷新当前焦点图幅的信息；无有效选择时显示提示。 */
    void updateCellInformation();

    /** @brief 查询指定 Cell 的对象。 */
    void listCellFeatures(qint64 cellId, const QString &cellText);

    /** @brief 删除当前选中 Cell 及其本地缓存。 */
    void removeSelectedCells();

    /** @brief 按文件名、Cell 名称或路径过滤列表。 */
    void applySearchFilter(const QString &text);

    QPointer<MapWidget> m_mapWidget;    ///< 借用地图，地图销毁后自动置空。
    QCheckBox *m_masterCheck = nullptr; ///< Cell 总复选框。
    QLabel *m_loadedCountLabel = nullptr; ///< 已加载海图数量显示。
    QLineEdit *m_searchEdit = nullptr;  ///< 海图文件搜索框。
    QListWidget *m_listWidget = nullptr; ///< Cell 列表控件。
    QTextBrowser *m_information = nullptr; ///< 右侧可复制的图幅元数据。
    QMap<qint64, bool> m_visibility;    ///< Cell ID 到可见性的映射。
    QSet<qint64> m_lastPartialCellIds;  ///< 半选状态恢复集合。
    bool m_updating = false;            ///< 是否正在程序化更新。
    bool m_removalInProgress = false;   ///< 是否正在后台移除 Cell。
};
