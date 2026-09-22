#pragma once

#include "sdk/EncViewerSdkGlobal.h"

#include <QDialog>
#include <QPointF>
#include <QString>

#include <optional>

class QLabel;
class QComboBox;
class QLineEdit;

/**
 * @brief 可复用的 WGS84 坐标输入组件，不绑定地图或工具。
 * @note 在 GUI 线程使用；parent 仅用于窗口归属。结果 x 为经度、y 为纬度，单位为度。
 */
class ENCVIEWER_SDK_EXPORT MapCoordinateInputDialog final : public QDialog
{
public:
    /** @brief 构造坐标输入对话框，可选填入初始 WGS84 坐标。 */
    explicit MapCoordinateInputDialog(QWidget *parent = nullptr,
                                      std::optional<QPointF> initialCoordinate = std::nullopt);

    /** @brief 返回已确认坐标；仅在对话框返回 Accepted 后读取。 */
    QPointF coordinate() const noexcept;

    /**
     * @brief 输入、校验并返回 WGS84 坐标，不执行地图或业务操作。
     * @param parent 窗口父对象，可为空。
     * @param initialCoordinate 初始坐标；非有限数或超出经纬度范围时忽略。
     * @return 确认时返回合法坐标；取消或关闭时返回 std::nullopt。
     */
    static std::optional<QPointF> getCoordinate(
        QWidget *parent = nullptr,
        std::optional<QPointF> initialCoordinate = std::nullopt);

protected:
    /** @brief 校验度/度分/度分秒文本后接受对话框。 */
    void accept() override;

private:
    QString coordinateText(QLineEdit *degrees,
                           QLineEdit *minutes,
                           QLineEdit *seconds,
                           QComboBox *hemisphere) const;
    void setInitialCoordinate(const QPointF &coordinate);

    QLineEdit *m_longitudeDegrees = nullptr; ///< 经度度输入框。
    QLineEdit *m_longitudeMinutes = nullptr; ///< 经度分输入框。
    QLineEdit *m_longitudeSeconds = nullptr; ///< 经度秒输入框。
    QComboBox *m_longitudeHemisphere = nullptr; ///< 经度方向选择框。
    QLineEdit *m_latitudeDegrees = nullptr;  ///< 纬度度输入框。
    QLineEdit *m_latitudeMinutes = nullptr;  ///< 纬度分输入框。
    QLineEdit *m_latitudeSeconds = nullptr;  ///< 纬度秒输入框。
    QComboBox *m_latitudeHemisphere = nullptr; ///< 纬度方向选择框。
    QLabel *m_errorLabel = nullptr;            ///< 校验错误提示。
    QPointF m_coordinate;                     ///< 已确认坐标。
};
