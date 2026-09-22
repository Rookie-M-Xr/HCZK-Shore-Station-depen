#pragma once

#include "map/item/MapItem.h"

/**
 * @brief 动态地图对象分类基类。
 *
 * 用于位置、运动状态或轨迹会随时间更新的地图对象。
 */
class ENCVIEWER_SDK_EXPORT MapDynamicItem : public MapItem
{
public:
    /** @brief 析构动态地图对象。 */
    ~MapDynamicItem() override;

protected:
    /** @brief 构造动态地图对象分类基类。 */
    explicit MapDynamicItem(bool visible = true) : MapItem(visible) {}
};
