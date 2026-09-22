#pragma once

#include "map/item/MapItem.h"

#include <QPointF>

class MapRenderContext;

/**
 * @brief 带统一删除按钮的临时业务物标基类。
 *
 * 派生对象只需提供删除按钮锚点，基类负责按钮绘制和命中测试。
 */
class ENCVIEWER_SDK_EXPORT MapRubberItem : public MapItem
{
public:
    /** @brief 析构临时业务物标。 */
    ~MapRubberItem() override;

    /** @brief 返回删除按钮跟随的 WGS84 经纬度锚点。 */
    virtual QPointF deleteAnchor() const = 0;

    /** @brief 在 Tool 上层绘制删除按钮。 */
    void drawInteractionControls(MapRenderContext &context) const final;

    /** @brief 判断屏幕点是否命中删除按钮。 */
    bool containsInteractionControl(const QPointF &screenPoint,
                           const MapRenderContext &context) const final;

protected:
    /** @brief 构造临时业务物标。 */
    explicit MapRubberItem(bool visible = true);
};
