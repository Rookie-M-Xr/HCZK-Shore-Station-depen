/**
 * @file SensorHczkProtocol.h
 * @brief 定义传感器域命令、线路载荷及其 Qt 元类型编解码接口。
 *
 * 固定布局结构直接对应线协议，字段顺序和字节宽度不得随意调整；无效测量值
 * 使用本文件定义的协议哨兵，禁止将 NaN 直接写入线路载荷。
 */
#ifndef SENSORHCZKPROTOCOL_H
#define SENSORHCZKPROTOCOL_H

#include <QByteArray>
#include <QString>
#include <QVariant>
#include <QtGlobal>

#include <limits>

namespace SensorHczkProtocol
{
// 定义传感器与航行控制状态使用的 HCZK 命令字。
enum class Command : quint16
{
    Unknown = 0x0000, // 未知或无效指令。
    Ais = 0xAC01, // AIS 目标。
    Arpa = 0xAC02, // ARPA 目标。
    Gps = 0xAC03, // GPS 导航数据。
    Gyro = 0xAC04, // 电罗经数据。
    Depth = 0xAC05, // 测深数据。
    Wind = 0xAC06, // 风速风向数据。
    Vialog = 0xAC07, // 计程仪数据。
    Tilt = 0xAC08, // 船体倾斜数据。
    Visibility = 0xAC09, // 能见度数据。
    Imu = 0xAC0A // 六自由度数据。
};
constexpr float InvalidFloat = -9999.0F; // HCZK浮点字段无真实有效值时使用的唯一线协议哨兵，仅允许在协议边界出现。

#pragma pack(push, 1)
// 连续型传感器在线状态和单字节设备编号。
struct DeviceInfo
{
    quint8 online; // 设备在线状态：0表示离线，1表示在线。
    quint8 id; // 设备编号。
};

// AIS载荷。当前编解码基线每帧承载一个目标，后续批量快照由发布层扩展。
struct AisPayload
{
    quint8 targetClass; // 船舶AIS分类。
    char name[21]; // 船名，未使用部分填零。
    quint32 mmsi; // 海上移动业务标识码。
    float sogKn; // 对地航速，单位kn。
    float headingDeg; // 船艏向，单位度。
    float rateOfTurnDegPerMin; // 转向率，单位度/分钟。
    float cogDeg; // 对地航向，单位度。
    float lengthM; // 船长，单位m。
    float widthM; // 船宽，单位m。
    double latitudeDeg; // 纬度，单位度。
    double longitudeDeg; // 经度，单位度。
    quint64 timestampMs; // 目标更新时间，UTC毫秒。
};

// ARPA载荷。每帧承载一个目标，状态L用于通知目标丢失。
struct ArpaPayload
{
    char status; // 目标状态：L、Q或T。
    char acquisitionType; // 捕获类型：A、M或R。
    quint8 targetId; // 雷达目标编号。
    float speedKn; // 目标航速，单位kn。
    float courseDeg; // 目标航向，单位度。
    float bearingDeg; // 目标方位，单位度。
    float distanceNm; // 目标距离，单位nm。
    float dcpaNm; // 最小会遇距离，单位nm。
    float tcpaMin; // 最小会遇时间，单位分钟。
    float danger; // 危险度，值域0～1。
    double latitudeDeg; // 纬度，单位度。
    double longitudeDeg; // 经度，单位度。
    quint64 timestampMs; // 目标更新时间，UTC毫秒。
};

// GPS 定位、运动和授时载荷。
struct GpsPayload
{
    DeviceInfo device; // GPS设备状态和编号。
    double latitudeDeg; // 纬度，单位度。
    double longitudeDeg; // 经度，单位度。
    float sogKn; // 对地航速，单位kn。
    float cogDeg; // 对地航向，单位度。
    quint64 timestampMs; // 定位时间，UTC毫秒。
    quint8 satelliteCount; // 参与定位的卫星数量。
};

// 电罗经航向和转向率载荷。
struct GyroPayload
{
    DeviceInfo device; // 罗经设备状态和编号。
    float headingDeg; // 船艏向，单位度。
    float rateOfTurnDegPerMin; // 转向率，单位度/分钟；未提供ROT时为InvalidFloat。
};

// 测深仪海深和吃水载荷。
struct DepthPayload
{
    DeviceInfo device; // 测深设备状态和编号。
    float seaDepthM; // 海底深度，单位m；无真实有效值时为InvalidFloat。
    float draftM; // 船舶吃水深度，单位m；无真实有效值时为InvalidFloat。
};

// 风速风向及气象要素载荷。
struct WindPayload
{
    DeviceInfo device; // 风速风向设备状态和编号。
    float relativeDirectionDeg; // 相对风向，单位度；无真实有效值时为InvalidFloat。
    float trueDirectionDeg; // 真风向，单位度；无真实有效值时为InvalidFloat。
    float relativeSpeedMps; // 相对风速，单位m/s；无真实有效值时为InvalidFloat。
    float trueSpeedMps; // 真风速，单位m/s；无真实有效值时为InvalidFloat。
    float relativeHumidityPercent; // 相对湿度，单位百分比；无真实有效值时为InvalidFloat。
    float trueHumidityPercent; // 协议定义的绝对湿度，单位百分比；无真实有效值时为InvalidFloat。
    float dewPointCelsius; // 露点温度，单位摄氏度；无真实有效值时为InvalidFloat。
    float pressure; // 气压，单位按协议约定；无真实有效值时为InvalidFloat。
    float airTemperatureCelsius; // 空气温度，单位摄氏度；无真实有效值时为InvalidFloat。
};

// 计程仪纵横向速度和累计里程载荷。
struct VialogPayload
{
    DeviceInfo device; // 计程仪设备状态和编号。
    float sogTransverseKn; // 横向对地航速，单位kn；无真实有效值时为InvalidFloat。
    float sogLongitudinalKn; // 纵向对地航速，单位kn；无真实有效值时为InvalidFloat。
    float stwTransverseKn; // 横向对水航速，单位kn；无真实有效值时为InvalidFloat。
    float stwLongitudinalKn; // 纵向对水航速，单位kn；无真实有效值时为InvalidFloat。
    float sternStwTransverseKn; // 船尾横向对水航速，单位kn；无真实有效值时为InvalidFloat。
    float sternSogTransverseKn; // 船尾横向对地航速，单位kn；无真实有效值时为InvalidFloat。
    float totalWaterMilesNm; // 累计对水里程，单位nm；无真实有效值时为InvalidFloat。
    float resetWaterMilesNm; // 清零后的对水里程，单位nm；无真实有效值时为InvalidFloat。
    float totalGroundMilesNm; // 累计对地里程，单位nm；无真实有效值时为InvalidFloat。
    float resetGroundMilesNm; // 清零后的对地里程，单位nm；无真实有效值时为InvalidFloat。
};

// 船体横摇、纵摇和周期载荷。
struct TiltPayload
{
    DeviceInfo device; // 倾斜仪设备状态和编号。
    float heelDeg; // 横倾角，单位度。
    float heelPeriodS; // 横摇周期，单位秒。
    float portHeelAmplitudeDeg; // 左舷横倾幅度，单位度。
    float starboardHeelAmplitudeDeg; // 右舷横倾幅度，单位度。
    float pitchDeg; // 纵摇角，单位度。
};

// 能见度测量载荷。
struct VisibilityPayload
{
    DeviceInfo device; // 能见度仪设备状态和编号。
    float visibilityM; // 能见距离，单位m。
    qint16 type; // 能见度数据类型。
};

// 六自由度运动测量载荷。
struct ImuPayload
{
    DeviceInfo device; // 六自由度设备状态和编号。
    float surge; // 纵荡，沿X轴平移。
    float sway; // 横荡，沿Y轴平移。
    float heave; // 垂荡，沿Z轴平移。
    float rollDeg; // 横摇角，单位度。
    float pitchDeg; // 纵摇角，单位度。
    float yawDeg; // 艏摇角，单位度。
};

#pragma pack(pop)

static_assert(sizeof(DeviceInfo) == 2, "DeviceInfo线协议尺寸必须为2字节");
static_assert(Q_BYTE_ORDER == Q_LITTLE_ENDIAN, "HCZK传感器载荷当前约定使用小端字节序");
static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559,
              "HCZK传感器载荷要求32位IEEE 754单精度浮点数");
