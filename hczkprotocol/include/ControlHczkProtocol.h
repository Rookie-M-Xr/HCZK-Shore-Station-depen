/**
 * @file ControlHczkProtocol.h
 * @brief 定义船岸控制域的下发命令、船端上报载荷及编解码接口。
 *
 * 固定载荷使用一字节对齐并由静态断言保护线路尺寸；航线、电子围栏和编队延时
 * 等变长载荷使用 Qt 容器表达，便于在 Qt 信号槽中安全传递。
 */
#ifndef CONTROLHCZKPROTOCOL_H
#define CONTROLHCZKPROTOCOL_H

#include <QByteArray>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QtGlobal>

#include <limits>

namespace ControlHczkProtocol
{
// 船岸控制域命令。0xAA～0xAE 为船端上报，0x1A～0x1F 为岸基下发或链路命令。
enum class Command : quint16
{
    Unknown = 0x0000, // 未知或未配置的控制命令。

    // 岸基下发及船岸链路维护命令。
    Heartbeat = 0x1A00,
    MasterControl = 0x1A01,
    ControlMode = 0x1A02,
    EngineLimit = 0x1A03,
    ManualControl = 0x1A04,
    PilotControl = 0x1A05,
    NavigationBoxReboot = 0x1A06,
    AvoidCollision = 0x1A07,
    AvoidReset = 0x1A08,
    Route = 0x1B01,
    RouteClear = 0x1B02,
    RouteRequest = 0x1B03,
    RoutePointChange = 0x1B04,
    RouteControlParameter = 0x1B05,
    PositionControlParameter = 0x1B06,
    CreateVirtualTarget = 0x1B07,
    SpeedPid = 0x1C01,
    HeadingPid = 0x1C02,
    PidRequest = 0x1C03,
    ElectronicFence = 0x1D01,
    ElectronicFenceEnable = 0x1D02,
    ElectronicFenceRequest = 0x1D03,
    Formation = 0x1E01,
    Simulation = 0x1F01,

