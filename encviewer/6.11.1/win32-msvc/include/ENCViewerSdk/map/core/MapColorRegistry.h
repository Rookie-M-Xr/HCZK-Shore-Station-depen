#pragma once

#include "sdk/EncViewerSdkGlobal.h"
#include "s52/S52SdkTypes.h"
#include <array>

#include <QColor>
#include <QList>
#include <QMap>
#include <QString>

class QSettings;

/**
 * @brief 地图工具和物标颜色定义。
 */
struct ENCVIEWER_SDK_EXPORT MapColorDefinition
{
    QString key;          ///< 稳定的颜色键，建议使用 tool.xxx 或 item.xxx 前缀。
    QString displayName;  ///< 设置编辑器中显示的名称。
    QColor defaultColor;  ///< 白天默认颜色。
    QColor duskColor;     ///< 黄昏默认颜色。
    QColor nightColor;    ///< 夜间默认颜色。
};

/** @brief 样式注册项所属的对象范围。 */
enum class MapStyleScope
{
    Tool, ///< 交互工具样式。
    Item  ///< 地图物标样式。
};

/** @brief 工具或物标的样式分组定义。 */
struct ENCVIEWER_SDK_EXPORT MapStyleDefinition
{
    QString key;                ///< 稳定的样式前缀，例如 tool.measure。
    QString displayName;        ///< 设置界面显示名称。
    MapStyleScope scope = MapStyleScope::Tool; ///< 样式所属范围。
};

/** @brief 工具或物标的数值样式定义。 */
struct ENCVIEWER_SDK_EXPORT MapStyleMetricDefinition
{
    QString key;                ///< 稳定的数值键。
    QString displayName;        ///< 设置界面显示名称。
    int defaultValue = 0;       ///< 默认值。
    int minimum = 1;             ///< 允许的最小值。
    int maximum = 64;            ///< 允许的最大值。
};

/**
 * @brief 地图工具和物标的统一颜色注册表。
 *
 * Tool/Item 通过注册颜色键接入设置编辑器，绘制时按键读取当前颜色。
 * 自定义 Tool/Item 无需修改设置窗口即可增加自己的颜色项。
 * @note 全部接口仅在 GUI 线程使用。自定义项应在设置加载和设置界面创建前注册。
 * 颜色修改自动请求地图重绘，不自动保存；无模式参数的读写使用当前白／昏／夜模式。
 */
class ENCVIEWER_SDK_EXPORT MapColorRegistry
{
public:
    /** @brief 返回进程内唯一的颜色注册表。 */
    static MapColorRegistry &instance();

    /**
     * @brief 注册一个颜色项。
     * @param key 稳定唯一键。
     * @param displayName 设置编辑器中的显示名称。
     * @param defaultColor 默认颜色。
     */
    void registerColor(const QString &key,
                       const QString &displayName,
                       const QColor &defaultColor);

    /** @brief 注册三模式配色；白天三种色表共用 day，保留已保存的用户值。 */
    void registerColor(const QString &key, const QString &displayName,
                       const QColor &day, const QColor &dusk, const QColor &night);
    /** @brief 返回指定模式的颜色；未知键返回 fallback。 */
    QColor color(const QString &key, S52ColorTableMode mode, const QColor &fallback = {}) const;
    /** @brief 设置指定模式的颜色，不改变其他模式；无效颜色不写入。 */
    void setColor(const QString &key, S52ColorTableMode mode, const QColor &color);
    /** @brief 返回当前绘制配色模式，三个白天模式统一返回 DayBright。 */
    S52ColorTableMode colorMode() const;

    /**
     * @brief 注册一个工具或物标样式分组。
     * @param key 样式前缀。
     * @param displayName 设置界面显示名称。
     * @param scope 样式所属范围。
     */
    void registerStyle(const QString &key,
                       const QString &displayName,
                       MapStyleScope scope);

