/**
 * @file HczkMessage.h
 * @brief 定义 HCZK 协议跨模块传递所使用的统一消息模型。
 *
 * 本文件只描述已经完成帧校验后的公共元数据，不负责业务载荷的具体解析。
 * 所有类型均采用 Qt 值类型，能够注册到元对象系统并通过队列连接跨线程传递。
 */
#ifndef HCZKMESSAGE_H
#define HCZKMESSAGE_H

#include <QByteArray>
#include <QMetaType>
#include <QString>
#include <QtGlobal>

// 定义统一协议消息所属的业务模块，用于主题路由和模块解析器选择。
enum class HczkDomain : quint8
{
    Unknown = 0, // 未配置或无法识别的业务模块。
    Sensor, // 传感器数据模块。
    Video, // 视频服务模块。
    Control, // 船舶控制模块。
    Decision // 航行决策模块。
};

// 为 HczkDomain 提供 Qt 哈希函数，使其可作为 QHash 键使用。
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
inline size_t qHash(HczkDomain domain, size_t seed = 0) noexcept
{
    return seed ^ static_cast<size_t>(domain);
}
#else
inline uint qHash(HczkDomain domain, uint seed = 0) noexcept
{
    return seed ^ static_cast<uint>(domain);
}
#endif

// 保存通过帧校验后的统一 HCZK 消息头和原始载荷，所有字段均按线协议解释。
struct HczkMessage
{
    quint32 sendId = 0; // 发送方标识。
    quint32 recvId = 0; // 接收方标识。
    qint64 timestamp = 0; // UTC 时间戳，单位毫秒。
    quint16 command = 0; // 业务命令字。
    quint16 version = 10; // 协议版本号。
    quint16 packetId = 0; // 数据包序号。
    quint16 reserved = 0; // 协议保留字段。
    QByteArray payload; // 未进行模块级解释的业务载荷。
};

Q_DECLARE_METATYPE(HczkDomain)
Q_DECLARE_METATYPE(HczkMessage)

#endif // HCZKMESSAGE_H
