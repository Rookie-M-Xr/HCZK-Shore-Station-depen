#pragma once

#include <QList>
#include <QMetaType>
#include <QPointF>
#include <QRectF>
#include <QString>

/**
 * @brief SDK 对外使用的 ENC 图元分类，用于对象分类和图层筛选。
 * @note 不区分单几何与多几何；具体几何结构由 EncSdkGeometryType 表示。
 *       Unknown 表示未知分类，不能作为有效的点、线、面查询筛选条件。
 */
enum class EncSdkPrimitive
{
    Unknown = 0, ///< 未知图元。
    Point = 1,   ///< 点图元。
    Line = 2,    ///< 线图元。
    Area = 3     ///< 面图元。
};

/**
 * @brief SDK 对外使用的 ENC 几何类型，区分单点、多点、单线、多线、单面和多面。
 * @note parts 和 polygonOffsets 的组织规则见 EncSdkGeometry；Unknown 不表示有效的已知几何类型。
 */
enum class EncSdkGeometryType
{
    Unknown = 0,   ///< 未知几何。
    Point,         ///< 单点。
    MultiPoint,    ///< 多点。
    LineString,    ///< 单线。
    MultiLineString, ///< 多线。
    Polygon,       ///< 单面。
    MultiPolygon   ///< 多面。
};

/**
 * @brief SDK ENC 文件导入模式。
 * @warning Rebuild 会清理主缓存目录的数据，不是仅重建本次输入的文件；失败不回滚。
 *          模式不表示导入结果，请通过导入完成结果判断成功与失败。
 */
enum class EncSdkImportMode
{
    Append, ///< 保留已有缓存；按文件名和更新清单跳过已导入文件，重新导入变化的文件。
    Rebuild ///< 先删除主目录全部 cache、idx、wd、主数据库及相关导入记录，再导入本次文件；失败不回滚，不删除原始 ENC 或追加目录的磁盘缓存。
};

/**
 * @brief SDK 对外公开的 WGS84 二维几何快照；x 为经度、y 为纬度，单位为度。
 * @note 查询保留源坐标转换后的双精度，不经过绘制用 float 压缩，不包含高度或水深。
 *       空几何的 parts 为空；点、水平线、垂直线不能用包围盒面积判断是否为空。
 */
struct EncSdkGeometry
{
    EncSdkGeometryType type = EncSdkGeometryType::Unknown; ///< 几何类型。
    QList<QList<QPointF>> parts;                            ///< 点、线或闭合面环坐标；Point 为一个单点 part，MultiPoint 每点一个 part，多线每线一个 part。
    QRectF boundingRect;                                   ///< 按返回顶点计算的经纬度包围盒；跨日期变更线时可能较大。
    QList<quint32> polygonOffsets;                         ///< 各面外环在 parts 中的下标；其后至下一项前为该面的内环。单面为 {0}，点线和空几何为空，不保存末尾 sentinel。
};

/** @brief 精确图层查询条件，不包含图层显示名称。 */
struct EncSdkLayerFilter
{
    QString acronym; ///< S-57 对象类缩写；查询时去除首尾空白并转大写。
    EncSdkPrimitive primitive = EncSdkPrimitive::Unknown; ///< 必须指定 Point、Line 或 Area。
};

/**
 * @brief SDK 对外公开的 ENC Cell 元数据快照。
 * @note 普通值类型；修改返回值不影响 SDK 内部数据，默认构造值不表示有效查询结果。
 *       覆盖字段是 WGS84 外环列表，不表达完整多面外环与内环分组，
 *       也不是当前视口中经过海图重叠裁切后的最终可见区域。
 */
struct EncSdkCellInfo
{
    qint64 id = 0;          ///< Cell 唯一 ID。
    QString cellName;       ///< Cell 名称。
    QString chartName;      ///< 海图名称。
    QString fileName;       ///< 源文件名。
    QString filePath;       ///< 记录的源文件路径；不保证文件仍存在或当前设备可访问。
    int featureCount = 0;   ///< Cell 元数据中的要素数量，不代表当前视口可见对象数。
    int usageBand = 0;      ///< S-57 usage band。
    int compileScale = 0;   ///< 编图比例尺分母，例如 50000 表示 1:50000。
    QRectF boundingRect;    ///< WGS84 经纬度包围盒，单位为度；跨日期变更线时可能较大。
    QList<QList<QPointF>> includePolygons; ///< WGS84 有效覆盖面外环。
    QList<QList<QPointF>> excludePolygons; ///< WGS84 排除覆盖面外环。
    bool fallbackFromBBox = false;         ///< 有效覆盖面是否由 Cell 包围盒回退生成。
};

/**
 * @brief SDK 对外公开的 Feature 身份与完整 WGS84 几何快照，不包含数据库属性。
 * @note 普通值类型；修改返回值不影响 SDK 内部数据。默认构造值不表示有效查询结果。
 *       海图名称通过 cellId 查询 Cell 元数据；几何类型和包围盒统一从 geometry 读取。
 */
struct EncSdkFeatureInfo
{
    qint64 id = 0;                 ///< SDK 生成的 Feature ID，非 S-57 永久标识；不保证更新或对象顺序变化后不变。
    qint64 cellId = 0;             ///< 所属 Cell ID。
    QString objectClass;           ///< S-57 对象类 acronym。
    EncSdkPrimitive primitive = EncSdkPrimitive::Unknown; ///< 图元类型。
    EncSdkGeometry geometry;       ///< 完整 WGS84 几何。
};

/**
 * @brief SDK 最近 SOUNDG 离散测深点查询结果快照。
 * @note 返回实际测深点记录，不是查询位置的插值水深，不做潮位修正。
 *       无匹配或查询失败通过查询接口的 std::optional 空值表示，并由错误输出区分；
 *       depthMeters 为 0 不能作为无效结果标记，默认构造值也不代表查询成功。
 */
struct EncSdkWaterDepth
{
    QPointF lonLat;              ///< 实际匹配的 SOUNDG 测深点，非查询位置；x 为 WGS84 经度、y 为纬度，单位为度。
    double depthMeters = 0.0;    ///< 水深，单位为米。
    double distanceMeters = 0.0; ///< 测深点与查询位置的距离，单位为米。
};

Q_DECLARE_METATYPE(EncSdkPrimitive)
Q_DECLARE_METATYPE(EncSdkGeometryType)
Q_DECLARE_METATYPE(EncSdkImportMode)
Q_DECLARE_METATYPE(EncSdkGeometry)
Q_DECLARE_METATYPE(EncSdkLayerFilter)
Q_DECLARE_METATYPE(QList<EncSdkLayerFilter>)
Q_DECLARE_METATYPE(EncSdkCellInfo)
Q_DECLARE_METATYPE(QList<EncSdkCellInfo>)
Q_DECLARE_METATYPE(EncSdkFeatureInfo)
Q_DECLARE_METATYPE(QList<EncSdkFeatureInfo>)
Q_DECLARE_METATYPE(EncSdkWaterDepth)
