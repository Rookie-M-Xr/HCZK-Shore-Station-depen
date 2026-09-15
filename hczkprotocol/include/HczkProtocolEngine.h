/**
 * @file HczkProtocolEngine.h
 * @brief 声明 HCZK 协议统一入口及 MQTT 主题与业务域的路由模型。
 *
 * 协议引擎负责完成“主题路由—帧校验—模块解析”以及反向编码，应用层只接收
 * 已解析的 QVariant 强类型载荷，不再自行拆包。
 */
#ifndef HCZKPROTOCOLENGINE_H
#define HCZKPROTOCOLENGINE_H

#include "ControlHczkProtocol.h"
#include "DecisionHczkProtocol.h"
#include "HczkMessage.h"
#include "SensorHczkProtocol.h"

#include <QHash>
#include <QObject>
#include <QVariant>
#include <QVector>

// 保存尚未建立强类型结构的 Control 或 Video 载荷；命令字保存在统一消息头中。
struct HczkRawPayload
{
    QByteArray bytes; // 经过帧校验和命令校验的原始业务载荷。
};

using HczkDecodedPayload = QVariant;

// 使用业务域和线路命令值共同标识一条 HCZK 指令，避免不同模块的命令值发生歧义。
struct HczkCommandKey
{
    HczkDomain domain = HczkDomain::Unknown; // 命令所属的业务模块。
    quint16 value = 0; // HCZK 线路上的原始命令值。
};

/**
 * @brief 应用层提交给协议库的统一强类型发送消息。
 *
 * command 同时指定业务域和线路命令字，payload 保存对应模块声明的 Qt 强类型
 * 载荷。协议引擎负责选择模块编码器，不允许应用层自行拼接 HCZK 公共帧头。
 */
struct HczkOutgoingMessage
{
    HczkCommandKey command; ///< 业务域和线路命令字组成的唯一命令键。
    QVariant payload;      ///< 与命令匹配的 Qt 强类型业务载荷。
};

// 保存协议库向应用层输出的统一解析结果；header.payload 固定为空，业务数据位于 decodedPayload。
struct HczkDecodedMessage
{
    QString topic; // 接收消息的 MQTT 主题。
    HczkCommandKey command; // 由业务域和原始命令值组成的唯一运行时标识。
    HczkMessage header; // 已校验的消息元数据，payload 字段为空。
    HczkDecodedPayload decodedPayload; // 模块解析器输出的类型化载荷。
};

Q_DECLARE_METATYPE(HczkRawPayload)
Q_DECLARE_METATYPE(HczkCommandKey)
Q_DECLARE_METATYPE(HczkOutgoingMessage)
Q_DECLARE_METATYPE(HczkDecodedMessage)

// 统一协调 MQTT 主题路由、HCZK 帧校验、模块载荷解析和发送编码。
// 对象应在所属工作线程中调用，输入输出数据均采用值语义，不接管外部缓冲区所有权。
class HczkProtocolEngine : public QObject
{
    Q_OBJECT

public:
    // 创建协议引擎并注册跨线程信号所需的 Qt 元类型。
    explicit HczkProtocolEngine(QObject* parent = nullptr);

    // 配置接收主题对应的业务模块；重复主题将覆盖原有配置。
    void setIncomingRoute(const QString& topic, HczkDomain domain);

    // 为同一接收主题追加候选业务模块，解析时按照添加顺序选择第一个成功的模块。
    void addIncomingRoute(const QString& topic, HczkDomain domain);

    // 配置业务模块对应的发送主题；重复模块将覆盖原有配置。
    void setOutgoingRoute(HczkDomain domain, const QString& topic);

    // 接收一条 MQTT 消息。函数依次完成路由、HCZK 帧校验和模块载荷解析。
    void inputData(const QString& topic, const QByteArray& frame);

    /**
     * @brief 统一编码并发送强类型业务消息。
     *
     * 函数根据 command.domain 选择模块载荷编码器，再统一调用 HCZK 帧编码器。
     * metadata 只提供公共头字段，command 和 payload 由 message 覆盖。
     */
    bool sendMessage(const HczkOutgoingMessage& message, const HczkMessage& metadata, QString* error = nullptr);

    /**
     * @brief 发送已经完成业务序列化的载荷。
     *
     * 该入口仍会校验命令归属和载荷长度，然后统一封装 HCZK 公共帧。
     */
    bool sendRawMessage(const HczkCommandKey& command, const QByteArray& payload, const HczkMessage& metadata, QString* error = nullptr);

signals:
    // 在当前线程发出完整的模块解析结果，不携带未经解析的 Decision 或 Sensor 载荷。
    void messageDecoded(const HczkDecodedMessage& message);

    // 在当前线程发出已完成 CRC 封装的 MQTT 主题和完整帧。
    void frameEncoded(const QString& topic, const QByteArray& frame);

    // 报告主题路由、帧校验或模块解析错误，不输出部分解析结果。
    void protocolError(const QString& topic, const QString& description);

private:
    // 根据业务域选择载荷编码器，将 Qt 强类型载荷转换为线路字节。
    bool encodeModulePayload(const HczkCommandKey& command,  const QVariant& payload, QByteArray* bytes, QString* error) const;

    // 根据业务模块解析载荷；payload 和 error 由调用方持有，本函数不保留其地址。
    bool decodeModulePayload(HczkDomain domain, const HczkMessage& message, HczkDecodedPayload* payload, QString* error) const;

    QHash<QString, QVector<HczkDomain>> m_incomingRoutes; // 接收主题到候选业务模块的有序映射。
    QHash<HczkDomain, QString> m_outgoingRoutes; // 业务模块到发送主题的映射。
};

#endif // HCZKPROTOCOLENGINE_H
