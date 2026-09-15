/**
 * @file DecisionHczkProtocol.h
 * @brief 定义决策规划域命令、固定/变长载荷及编解码接口。
 *
 * 变长航线和目标列表使用 QVector 管理内存，避免将柔性数组及所有权暴露到库外。
 */
#ifndef DECISIONHCZKPROTOCOL_H
#define DECISIONHCZKPROTOCOL_H

#include <QByteArray>
#include <QString>
#include <QtGlobal>
#include <QVariant>
#include <QVector>

namespace DecisionHczkProtocol
{
// 定义决策规划模块使用的 HCZK 命令字。
enum class Command : quint16
{
    SetGoal = 0x5A01, // 设置规划目标点。
    SetPath = 0x1B01, // 向规划器下发航线。
    LidarTargets = 0x5A03, // 激光雷达目标列表。
    WaveRadarTargets = 0x5A04, // 波浪雷达目标列表。
    NavigationRadarTargets = 0x5A05, // 导航雷达目标列表。
    UavTargets = 0x5A06, // 无人机目标列表。
    PlannerState = 0x5B01, // 规划器状态。
    RouteRequest = 0x1B03, // 规划器请求当前航线。
    GlobalRoute = 0x5B03, // 全局规划航线结果。
    LocalSpeedHead = 0x5B04, // 局部规划航速航向结果。
    LocalRoute = 0x5B05 // 局部规划航线结果。
};

#pragma pack(push, 1)
// 设置规划目标点的固定长度载荷。
struct SetGoalPayload
{
    double longitudeDeg; // 目标经度，单位度。
    double latitudeDeg; // 目标纬度，单位度。
};

// 单个障碍物目标的线路结构。
struct TargetInfo
{
    qint16 id; // 目标编号。
    double x; // 目标 X 坐标，单位和坐标系由 coordinateType 约定。
    double y; // 目标 Y 坐标，单位和坐标系由 coordinateType 约定。
    double z; // 目标 Z 坐标，单位和坐标系由 coordinateType 约定。
    float headingDeg; // 目标航向，单位度。
    float speedMps; // 目标速度，单位 m/s。
    float lengthM; // 目标长度，单位 m。
    float widthM; // 目标宽度，单位 m。
    float heightM; // 目标高度，单位 m。
};

// 目标列表的变长载荷头。
struct TargetListHeader
{
    qint64 timestampMs; // 目标数据时间，UTC 毫秒。
    qint16 coordinateType; // 坐标系类型。
    qint16 targetCount; // 紧随其后的目标数量。
};

// 规划器运行状态载荷。
struct PlannerStatusPayload
{
    quint8 status; // 规划器状态枚举值。
};

// 局部规划器输出的航速和航向载荷。
struct LocalSpeedHeadPayload
{
    float decisionSpeedKn; // 决策航速，单位 kn。
    float decisionHeadingDeg; // 决策航向，单位度。
};

// 航线变长载荷头。
struct RouteHeaderPayload
{
    quint32 id; // 航线编号。
    char name[16]; // UTF-8 航线名称，必须以零结尾。
    quint16 followIndex; // 当前跟踪航点索引。
    quint16 pointCount; // 紧随其后的航点数量。
};

// 单个航点的线路结构。
struct RoutePointPayload
{
    quint16 index; // 航点索引。
    double longitudeDeg; // 航点经度，单位度。
    double latitudeDeg; // 航点纬度，单位度。
    float speedKn; // 计划航速，单位 kn。
    float turnRadiusM; // 转弯半径，单位 m。
};
#pragma pack(pop)

// 表示无业务字段的航线请求，避免应用层依赖空字节数组。
struct RouteRequestPayload
{
};

// 航线是变长载荷，在线路头之后紧跟 pointCount 个航点。
struct RoutePayload
{
    RouteHeaderPayload header{}; // 航线元数据。
    QVector<RoutePointPayload> points; // 按线路顺序排列的航点。
};

// 目标列表是变长载荷。sourceCommand 用于区分激光雷达、波浪雷达等来源。
struct TargetListPayload
{
    Command sourceCommand = Command::LidarTargets; // 标识目标数据来源。
    TargetListHeader header{}; // 目标列表元数据。
    QVector<TargetInfo> targets; // 本帧包含的目标集合。
};

using Payload = QVariant;

static_assert(sizeof(SetGoalPayload) == 16, "决策目标点载荷尺寸错误");
static_assert(sizeof(PlannerStatusPayload) == 1, "规划器状态载荷尺寸错误");
static_assert(sizeof(LocalSpeedHeadPayload) == 8, "局部航速航向载荷尺寸错误");
static_assert(sizeof(RouteHeaderPayload) == 24, "航线载荷头尺寸错误");
static_assert(sizeof(RoutePointPayload) == 26, "航点载荷尺寸错误");

// 判断命令字是否属于决策模块，不检查载荷内容。
bool isDecisionCommand(quint16 command);

// 检查命令字与线路载荷长度是否匹配；error 可为空且由调用方持有。
bool validatePayload(quint16 command, const QByteArray& payload, QString* error = nullptr);

// 将已通过帧校验的线路载荷解析为强类型对象；不保留任何输出指针。
bool decodePayload(quint16 command, const QByteArray& bytes, Payload* payload, QString* error = nullptr);

// 将强类型对象序列化为线路载荷，并校验命令与对象类型是否匹配。
bool encodePayload(Command command, const Payload& payload, QByteArray* bytes, QString* error = nullptr);

// 将固定布局对象复制为线路字节，仅用于满足紧凑布局约束的协议结构。
template<typename Payload>
QByteArray encodePod(const Payload& payload)
{
    return QByteArray(reinterpret_cast<const char*>(&payload), sizeof(Payload));
}
}

Q_DECLARE_METATYPE(DecisionHczkProtocol::SetGoalPayload)
Q_DECLARE_METATYPE(DecisionHczkProtocol::RoutePayload)
Q_DECLARE_METATYPE(DecisionHczkProtocol::TargetListPayload)
Q_DECLARE_METATYPE(DecisionHczkProtocol::PlannerStatusPayload)
Q_DECLARE_METATYPE(DecisionHczkProtocol::RouteRequestPayload)
Q_DECLARE_METATYPE(DecisionHczkProtocol::LocalSpeedHeadPayload)
Q_DECLARE_METATYPE(DecisionHczkProtocol::Command)

#endif // DECISIONHCZKPROTOCOL_H
