#pragma once

#include "sdk/EncViewerSdkGlobal.h"

#include <QMetaType>
#include <QString>
#include <QtGlobal>

/**
 * @brief 坐标系坐标单位。
 */
enum class EncCoordinateUnit : quint8
{
    Unknown, ///< 未知单位。
    Degree,  ///< 角度。
    Meter,   ///< 米。
    Foot,    ///< 英尺。
    Other    ///< 由 metersPerUnit 描述的其他线性单位。
};

/**
 * @brief 坐标系轴顺序；运行时转换统一使用传统 GIS 轴顺序。
 */
enum class EncCoordinateAxisOrder : quint8;
struct EncCoordinateReferenceSystemPrivate;

/**
 * @brief 描述 ENC 源数据或全局绘制坐标系的通用 CRS 值类型。
 * @note 转换优先使用有效的 EPSG 标识，其次 PROJJSON，最后 WKT；不要填写互相冲突的定义。
 * @note 单位字段为描述信息，不会覆盖定义本身的单位。坐标转换统一使用经纬度或东、北顺序。
 */
struct ENCVIEWER_SDK_EXPORT EncCoordinateReferenceSystem
{
    QString authorityName;                                  ///< 权威机构名称，例如 EPSG。
    int authorityCode = 0;                                  ///< 权威机构代码，例如 4326。
    QString wkt2;                                           ///< WKT2 坐标系定义。
    QString projJson;                                       ///< 可选 PROJJSON 坐标系定义。
    EncCoordinateUnit coordinateUnit = EncCoordinateUnit::Unknown; ///< 坐标单位。
    QString coordinateUnitName;                            ///< 坐标单位原始名称。
    double metersPerUnit = 0.0;                            ///< 每个坐标单位对应的米数；角度坐标为 0。
private:
    friend struct EncCoordinateReferenceSystemPrivate;
    friend ENCVIEWER_SDK_EXPORT bool operator==(const EncCoordinateReferenceSystem &, const EncCoordinateReferenceSystem &);
    EncCoordinateAxisOrder axisOrder = static_cast<EncCoordinateAxisOrder>(0); ///< 原始轴顺序。
    int s57HorizontalDatum = 0;                              ///< S-57 DSPM.HDAT 值。
    bool inferred = false;                                  ///< 是否由产品规范推断。

public:
    /** @brief 返回标准 WGS84 经纬度坐标系。 */
    static EncCoordinateReferenceSystem epsg4326();

    /** @brief 返回标准 Web Mercator 坐标系。 */
    static EncCoordinateReferenceSystem epsg3857();

    /** @brief 判断是否提供了定义；不解析定义，也不保证 GDAL 能创建转换。 */
    bool isValid() const;

    /** @brief 判断声明的 EPSG 标识是否匹配 code，不进行语义比较。 */
    bool isEpsg(int code) const;

    /** @brief 返回适合界面和日志显示的坐标系名称。 */
    QString displayName() const;

private:
    /** @brief 内部定义键，采用与转换器相同的定义优先级。 */
    QString definitionKey() const;

    /** @brief 内部定义键比较，不进行 CRS 语义解析。 */
    bool isEquivalentTo(const EncCoordinateReferenceSystem &other) const;
};

/** @brief 比较两个 CRS 的完整元数据（含内部来源信息），不判断坐标系语义等价。 */
ENCVIEWER_SDK_EXPORT bool operator==(const EncCoordinateReferenceSystem &left,
                                     const EncCoordinateReferenceSystem &right);

/** @brief 比较两个 CRS 的完整元数据（含内部来源信息），不判断坐标系语义等价。 */
inline bool operator!=(const EncCoordinateReferenceSystem &left,
                       const EncCoordinateReferenceSystem &right)
{
    return !(left == right);
}

Q_DECLARE_METATYPE(EncCoordinateReferenceSystem)
