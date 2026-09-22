#pragma once

#include "sdk/EncViewerSdkGlobal.h"

#include <QString>
#include <QStringList>
#include <QVector>
#include <QWidget>

class QComboBox;
class QPlainTextEdit;

/**
 * @brief 显示追加型文本日志的独立控件。
 */
class ENCVIEWER_SDK_EXPORT TextLogWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造日志控件。
     * @param importLog 是否自动接收 ENC 导入日志，默认开启。运行日志由 ChartUiController 管理并转发。
     * @param parent 父控件。
     * @note 仅在 GUI 线程使用，最多保留 1000 条完整记录，支持独立 show 或嵌入父控件。
     *       控件不可见时丢弃新日志，重新显示后仅接收后续日志，不补放隐藏期间的记录。
     */
    explicit TextLogWidget(bool importLog = true, QWidget *parent = nullptr);

private:
    friend class ChartUiController;
    friend class EncChartWorkspaceController;
    friend class AppUserSettings;
    friend class ChartSettingsPage;
    friend class SdkWidgetBoundaryTestAccess;

    /** @brief 追加一行带时间戳的日志文本。 */
    void appendLogMessage(const QString &message);

    /** @brief 启用或关闭按方括号标签筛选日志。 */
    void setTagFilterEnabled(bool enabled);

    /** @brief 清空全部日志文本。 */
    void clear();

    /** @brief 返回当前日志文本。 */
    QString text() const;

private:
    /** @brief 内存中保留的一条完整日志记录。 */
    struct LogEntry
    {
        QString timestamp; ///< 追加时生成的时间戳。
        QString message;   ///< 原始日志正文。
        QStringList tags;  ///< 从正文提取的方括号标签。
    };

    /** @brief 提取日志正文中的方括号标签。 */
    static QStringList extractTags(const QString &message);
    /** @brief 判断日志是否匹配当前标签筛选。 */
    bool matchesCurrentTagFilter(const LogEntry &entry) const;
    /** @brief 向文本框追加一条已通过筛选的日志。 */
    void appendVisibleEntry(const LogEntry &entry);
    /** @brief 按当前筛选条件重建文本框。 */
    void rebuildVisibleEntries();
    /** @brief 将新标签加入筛选列表。 */
    void addTagFilter(const QString &tag);

    QComboBox *m_tagFilter = nullptr; ///< 标签筛选下拉框。
    QPlainTextEdit *m_textEdit = nullptr; ///< 日志文本框。
    QVector<LogEntry> m_entries; ///< 有上限的内存日志记录。
    bool m_tagFilterEnabled = false; ///< 是否显示并启用标签筛选。
};
