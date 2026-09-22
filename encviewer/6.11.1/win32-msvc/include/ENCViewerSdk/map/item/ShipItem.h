#pragma once

#include "map/item/MapDynamicItem.h"

#include <QString>

class ShipRangeVectorItem;
class ShipSectorItem;

class TrackItem;

/** @brief 船舶物标的船型。 */
enum class ShipKind : quint8
{
    OwnShip,
    Small,
    Medium,
    Large
};

/** @brief 船舶物标公共基类。 */
class ENCVIEWER_SDK_EXPORT ShipItem : public MapDynamicItem
{
public:
    /** @brief 返回随物标注册的独立轨迹；可通过 setVisible 单独关闭，有效位置更新自动记录轨迹，隐藏时仍记录。 */
    QSharedPointer<TrackItem> trackItem() const noexcept;
    /** @brief 返回自动附带的航程矢量线；随船舶注册到地图，可独立设置时间和可见性。 */
    QSharedPointer<ShipRangeVectorItem> rangeVectorItem() const noexcept;
    /** @brief 返回自动附带的艏向／迹向扇形；随船舶注册，可独立设置范围、方向和可见性。 */
    QSharedPointer<ShipSectorItem> sectorItem() const noexcept;
    /** @brief 返回船舶 MMSI；0 表示未设置，与 MapItemId 独立。 */
    quint32 mmsi() const noexcept;
    /** @brief 更新船舶 MMSI，值变化时通知管理器更新，保持对象及 ID 不变。
     * @param mmsi 船舶 MMSI，0 表示未设置；注册对象须在 GUI 线程更新。 */
    void setMmsi(quint32 mmsi);

    /** @brief 更新运动数据，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setMotion(MapTargetMotion motion);
    /** @brief 更新在线状态，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setOnlineState(ShipOnlineState onlineState);
    /** @brief 更新受控状态，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setControlState(ShipControlState controlState);
    /** @brief 更新船舶名称，保持对象及 ID 不变；已注册对象自动通知更新。 */
    void setName(QString name);

    /** @brief 返回船舶位置范围。 */
    QRectF geographicBounds() const override;
    /** @brief 返回船舶名称。 */
    const QString &name() const noexcept;
    /** @brief 返回船舶运动数据。 */
    const MapTargetMotion &motion() const noexcept;
    /** @brief 返回船舶在线状态。 */
    ShipOnlineState onlineState() const noexcept;
    /** @brief 返回船舶受控状态。 */
    ShipControlState controlState() const noexcept;
    /** @brief 返回船舶图标类型。 */
    virtual ShipKind shipKind() const noexcept = 0;

    /** @brief 返回船舶绘制顺序，本船位于其他船舶上层。 */
    int drawOrder() const noexcept override;

    /** @brief 绘制船舶 SVG、运动文字和状态标记；航程线由 ShipRangeVectorItem 单独绘制。 */
    void draw(MapRenderContext &context) const override;

    /** @brief 判断屏幕点是否命中船舶符号。 */
    bool containsScreenPoint(const QPointF &screenPoint,
                 const MapRenderContext &context) const override;

protected:
    /** @brief 构造船舶公共数据。 */
    ShipItem(QString name,
             MapTargetMotion motion,
             ShipOnlineState onlineState,
             ShipControlState controlState,
             bool visible = true);

private:
    QSharedPointer<TrackItem> m_trackItem; ///< 自动附带的轨迹。
    QSharedPointer<ShipRangeVectorItem> m_rangeVectorItem; ///< 自动附带的独立矢量对象。
    QSharedPointer<ShipSectorItem> m_sectorItem; ///< 自动附带的独立扇形对象。
    quint32 m_mmsi = 0; ///< 船舶 MMSI，0 表示未设置。
    QString m_name; ///< 船舶名称。
    MapTargetMotion m_motion; ///< 位置、航向和速度。
    ShipOnlineState m_onlineState = ShipOnlineState::Offline; ///< 在线状态。
    ShipControlState m_controlState = ShipControlState::Uncontrolled; ///< 受控状态。
};

/** @brief 本船物标。 */
class ENCVIEWER_SDK_EXPORT OwnShipItem final : public ShipItem
{
public:
    /** @brief 为本子类全部现有及新建对象设置单份 SVG；仅 GUI 线程调用。
     * @param path 本地文件或 Qt 资源路径；船首朝上，viewBox 中心为船位。
     * SVG 必须包含 ship-shape 分组；ship-body/ship-detail 元素用于填充调色，
     * 子元素不得用固定颜色或 CSS 覆盖这些角色。
     * @param error 可空，失败时写入原因。
     * @return 成功返回 true；失败保留原图标。内容在调用时读取，不监视文件变化。 */
    static bool setSvg(const QString &path, QString *error = nullptr);
    /** @brief 恢复本子类内置 SVG 并刷新现有地图；仅 GUI 线程调用。 */
    static void resetSvg();

