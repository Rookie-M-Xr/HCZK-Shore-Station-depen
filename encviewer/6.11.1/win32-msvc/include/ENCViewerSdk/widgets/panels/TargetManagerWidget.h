#pragma once

#include "sdk/EncViewerSdkGlobal.h"
#include <QWidget>
#include <memory>

class RealtimeDataManager;

/** @brief AIS／ARPA 实时目标管理界面，支持独立窗口和嵌入容器。
 * @note 只在 GUI 线程使用；读取管理器快照，不修改接收的业务数据。 */
class ENCVIEWER_SDK_EXPORT TargetManagerWidget final : public QWidget
{
    Q_OBJECT
public:
    /** @brief 展示指定管理器的 AIS／ARPA 目标及轨迹操作。
     * @param manager 借用指针，须与控件同线程；为空或销毁后禁用操作。
     * @param parent Qt 父控件；为空时作为独立窗口。
     * @note 无地图时可查看数据，定位及轨迹操作禁用。 */
    explicit TargetManagerWidget(RealtimeDataManager *manager, QWidget *parent = nullptr);
    /** @brief 销毁窗体，不删除管理器、目标或轨迹。 */
    ~TargetManagerWidget() override;
private:
    struct Private;
    std::unique_ptr<Private> d;
};