    /**
     * @brief 注册一个字号或符号大小参数。
     * @param key 稳定的数值键。
     * @param displayName 设置界面显示名称。
     * @param defaultValue 默认值。
     * @param minimum 最小值。
     * @param maximum 最大值。
     */
    void registerMetric(const QString &key,
                        const QString &displayName,
                        int defaultValue,
                        int minimum = 1,
                        int maximum = 64);

    /** @brief 返回颜色键当前颜色；未注册或无效时返回 fallback。 */
    QColor color(const QString &key, const QColor &fallback = {}) const;

    /** @brief 设置颜色键当前颜色；无效颜色不会写入。 */
    void setColor(const QString &key, const QColor &color);

    /** @brief 返回按注册顺序排列的颜色定义。 */
    QList<MapColorDefinition> definitions() const;

    /** @brief 返回按注册顺序排列的工具和物标样式分组。 */
    QList<MapStyleDefinition> styleDefinitions() const;

    /** @brief 返回按注册顺序排列的字号和符号大小参数。 */
    QList<MapStyleMetricDefinition> metricDefinitions() const;

    /** @brief 返回数值样式当前值；未注册或无效时返回 fallback。 */
    int metric(const QString &key, int fallback = 0) const;

    /** @brief 设置数值样式当前值并限制在注册范围内。 */
    void setMetric(const QString &key, int value);

    /** @brief 将已注册的颜色和数值恢复为默认值，保留注册定义。 */
    void resetToDefaults();

private:
    friend class ChartSettingsStorage;
    friend class MapColorRegistryTestAccess;
    friend class MapStyleColorScope;
    friend class TrackItem;
    QString resolvedKey(const QString &key) const;

    void load(QSettings &settings);
    void save(QSettings &settings) const;
    MapColorRegistry();
    MapColorRegistry(const MapColorRegistry &) = delete;
    MapColorRegistry &operator=(const MapColorRegistry &) = delete;

    QList<MapColorDefinition> m_definitions;
    std::array<QMap<QString, QColor>, 3> m_colors;
    int m_modeOverride = -1;
    QString m_itemPrefix;
    QString m_toolPrefix;
    QList<MapStyleDefinition> m_styleDefinitions;
    QList<MapStyleMetricDefinition> m_metricDefinitions;
    QMap<QString, int> m_metrics;
    QMap<QString, quint64> m_metricRevisions; ///< 数值实际变化次数，用于识别两次采样间的轨迹开关切换。
};