    /** @brief 构造本船物标。 */
    OwnShipItem(QString name,
                MapTargetMotion motion,
                ShipOnlineState onlineState,
                ShipControlState controlState,
                bool visible = true);
    /** @brief 返回本船物标类型。 */ MapItemType type() const noexcept override;
    /** @brief 返回本船图标类型。 */ ShipKind shipKind() const noexcept override;
};

/** @brief 小艇或无人艇物标。 */
class ENCVIEWER_SDK_EXPORT SShipItem final : public ShipItem
{
public:
    /** @brief 为本子类全部现有及新建对象设置单份 SVG；仅 GUI 线程调用。
     * @param path 本地文件或 Qt 资源路径；船首朝上，viewBox 中心为船位。
     * SVG 必须包含 ship-shape 分组；ship-body/ship-detail 元素用于填充调色，
     * 子元素不得用固定颜色或 CSS 覆盖这些角色。
     * @param error 可空，失败时写入原因。
     * @return 成功返回 true；失败保留原图标。内容在调用时读取，不监视文件变化。 */
    static bool setSvg(const QString &path, QString *error = nullptr);
    /** @brief 恢复本子类内置 SVG 并刷新现有地图；仅 GUI 线程调用。 */
    static void resetSvg();

    /** @brief 构造小艇物标。 */
    SShipItem(QString name,
              MapTargetMotion motion,
              ShipOnlineState onlineState,
              ShipControlState controlState,
              bool visible = true);
    /** @brief 返回普通船舶物标类型。 */ MapItemType type() const noexcept override;
    /** @brief 返回小艇图标类型。 */ ShipKind shipKind() const noexcept override;
};

/** @brief 中型船物标。 */
class ENCVIEWER_SDK_EXPORT MShipItem final : public ShipItem
{
public:
    /** @brief 为本子类全部现有及新建对象设置单份 SVG；仅 GUI 线程调用。
     * @param path 本地文件或 Qt 资源路径；船首朝上，viewBox 中心为船位。
     * SVG 必须包含 ship-shape 分组；ship-body/ship-detail 元素用于填充调色，
     * 子元素不得用固定颜色或 CSS 覆盖这些角色。
     * @param error 可空，失败时写入原因。
     * @return 成功返回 true；失败保留原图标。内容在调用时读取，不监视文件变化。 */
    static bool setSvg(const QString &path, QString *error = nullptr);
    /** @brief 恢复本子类内置 SVG 并刷新现有地图；仅 GUI 线程调用。 */
    static void resetSvg();

    /** @brief 构造中型船物标。 */
    MShipItem(QString name,
              MapTargetMotion motion,
              ShipOnlineState onlineState,
              ShipControlState controlState,
              bool visible = true);
    /** @brief 返回普通船舶物标类型。 */ MapItemType type() const noexcept override;
    /** @brief 返回中型船图标类型。 */ ShipKind shipKind() const noexcept override;
};

/** @brief 大船物标。 */
class ENCVIEWER_SDK_EXPORT LShipItem final : public ShipItem
{
public:
    /** @brief 为本子类全部现有及新建对象设置单份 SVG；仅 GUI 线程调用。
     * @param path 本地文件或 Qt 资源路径；船首朝上，viewBox 中心为船位。
     * SVG 必须包含 ship-shape 分组；ship-body/ship-detail 元素用于填充调色，
     * 子元素不得用固定颜色或 CSS 覆盖这些角色。
     * @param error 可空，失败时写入原因。
     * @return 成功返回 true；失败保留原图标。内容在调用时读取，不监视文件变化。 */
    static bool setSvg(const QString &path, QString *error = nullptr);
    /** @brief 恢复本子类内置 SVG 并刷新现有地图；仅 GUI 线程调用。 */
    static void resetSvg();

    /** @brief 构造大船物标。 */
    LShipItem(QString name,
              MapTargetMotion motion,
              ShipOnlineState onlineState,
              ShipControlState controlState,
              bool visible = true);
    /** @brief 返回普通船舶物标类型。 */ MapItemType type() const noexcept override;
    /** @brief 返回大型船图标类型。 */ ShipKind shipKind() const noexcept override;
};
