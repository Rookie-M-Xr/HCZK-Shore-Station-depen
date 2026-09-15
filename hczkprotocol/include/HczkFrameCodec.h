/**
 * @file HczkFrameCodec.h
 * @brief 声明 HCZK 线协议帧的组包、拆包和 CRC32 完整性校验接口。
 *
 * 帧编解码层只处理公共包头和原始业务载荷，不判断命令所属的业务模块。
 */
#ifndef HCZKFRAMECODEC_H
#define HCZKFRAMECODEC_H

#include <QByteArray>
#include <QString>
#include <QtGlobal>

#include <cstddef>
#include <cstdint>

#include "HczkMessage.h"

// HCZK 线协议包头。不得调整字段顺序、宽度或填充；msglength 包含业务载荷和末尾 CRC，
// 但不包含包头本身。
#pragma pack(push, 1)
struct HczkPackHeader
{
    char sys_name[4]; // 固定帧标识“HCZK”。
    quint32 sendId; // 发送方标识。
    quint32 recvId; // 接收方标识。
    qint64 time; // UTC 时间戳，单位毫秒。
    quint16 cmd; // 业务命令字。
    quint16 version; // 协议版本号。
    quint16 packId; // 数据包序号。
    quint16 reserved2; // 协议保留字段。
    quint16 msglength; // 业务载荷与 CRC 的总字节数。
};
#pragma pack(pop)

// 定义 HCZK 帧完整性检查可能返回的错误。
enum class HczkFrameError
{
    None, // 未发生错误。
    EmptyData, // 输入数据为空。
    InvalidMagic, // 帧标识不是 HCZK。
    IncompleteHeader, // 输入不足一个完整包头。
    InvalidLength, // 长度字段非法或与输入不一致。
    IncompleteFrame, // 输入不足一个完整帧。
    CrcMismatch, // CRC32 校验失败。
    UnknownTopic // MQTT 主题未配置协议路由。
};

Q_DECLARE_METATYPE(HczkFrameError)

// 保存 HCZK 帧解析结果；成功时 frame 为完整帧，失败时 message 为中文错误说明。
struct HczkFrameResult
{
    HczkFrameError error = HczkFrameError::None; // 帧错误码。
    QByteArray frame; // 校验成功的完整帧。
    QString message; // 失败原因；成功时为空。

    // 判断帧解析和校验是否成功，不修改结果内容。
    bool isValid() const
    {
        return error == HczkFrameError::None;
    }
};

// 提供无状态的 HCZK 帧编解码能力；所有输入和输出缓冲区均由调用方持有。
class HczkFrameCodec final
{
public:
    // 从缓冲区首部提取并校验一帧；仅成功时移除已提取字节。
    static HczkFrameResult takeFrame(QByteArray& buffer);

    // 根据路由字段和业务载荷生成完整协议帧，帧时间使用当前 UTC 毫秒。
    static QByteArray encodeFrame(quint32 sendId, quint32 recvId, quint16 cmd, const QByteArray& payload, quint16 version = 10, quint16 packId = 0);

    // 使用指定包头和业务载荷生成完整协议帧，并重写帧标识、长度和 CRC。
    static QByteArray encodeFrame(HczkPackHeader header, const QByteArray& payload);

    // 校验一个且仅一个完整协议帧，并按需复制包头和载荷；不接受尾随字节。
    static HczkFrameResult decodeFrame(const QByteArray& frame, HczkPackHeader* header = nullptr, QByteArray* payload = nullptr);

    // 将统一消息编码为完整 HCZK 帧；时间戳非正数时使用当前 UTC 毫秒。
    static QByteArray encodeMessage(const HczkMessage& message);

    // 校验完整帧并转换为统一消息，message 不能为空且由调用方持有。
    static HczkFrameResult decodeMessage(const QByteArray& frame, HczkMessage* message);
};

#endif // HCZKFRAMECODEC_H
