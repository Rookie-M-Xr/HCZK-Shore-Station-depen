#pragma once

#include "sdk/EncViewerSdkGlobal.h"
#include "target/ShipData.h"
#include "target/AisData.h"
#include "target/ArpaData.h"
#include "map/item/ShipItem.h"
#include "map/item/MapItemManager.h"
#include <QFlags>
#include <QObject>
#include <memory>
#include <optional>
#include <tuple>

class MapWidget;
class AisItem;
class ArpaItem;

/** @brief 本次更新中完整有效的数据组；组内数据整体替换，未选中的组不变。 */
enum class RealtimeDataPart { Dynamic = 1, Static = 2, State = 4, All = 7 };
Q_DECLARE_FLAGS(RealtimeDataParts, RealtimeDataPart)
Q_DECLARE_OPERATORS_FOR_FLAGS(RealtimeDataParts)

/** @brief AIS／ARPA 来源主键；同源不同观察船和不同目标相互隔离。 */
struct ObservedTargetKey
{
    QString sourceId; ///< 非空来源标识，由宿主配置。
    quint32 observerMmsi = 0; ///< 所属船 MMSI，允许未知值 0。
    quint32 targetId = 0; ///< AIS 为目标 MMSI，ARPA 为航迹号。
    /** @brief 按来源、观察船和目标编号排序，供关联容器使用。 */
    bool operator<(const ObservedTargetKey &other) const noexcept
    { return std::tie(sourceId, observerMmsi, targetId) < std::tie(other.sourceId, other.observerMmsi, other.targetId); }
};

/**
 * @brief 接收已解码业务数据，维护目标快照、物标和轨迹。
 * @note 全部接口只在所属 GUI 线程调用；后台接入方通过 queued 调用提交值副本。
 * 动态组提交即声明经纬度有效；非有限值／越界位置被拒绝，0/0 作为合法坐标。
 * 数据时间为 UTC 秒，0 使用接收时间；动态过期更新整体拒绝。
 * 轨迹复用每个 Item 的唯一 TrackItem，读取／清空与设置页操作一致，不额外记录副本。
 */
class ENCVIEWER_SDK_EXPORT RealtimeDataManager : public QObject
{
    Q_OBJECT
public:
    /** @brief 借用指定地图；空地图时仅管理数据，不创建物标；跨线程或已绑定地图销毁后拒绝更新。 */
    explicit RealtimeDataManager(MapWidget *map, QObject *parent = nullptr);
    /** @brief 返回借用的绑定地图；未绑定或已销毁时为空。 */
    MapWidget *map() const;
    /** @brief 返回 AIS／ARPA 最近有效动态时间（UTC）；尚未收到动态数据或类型不支持时无效。
     * @param group 仅支持 Ais 或 Arpa。
     * @param key 来源、所属船与目标组成的业务键。 */
    QDateTime targetUpdateTime(TrackTargetGroup group, const ObservedTargetKey &key) const;
    /** @brief 设置单个 AIS／ARPA 的轨迹开关；分类总开关仍生效。
     * @param group 仅支持 Ais 或 Arpa。
     * @param key 目标业务键。
     * @param visible 是否显示并记录后续位置。
     * @return 无地图、无物标、类型不支持或线程不匹配时返回 false。 */
    bool setTrackVisible(TrackTargetGroup group, const ObservedTargetKey &key, bool visible);
    /** @brief 清空单个 AIS／ARPA 的轨迹点，不改变开关。
     * @param group 仅支持 Ais 或 Arpa。
     * @param key 目标业务键。
     * @return 无地图、无物标、类型不支持或线程不匹配时返回 false。 */
    bool clearTrack(TrackTargetGroup group, const ObservedTargetKey &key);
    /** @brief 移除本管理器创建的全部物标及附属对象。 */
    ~RealtimeDataManager() override;
    /** @brief 更新船舶；首次动态更新创建 Item，后续保持 ID。静态／状态更新可先于动态到达。
     * @param id 非空稳定业务 ID，与 MMSI 和 MapItemId 独立。
     * @param data 业务数据副本。
     * @param kind 船型，同一 ID 创建后不允许变更。
     * @param parts 本次有效数据组；默认完整快照，状态组控制 online。
     * @param error 可空，错误说明；重入修改被拒绝。 */
    bool updateShip(const QString &id, const ShipData &data, ShipKind kind = ShipKind::OwnShip,
                    RealtimeDataParts parts = RealtimeDataPart::All, QString *error = nullptr);
    /** @brief 更新 AIS；键取 sourceId、data.ownShipMMSI、data.staticInfo.mmsi，MMSI 必须非零。
     * @param risk 显示风险等级，由调用方决策，不从 dangerValue 猜测阈值。
     * @param parts 有效数据组；状态组包含航行状态、风险与会遇数据。 */
    bool updateAis(const QString &sourceId, const AisData &data,
                   RealtimeDataParts parts = RealtimeDataPart::All,
                   MapTargetRiskLevel risk = MapTargetRiskLevel::Low, QString *error = nullptr);
    /** @brief 更新已归一化 ARPA 完整快照；要求经纬度有效、courseTR='T'、units='N'，速度为节。
     * @note 相对方向／其他单位须在接入层转换；status='L' 保留快照但隐藏物标且不追加轨迹。
     * @param risk 显示风险由调用方指定。 */
    bool updateArpa(const QString &sourceId, const ArpaData &data,
                    MapTargetRiskLevel risk = MapTargetRiskLevel::Low, QString *error = nullptr);
    /** @brief 返回船舶业务快照，不存在时为空。 */
    std::optional<ShipData> ship(const QString &id) const;
    /** @brief 返回 AIS 业务快照，不存在时为空。 */
    std::optional<AisData> ais(const ObservedTargetKey &key) const;
    /** @brief 返回 ARPA 业务快照，不存在时为空。 */
    std::optional<ArpaData> arpa(const ObservedTargetKey &key) const;
    /** @brief 返回船舶业务 ID 列表。 */
    QStringList shipIds() const;
    /** @brief 返回 AIS 主键列表。 */
    QList<ObservedTargetKey> aisKeys() const;
    /** @brief 返回 ARPA 主键列表。 */
    QList<ObservedTargetKey> arpaKeys() const;
    /** @brief 返回可调整显隐和附属显示的船舶 Item；未收到动态数据时为空。 */
    QSharedPointer<ShipItem> shipItem(const QString &id) const;
    /** @brief 返回 AIS Item；可用其 trackItem 设置单目标轨迹显隐。 */
    QSharedPointer<AisItem> aisItem(const ObservedTargetKey &key) const;
    /** @brief 返回 ARPA Item；丢失状态始终隐藏，恢复跟踪后恢复可见。 */
    QSharedPointer<ArpaItem> arpaItem(const ObservedTargetKey &key) const;
    /** @brief 删除船舶及附属对象，不影响其他管理器的目标。 */
    bool removeShip(const QString &id);
    /** @brief 删除 AIS 及轨迹。 */
    bool removeAis(const ObservedTargetKey &key);
    /** @brief 删除 ARPA 及轨迹。 */
    bool removeArpa(const ObservedTargetKey &key);
    /** @brief 清空本管理器指定分类的目标及附属对象，默认全部。 */
    void clearTargets(TrackTargetGroup group = TrackTargetGroup::All);
    /** @brief 仅清空本管理器的轨迹点，保留物标和记录开关。 */
    void clearTracks(TrackTargetGroup group = TrackTargetGroup::All);
signals:
    /** @brief 成功更新／删除／清空后发出。 */
    void targetsChanged();
private:
    struct Private;
    std::unique_ptr<Private> d;
};
