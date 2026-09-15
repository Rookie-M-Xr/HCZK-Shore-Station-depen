/**
 * @file VideoHczkProtocol.h
 * @brief 定义视频服务域命令及当前已明确的固定长度载荷。
 *
 * 未建立强类型 DTO 的变长视频载荷由协议引擎以 HczkRawPayload 形式交给应用层。
 */
#ifndef VIDEOHCZKPROTOCOL_H
#define VIDEOHCZKPROTOCOL_H

#include <QtGlobal>

namespace VideoHczkProtocol
{
// 定义视频服务模块的请求、应答和告警命令字。
enum class Command : quint16
{
    Heartbeat = 0x2A00, // 心跳请求。
    CameraListRequest = 0x2A01, // 摄像机列表请求。
    CameraCapabilityRequest = 0x2A02, // 摄像机能力请求。
    StartStream = 0x2A03, // 启动视频流。
    StopStream = 0x2A04, // 停止视频流。
    SwitchProfile = 0x2A05, // 切换视频配置档。
    StatusRequest = 0x2A06, // 视频状态请求。
    SetDefaultPushParameters = 0x2A07, // 设置默认推流参数。
    RestartService = 0x2A08, // 重启视频服务。
    HeartbeatReply = 0xBA00, // 心跳应答。
    CameraListReply = 0xBA01, // 摄像机列表应答。
    CameraCapabilityReply = 0xBA02, // 摄像机能力应答。
    StartStreamReply = 0xBA03, // 启动视频流应答。
    StopStreamReply = 0xBA04, // 停止视频流应答。
    SwitchProfileReply = 0xBA05, // 切换视频配置档应答。
    StatusReply = 0xBA06, // 视频状态应答。
    SetDefaultPushParametersReply = 0xBA07, // 设置默认推流参数应答。
    RestartServiceReply = 0xBA08, // 重启视频服务应答。
    Alarm = 0xBA09 // 视频告警通知。
};

#pragma pack(push, 1)
// 视频服务心跳载荷。
struct HeartbeatPayload
{
    quint64 timestampMs; // UTC 时间戳，单位毫秒。
};

// 摄像机列表请求载荷。
struct CameraListRequestPayload
{
    quint8 listType; // 请求的摄像机列表类型。
};

// 摄像机能力请求载荷。
struct CameraCapabilityRequestPayload
{
    quint16 cameraId; // 摄像机编号。
};

// 视频服务重启请求载荷。
struct RestartServicePayload
{
    quint8 restartType; // 重启范围或方式。
};
#pragma pack(pop)

// 判断命令字是否属于视频模块，不检查载荷内容。
bool isVideoCommand(quint16 command);

// 返回固定载荷长度；变长命令返回 -1，未知命令返回 -2。
int fixedPayloadSize(quint16 command);
}

#endif // VIDEOHCZKPROTOCOL_H