    // 船端状态上报及请求应答命令。
    NavigationBoxStatus = 0xAA01,
    NavigationInfo = 0xAA02,
    ThrustStatus = 0xAA03,
    SensorStatus = 0xAA04,
    NavigationControlStatus = 0xAA05,
    NavigationTrack = 0xAA06,
    PositionControlStatus = 0xAA07,
    AvoidCollisionStatus = 0xAB01,
    RouteResponse = 0xAB02,
    PidResponse = 0xAC03,
    ElectronicFenceResponse = 0xAD01,
    FormationDelayReport = 0xAE01,
    HandControlResponse = 0xBB03, // 海豚二号手持遥控操作结果应答。
    DelayRequest = 0xFFF1,
    DelayResponse = 0xFFFF
};

#pragma pack(push, 1)
// 控制权切换载荷。
struct MasterControlPayload { qint16 masterId; }; // masterId：目标控制站编号。

// 航行控制模式切换载荷。
struct ControlModePayload { qint16 mode; }; // mode：协议定义的控制模式枚举值。

// 四路推进设备的使能和最大油门限制。
struct EngineLimitPayload
{
    quint8 enabled[4]; // 各推进设备使能标志，0 表示禁用，1 表示启用。
    float maxThrottle[4]; // 各推进设备允许的最大油门开度。
};

// 两路舵角和四路油门的远程手动控制量。
struct ManualControlPayload
{
    float rudder[2]; // 左、右舵角指令，单位度。
    float throttle[4]; // 四路推进设备油门指令。
};

// 航迹跟踪点切换载荷。
struct RoutePointChangePayload { qint32 pointIndex; }; // pointIndex：目标航点索引。

// 空载荷用于心跳等没有数据域的命令，编码后长度为零。
struct EmptyPayload {};

struct NavigationBoxStatusPayload
{
    qint16 communicationStatus; // 船岸通信状态。
    qint16 masterId; // 当前拥有控制权的控制站编号。
    qint16 controlMode; // 当前航行控制模式。
};

struct NavigationInfoPayload
{
    qint64 timestampMs; // 导航数据时间，UTC 毫秒。
    double longitudeDeg; // 本船经度，单位度。
    double latitudeDeg; // 本船纬度，单位度。
    float altitudeM; // 海拔高度，单位 m。
    float headingDeg; // 船艏向，单位度。
    float cogDeg; // 对地航向，单位度。
    float sogKn; // 对地航速，单位 kn。
    float rateOfTurnDegPerMin; // 转向率，单位度/分钟。
    float rollDeg; // 横摇角，单位度。
    float pitchDeg; // 纵摇角，单位度。
    float windSpeed; // 风速，单位由船岸接口协议约定。
    float windAngleDeg; // 风向，单位度。
    float temperatureCelsius; // 环境温度，单位摄氏度。
    float humidityPercent; // 相对湿度，单位百分比。
    quint8 gnssStatus; // GNSS 定位质量状态。
};

struct ThrustStatusPayload
{
    float rudderCommandDeg[2]; // 左、右舵角指令，单位度。
    float rudderFeedbackDeg[2]; // 左、右舵角反馈，单位度。
    float throttleCommand[4]; // 四路油门指令。
    float throttleFeedback[4]; // 四路油门反馈。
    quint8 permitted[4]; // 四路推进设备控制许可标志。
    quint8 enabled[4]; // 四路推进设备使能状态。
    qint16 rpmCommand[4]; // 四路转速指令，单位 r/min。
    qint16 rpmFeedback[4]; // 四路实际转速，单位 r/min。
};

struct SensorStatusPayload
{
    quint8 gps1; // 主 GPS 在线状态。
    quint8 gps2; // 备用 GPS 在线状态。
    quint8 gyro; // 电罗经在线状态。
    quint8 vialog; // 计程仪在线状态。
    quint8 depth; // 测深仪在线状态。
    quint8 ais; // AIS 在线状态。
    quint8 arpa; // ARPA 在线状态。
    quint8 wind; // 风速风向仪在线状态。
    quint8 imu; // IMU 在线状态。
    quint8 reservedDevices[10]; // 预留设备状态，当前发送端填零。
};

struct NavigationControlStatusPayload
{
    quint8 automaticHeading; // 自动航向控制是否启用。
    quint8 headingType; // 航向控制来源或类型。
    float desiredHeadingDeg; // 期望航向，单位度。
    quint8 automaticSpeed; // 自动航速控制是否启用。
    quint8 speedType; // 航速控制来源或类型。
    float desiredSpeedKn; // 期望航速，单位 kn。
    quint8 automaticRoute; // 自动航迹跟踪是否启用。
    quint8 routeAvailable; // 当前是否存在可执行航线。
};

struct NavigationTrackPayload
{
    qint16 result; // 航迹计算结果状态。
    qint16 type; // 航迹跟踪类型。
    qint16 followIndex; // 当前跟踪航点索引。
    float distanceM; // 到目标航点距离，单位 m。
    float bearingDeg; // 目标航点方位，单位度。
    float crossTrackErrorM; // 横向偏航距离，单位 m。
    float bearingErrorDeg; // 航向误差，单位度。
    float arrivalDistanceM; // 航点到达判定半径，单位 m。
    float changeAngleDeg; // 航段切换角度，单位度。
    double longitudeDeg; // 当前参考点经度，单位度。
    double latitudeDeg; // 当前参考点纬度，单位度。
};

struct AvoidCollisionStatusPayload
{
    quint8 enabled; // 避碰功能启用状态。
    quint8 status; // 当前避碰状态。
    quint16 targetId; // 当前危险目标标识。
    float danger; // 危险度。
    quint8 strategy; // 当前采用的避碰策略。
};

// 链路延时请求/应答共同使用的时间戳载荷。
struct DelayPayload { quint64 timestampMs; }; // timestampMs：发送时刻，UTC 毫秒。

struct PositionControlStatusPayload
{
    double longitudeDeg; // 定点目标经度，单位度。
    double latitudeDeg; // 定点目标纬度，单位度。
    float innerRadiusM; // 内控制半径，单位 m。
    float outerRadiusM; // 外控制半径，单位 m。
    float speedGain; // 距离到速度的控制增益。
    float distanceErrorM; // 当前距离误差，单位 m。
    float desiredHeadingDeg; // 控制器输出期望航向，单位度。
    float desiredSpeedKn; // 控制器输出期望航速，单位 kn。
};

struct PidLoopPayload
{
    float kp; // 比例增益。
    float ki; // 积分增益。
    float kd; // 微分增益。
    float maxIntegral; // 积分项限幅。
    float maxOutput; // 控制输出限幅。
};

struct PidResponsePayload
{
    PidLoopPayload speedOuter; // 航速外环参数。
    PidLoopPayload speedInner; // 航速内环参数。
    PidLoopPayload headingOuter; // 航向外环参数。
    PidLoopPayload headingInner; // 航向内环参数。
};

struct RoutePointPayload
{
    quint16 index; // 航点索引。
    double longitudeDeg; // 航点经度，单位度。
    double latitudeDeg; // 航点纬度，单位度。
    float speedKn; // 计划航速，单位 kn。
    float radiusM; // 航点转弯或到达半径，单位 m。
};

struct FormationDelayItem
{
    quint32 shipId; // 编队成员船标识，当前使用 MMSI。
    qint64 delayMs; // 与成员船之间的测量延时，单位 ms。
};
#pragma pack(pop)

// 变长载荷使用 Qt 容器，避免把柔性数组和所有权暴露给库外。
struct RouteResponsePayload
{
    quint32 routeId = 0; // 航线编号。
    QByteArray name; // UTF-8 航线名称，编码时按协议定长写入。
    quint16 followIndex = 0; // 当前跟踪航点索引。
    QVector<RoutePointPayload> points; // 按执行顺序排列的航点。
};

struct ElectronicFencePoint
{
    double longitudeDeg = 0.0; // 围栏点经度，单位度。
    double latitudeDeg = 0.0; // 围栏点纬度，单位度。
};

struct ElectronicFenceItem
{
    QByteArray uuid; // 围栏唯一标识的线路字节。
    QByteArray name; // UTF-8 围栏名称。
    bool enabled = false; // 围栏是否启用。
    qint64 startTimeMs = 0; // 生效起始时间，UTC 毫秒。
    qint64 endTimeMs = 0; // 生效结束时间，UTC 毫秒。
    bool automatic = false; // 是否由系统自动启停。
    quint8 type = 0; // 围栏业务类型。
    QVector<ElectronicFencePoint> points; // 围栏边界点集合。
};

// 电子围栏查询应答，线路上按“围栏数量—围栏头—围栏点”顺序编码。
struct ElectronicFenceResponsePayload { QVector<ElectronicFenceItem> fences; };
// 编队链路延时上报，线路上先写成员数量，再顺序写入成员项。
struct FormationDelayReportPayload { QVector<FormationDelayItem> ships; };

static_assert(Q_BYTE_ORDER == Q_LITTLE_ENDIAN, "HCZK控制载荷当前约定使用小端字节序");
static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559,
              "HCZK控制载荷要求32位IEEE 754单精度浮点数");
