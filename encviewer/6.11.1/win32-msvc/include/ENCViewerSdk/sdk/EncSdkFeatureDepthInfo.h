#pragma once

#include "EncSdkTypes.h"
#include <optional>

/**
 * @brief 同次 Cell cache 读取产生的完整 WGS84 几何和水深快照，不访问 SQLite。
 * @note 独立值类型；不改变 EncSdkFeatureInfo 的布局。水深单位为米，零和负值合法，
 *       空值表示该属性不存在或不适用于对象类。保留 cache 的 float 有效水深精度，
 *       提升为 double 不恢复原始精度，不插值、不舍入、不做潮位或安全裕量修正。
 *       SDK 的 SOUNDG 已拆成单点 Feature；其 geometry 为一个 Point，水深只属于该点。
 *       Cell/Feature ID 不构成跨重导入稳定的永久身份，不能与另一次查询的几何拼接。
 */
struct EncSdkFeatureDepthInfo
{
    EncSdkFeatureInfo feature;                 ///< 身份与完整几何，包括多面和孔洞分组。
    std::optional<double> drval1;              ///< DEPARE 水深下界；其他对象类为空。
    std::optional<double> soundingDepthMeters; ///< SOUNDG 单个测深点的水深；其他对象类为空。
};

Q_DECLARE_METATYPE(EncSdkFeatureDepthInfo)
Q_DECLARE_METATYPE(QList<EncSdkFeatureDepthInfo>)