static_assert(sizeof(double) == 8 && std::numeric_limits<double>::is_iec559,
              "HCZK传感器载荷要求64位IEEE 754双精度浮点数");
static_assert(sizeof(AisPayload) == 74, "AIS线协议尺寸不匹配");
static_assert(sizeof(ArpaPayload) == 55, "ARPA线协议尺寸不匹配");
static_assert(sizeof(GpsPayload) == 35, "GPS线协议尺寸不匹配");
static_assert(sizeof(GyroPayload) == 10, "罗经线协议尺寸不匹配");
static_assert(sizeof(DepthPayload) == 10, "测深线协议尺寸不匹配");
static_assert(sizeof(WindPayload) == 38, "风速风向线协议尺寸不匹配");
static_assert(sizeof(VialogPayload) == 42, "计程仪线协议尺寸不匹配");
static_assert(sizeof(TiltPayload) == 22, "倾斜仪线协议尺寸不匹配");
static_assert(sizeof(VisibilityPayload) == 8, "能见度线协议尺寸不匹配");
static_assert(sizeof(ImuPayload) == 26, "六自由度线协议尺寸不匹配");

using Payload = QVariant;

// 判断指令号是否属于传感器业务指令；本函数不校验HCZK帧。
bool isSensorCommand(quint16 command);

// 返回指定传感器指令的固定载荷长度，未知指令返回0。
int payloadSize(quint16 command);

// 返回载荷类型对应的传感器业务指令号。
Command commandFor(const Payload& payload);

// 校验并序列化传感器业务载荷；输出字节由调用方持有，不生成HCZK帧。
bool encodePayload(const Payload& payload, QByteArray* bytes, QString* error = nullptr);

// 按指令校验并反序列化传感器业务载荷；输入字节由调用方持有，不校验HCZK帧。
bool decodePayload(quint16 command, const QByteArray& bytes, Payload* payload, QString* error = nullptr);
}

Q_DECLARE_METATYPE(SensorHczkProtocol::AisPayload)
Q_DECLARE_METATYPE(SensorHczkProtocol::ArpaPayload)
Q_DECLARE_METATYPE(SensorHczkProtocol::GpsPayload)
Q_DECLARE_METATYPE(SensorHczkProtocol::GyroPayload)
Q_DECLARE_METATYPE(SensorHczkProtocol::DepthPayload)
Q_DECLARE_METATYPE(SensorHczkProtocol::WindPayload)
Q_DECLARE_METATYPE(SensorHczkProtocol::VialogPayload)
Q_DECLARE_METATYPE(SensorHczkProtocol::TiltPayload)
Q_DECLARE_METATYPE(SensorHczkProtocol::VisibilityPayload)
Q_DECLARE_METATYPE(SensorHczkProtocol::ImuPayload)

#endif // SENSORHCZKPROTOCOL_H