static_assert(sizeof(double) == 8 && std::numeric_limits<double>::is_iec559,
              "HCZK控制载荷要求64位IEEE 754双精度浮点数");
static_assert(sizeof(NavigationBoxStatusPayload) == 6, "航控箱状态线协议尺寸不匹配");
static_assert(sizeof(NavigationInfoPayload) == 69, "本船状态线协议尺寸不匹配");
static_assert(sizeof(ThrustStatusPayload) == 72, "推进器状态线协议尺寸不匹配");
static_assert(sizeof(SensorStatusPayload) == 19, "传感器状态线协议尺寸不匹配");
static_assert(sizeof(NavigationControlStatusPayload) == 14, "自动控制状态线协议尺寸不匹配");
static_assert(sizeof(NavigationTrackPayload) == 46, "航迹状态线协议尺寸不匹配");
static_assert(sizeof(AvoidCollisionStatusPayload) == 9, "避碰状态线协议尺寸不匹配");
static_assert(sizeof(DelayPayload) == 8, "延时载荷线协议尺寸不匹配");
static_assert(sizeof(PositionControlStatusPayload) == 40, "定点位控状态线协议尺寸不匹配");
static_assert(sizeof(PidResponsePayload) == 80, "PID反馈线协议尺寸不匹配");
static_assert(sizeof(RoutePointPayload) == 26, "航点线协议尺寸不匹配");
static_assert(sizeof(FormationDelayItem) == 12, "编队延时项线协议尺寸不匹配");

using Payload = QVariant;

// 判断原始命令字是否属于控制域；不检查载荷长度。
bool isControlCommand(quint16 command);
// 判断控制命令是否已经建立 QVariant 强类型载荷映射。
bool isTypedCommand(quint16 command);
// 返回定长命令的载荷字节数；变长命令返回 -1，未知命令返回 -2。
int fixedPayloadSize(quint16 command);
// 校验命令与 QVariant 类型并序列化业务载荷，不包含 HCZK 公共帧头。
bool encodePayload(Command command, const Payload& payload, QByteArray* bytes,
                   QString* error = nullptr);
// 校验载荷长度并反序列化为命令对应的 QVariant 强类型对象。
bool decodePayload(quint16 command, const QByteArray& bytes, Payload* payload,
                   QString* error = nullptr);
}

Q_DECLARE_METATYPE(ControlHczkProtocol::Command)
Q_DECLARE_METATYPE(ControlHczkProtocol::EmptyPayload)
Q_DECLARE_METATYPE(ControlHczkProtocol::NavigationBoxStatusPayload)
Q_DECLARE_METATYPE(ControlHczkProtocol::NavigationInfoPayload)
Q_DECLARE_METATYPE(ControlHczkProtocol::ThrustStatusPayload)
Q_DECLARE_METATYPE(ControlHczkProtocol::SensorStatusPayload)
Q_DECLARE_METATYPE(ControlHczkProtocol::NavigationControlStatusPayload)
Q_DECLARE_METATYPE(ControlHczkProtocol::NavigationTrackPayload)
Q_DECLARE_METATYPE(ControlHczkProtocol::AvoidCollisionStatusPayload)
Q_DECLARE_METATYPE(ControlHczkProtocol::DelayPayload)
Q_DECLARE_METATYPE(ControlHczkProtocol::PositionControlStatusPayload)
Q_DECLARE_METATYPE(ControlHczkProtocol::PidResponsePayload)
Q_DECLARE_METATYPE(ControlHczkProtocol::RouteResponsePayload)
Q_DECLARE_METATYPE(ControlHczkProtocol::ElectronicFenceResponsePayload)
Q_DECLARE_METATYPE(ControlHczkProtocol::FormationDelayReportPayload)

#endif // CONTROLHCZKPROTOCOL_H
