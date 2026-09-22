#pragma once

#include "sdk/EncViewerSdkGlobal.h"
#include "data/RouteData.h"
#include <QObject>
#include <QSharedPointer>
#include <memory>
#include <optional>

class MapWidget;
class RouteItem;

/**
 * @brief 管理需要本地保存的业务数据，当前实现航线。
 * @note 所有操作在创建线程执行；绑定地图时必须在 GUI 线程。
 * 绑定地图后自动保存工具完成的创建和编辑；取消编辑不保存。析构不删除磁盘数据。
 * 每张地图仅绑定一个管理器处理工具事件；多个界面可共享该管理器。
 */
class ENCVIEWER_SDK_EXPORT StaticDataManager : public QObject
{
    Q_OBJECT
public:
    /** @brief 构造管理器，不自动加载。
     * @param dataDirectory 本地数据根目录；空值为程序旁 WorkFile/Data，不随 CWD 改变。
     * @param map 可空，借用地图；同步航线物标并订阅创建／编辑完成事件，自动保存到本地。
     * @param parent Qt 父对象，与 map 的生命周期无隐式绑定。 */
    explicit StaticDataManager(const QString &dataDirectory = {}, MapWidget *map = nullptr, QObject *parent = nullptr);
    /** @brief 移除本管理器的航线显示；不删除保存文件。 */
    ~StaticDataManager() override;
    /** @brief 返回规范化后的绝对数据根目录。 */
    QString dataDirectory() const;
    /** @brief 加载 Routes 中全部 JSON；任一文件损坏／版本不支持时返回 false，保留全部当前数据和显示。
     * @param error 可空，返回首个错误；缺少目录按空数据处理，不创建目录。 */
    bool loadRoutes(QString *error = nullptr);
    /** @brief 原子保存单条航线，成功后同步内存及显示，并回填 ID 和 UTC 时间。
     * @param routeData 待保存副本；失败不修改该副本或正式数据。
     * @param error 可空，返回校验或 I/O 错误；外部文件变更需重新 loadRoutes 后再保存。 */
    bool saveRoute(RouteData &routeData, QString *error = nullptr);
    /** @brief 保存并接管地图工具创建的航线，保留原 Item ID 和显隐状态。
     * @param item 必须是已注册到本管理器绑定地图的航线。
     * @param error 可空，返回校验或保存错误。
     * @return 保存成功返回 true；失败时原地图对象不变，允许重试。
     * @note 已接管的对象重复调用返回成功，不再次保存或创建业务记录。 */
    bool adoptRouteItem(const QSharedPointer<RouteItem> &item, QString *error = nullptr);
    /** @brief 将航线文件移入 Routes/Trash，成功后移除内存及显示。
     * @param id 航线业务 ID。
     * @param error 可空，返回失败原因。
     * @param recoveryPath 可空，成功时返回回收文件绝对路径，文件保留用于恢复。 */
    bool removeRoute(const QUuid &id, QString *error = nullptr, QString *recoveryPath = nullptr);
    /** @brief 返回正式航线副本；不存在时为空。 */
    std::optional<RouteData> route(const QUuid &id) const;
    /** @brief 返回正式航线副本列表，按业务 ID 排列。 */
    QList<RouteData> routes() const;
    /** @brief 返回绑定地图中的航线对象；无地图／未显示时为空，不转移独占所有权。 */
    QSharedPointer<RouteItem> routeItem(const QUuid &id) const;
    /** @brief 设置单条已加载航线显隐，不写入磁盘；未找到返回 false。 */
    bool setRouteVisible(const QUuid &id, bool visible);
signals:
    /** @brief 地图工具结果自动保存失败时发出；SDK 不弹窗，宿主负责提示。
     * @param error 失败原因。多航线逐条保存，已成功保存的结果不回滚；失败时保留原文件。 */
    void routePersistenceFailed(const QString &error);
    /** @brief 成功加载、保存、删除或改变显隐后发出，接收方可重新查询快照和物标。 */
    void routesChanged();
private:
    struct Private;
    std::unique_ptr<Private> d;
};
