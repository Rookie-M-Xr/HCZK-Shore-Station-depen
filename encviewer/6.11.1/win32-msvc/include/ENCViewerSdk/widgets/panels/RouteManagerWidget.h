#pragma once

#include "sdk/EncViewerSdkGlobal.h"
#include <QWidget>
#include <memory>

class StaticDataManager;

/** @brief 航线列表和航点编辑控件，可独立显示或嵌入容器。
 * @note 编辑副本通过保存按钮提交；只在 GUI 线程使用。 */
class ENCVIEWER_SDK_EXPORT RouteManagerWidget final : public QWidget
{
    Q_OBJECT
public:
    /** @brief 绑定现有静态数据管理器并显示其已加载航线。
     * @param manager 借用指针，必须与控件同线程；为空或销毁后禁用操作。
     * @param parent Qt 父控件，为空时作为独立窗口。
     * @note 不隐式加载磁盘文件，用户可点击“重新加载”。 */
    explicit RouteManagerWidget(StaticDataManager *manager, QWidget *parent = nullptr);
    /** @brief 销毁编辑副本，不隐式保存；宿主直接销毁前应先调用 close() 处理未保存修改。 */
    ~RouteManagerWidget() override;
protected:
    /** @brief 关闭时处理未保存修改；嵌入容器时宿主应先调用本控件 close() 并检查结果。 */
    void closeEvent(QCloseEvent *event) override;
private:
    struct Private;
    std::unique_ptr<Private> d;
};