/** @brief 内置工具和物标使用的颜色键。 */
namespace MapColorKeys
{
/** @brief 艏向／迹向扇形的配色及默认参数。 */
inline QString shipSectorLine() { return QStringLiteral("item.shipSector.line"); }
inline QString shipSectorFill() { return QStringLiteral("item.shipSector.fill"); }
inline QString shipSectorWidth() { return QStringLiteral("item.shipSector.width"); }
inline QString shipSectorRadiusMeters() { return QStringLiteral("item.shipSector.radiusMeters"); }
inline QString shipSectorOpeningAngle() { return QStringLiteral("item.shipSector.openingAngle"); }
inline QString shipSectorDirection() { return QStringLiteral("item.shipSector.direction"); }
inline QString shipSectorShow() { return QStringLiteral("item.shipSector.show"); }
/** @brief 航程矢量线独立样式键。 */
inline QString shipRangeVectorLine() { return QStringLiteral("item.shipRangeVector.line"); }
inline QString shipRangeVectorText() { return QStringLiteral("item.shipRangeVector.text"); }
inline QString shipRangeVectorWidth() { return QStringLiteral("item.shipRangeVector.width"); }
inline QString shipRangeVectorTickSize() { return QStringLiteral("item.shipRangeVector.tickSize"); }
inline QString shipRangeVectorTextSize() { return QStringLiteral("item.shipRangeVector.textSize"); }
/** @brief 默认预测时长，单位分钟；显式设置单船时间时不使用此值。 */
inline QString shipRangeVectorDurationMinutes() { return QStringLiteral("item.shipRangeVector.durationMinutes"); }
/** @brief 时间刻度文字显示开关，0 隐藏，1 显示。 */
inline QString shipRangeVectorShowText() { return QStringLiteral("item.shipRangeVector.showText"); }
inline QString selectionColor() { return QStringLiteral("item.common.selectionColor"); }
inline QString selectionWidth() { return QStringLiteral("item.common.selectionWidth"); }
inline QString routeCreateLine() { return QStringLiteral("tool.route.create.line"); }
inline QString routeCreateGuide() { return QStringLiteral("tool.route.create.guide"); }
inline QString routeCreateText() { return QStringLiteral("tool.route.create.text"); }
inline QString routeCreateTextBackground() { return QStringLiteral("tool.route.create.textBackground"); }
inline QString routeCreateTextSize() { return QStringLiteral("tool.route.create.textSize"); }
inline QString routeCreateSymbolSize() { return QStringLiteral("tool.route.create.symbolSize"); }
inline QString routeEditLine() { return QStringLiteral("tool.route.edit.line"); }
inline QString routeEditGuide() { return QStringLiteral("tool.route.edit.guide"); }
inline QString routeEditText() { return QStringLiteral("tool.route.edit.text"); }
inline QString routeEditTextBackground() { return QStringLiteral("tool.route.edit.textBackground"); }
inline QString routeEditTextSize() { return QStringLiteral("tool.route.edit.textSize"); }
inline QString routeEditSymbolSize() { return QStringLiteral("tool.route.edit.symbolSize"); }
inline QString routeWaypointText() { return QStringLiteral("item.route.waypointText"); }
inline QString routeWaypointBackground() { return QStringLiteral("item.route.waypointBackground"); }
inline QString routeItemLine() { return QStringLiteral("item.route.line"); }
inline QString routeItemTextBackground() { return QStringLiteral("item.route.textBackground"); }
inline QString routeItemTextSize() { return QStringLiteral("item.route.textSize"); }
inline QString routeItemSymbolSize() { return QStringLiteral("item.route.symbolSize"); }
inline QString routeItemShowName() { return QStringLiteral("item.route.showName"); }
inline QString routeCreateMarker() { return QStringLiteral("tool.route.create.marker"); }
inline QString routeEditMarker() { return QStringLiteral("tool.route.edit.marker"); }
inline QString queryHighlightOuter() { return QStringLiteral("tool.queryBox.highlightOuter"); }
inline QString queryHighlightInner() { return QStringLiteral("tool.queryBox.highlightInner"); }
inline QString shipBody() { return QStringLiteral("item.ship.body"); }
inline QString shipDetail() { return QStringLiteral("item.ship.detail"); }
inline QString queryBox() { return QStringLiteral("tool.queryBox"); }
inline QString measureLine() { return QStringLiteral("tool.measure.line"); }
inline QString measureText() { return QStringLiteral("tool.measure.text"); }
inline QString measureMarker() { return QStringLiteral("tool.measure.marker"); }
inline QString measureTextBackground() { return QStringLiteral("tool.measure.textBackground"); }
inline QString measureItemLine() { return QStringLiteral("item.measure.line"); }
inline QString measureItemText() { return QStringLiteral("item.measure.text"); }
inline QString measureItemMarker() { return QStringLiteral("item.measure.marker"); }
inline QString measureItemTextBackground() { return QStringLiteral("item.measure.textBackground"); }
inline QString measureTextSize() { return QStringLiteral("tool.measure.textSize"); }
inline QString measureItemTextSize() { return QStringLiteral("item.measure.textSize"); }
inline QString measureSymbolSize() { return QStringLiteral("tool.measure.symbolSize"); }
inline QString measureItemSymbolSize() { return QStringLiteral("item.measure.symbolSize"); }
inline QString eblLine() { return QStringLiteral("tool.ebl.line"); }
inline QString eblText() { return QStringLiteral("tool.ebl.text"); }
inline QString eblMarker() { return QStringLiteral("tool.ebl.marker"); }
inline QString eblTextBackground() { return QStringLiteral("tool.ebl.textBackground"); }
inline QString eblItemLine() { return QStringLiteral("item.ebl.line"); }
inline QString eblItemText() { return QStringLiteral("item.ebl.text"); }
inline QString eblItemMarker() { return QStringLiteral("item.ebl.marker"); }
inline QString eblItemTextBackground() { return QStringLiteral("item.ebl.textBackground"); }
inline QString eblTextSize() { return QStringLiteral("tool.ebl.textSize"); }
inline QString eblItemTextSize() { return QStringLiteral("item.ebl.textSize"); }
inline QString eblSymbolSize() { return QStringLiteral("tool.ebl.symbolSize"); }
inline QString eblItemSymbolSize() { return QStringLiteral("item.ebl.symbolSize"); }
inline QString rubberDeleteBorder() { return QStringLiteral("item.rubber.deleteBorder"); }
inline QString rubberDeleteBackground() { return QStringLiteral("item.rubber.deleteBackground"); }
inline QString rubberDeleteIcon() { return QStringLiteral("item.rubber.deleteIcon"); }
inline QString rubberSymbolSize() { return QStringLiteral("item.rubber.symbolSize"); }
inline QString aisRiskLow() { return QStringLiteral("item.ais.risk.low"); }
inline QString aisRiskMedium() { return QStringLiteral("item.ais.risk.medium"); }
inline QString aisRiskHigh() { return QStringLiteral("item.ais.risk.high"); }
inline QString aisText() { return QStringLiteral("item.ais.text"); }
inline QString aisTextBackground() { return QStringLiteral("item.ais.textBackground"); }
inline QString aisVector() { return QStringLiteral("item.ais.vector"); }
inline QString aisSymbolSize() { return QStringLiteral("item.ais.symbolSize"); }
inline QString aisTextSize() { return QStringLiteral("item.ais.textSize"); }
inline QString arpaRiskLow() { return QStringLiteral("item.arpa.risk.low"); }
inline QString arpaRiskMedium() { return QStringLiteral("item.arpa.risk.medium"); }
inline QString arpaRiskHigh() { return QStringLiteral("item.arpa.risk.high"); }
inline QString arpaText() { return QStringLiteral("item.arpa.text"); }
inline QString arpaTextBackground() { return QStringLiteral("item.arpa.textBackground"); }
inline QString arpaVector() { return QStringLiteral("item.arpa.vector"); }
inline QString arpaSymbolSize() { return QStringLiteral("item.arpa.symbolSize"); }
inline QString arpaTextSize() { return QStringLiteral("item.arpa.textSize"); }
inline QString shipOnline() { return QStringLiteral("item.ship.online"); }
inline QString shipOffline() { return QStringLiteral("item.ship.offline"); }
inline QString shipControlled() { return QStringLiteral("item.ship.controlled"); }
inline QString shipText() { return QStringLiteral("item.ship.text"); }
inline QString shipTextBackground() { return QStringLiteral("item.ship.textBackground"); }
/** @brief 旧船舶矢量色键，仅保留源码兼容；新航程线使用 shipRangeVectorLine。 */
inline QString shipVector() { return QStringLiteral("item.ship.vector"); }
inline QString shipSymbolSize() { return QStringLiteral("item.ship.symbolSize"); }
inline QString shipTextSize() { return QStringLiteral("item.ship.textSize"); }
inline QString trackItemLine() { return QStringLiteral("item.track.line"); }
inline QString trackItemShow() { return QStringLiteral("item.track.show"); }
inline QString aisTrackLine() { return QStringLiteral("item.aisTrack.line"); }
inline QString aisTrackWidth() { return QStringLiteral("item.aisTrack.width"); }
inline QString aisTrackShow() { return QStringLiteral("item.aisTrack.show"); }
inline QString arpaTrackLine() { return QStringLiteral("item.arpaTrack.line"); }
inline QString arpaTrackWidth() { return QStringLiteral("item.arpaTrack.width"); }
inline QString arpaTrackShow() { return QStringLiteral("item.arpaTrack.show"); }
inline QString shipRangeVectorShow() { return QStringLiteral("item.shipRangeVector.show"); }
inline QString trackItemWidth() { return QStringLiteral("item.track.width"); }
}
