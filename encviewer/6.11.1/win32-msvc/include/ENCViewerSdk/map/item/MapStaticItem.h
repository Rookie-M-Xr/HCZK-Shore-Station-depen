#pragma once

#include "map/item/MapItem.h"

/**
 * @brief 静态地图对象分类基类。
 *
 * 用于几何和业务状态不会随时间持续变化的地图对象。
 */
class ENCVIEWER_SDK_EXPORT MapStaticItem : public MapItem
{
public:
    /** @brief 析构静态地图对象。 */
    ~MapStaticItem() override;

protected:
    /** @brief 构造静态地图对象分类基类。 */
    explicit MapStaticItem(bool visible = true) : MapItem(visible) {}
};
