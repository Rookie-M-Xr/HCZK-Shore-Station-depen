#pragma once

#include "EncSdkTypes.h"
#include "EncSdkFeatureDepthInfo.h"
#include "EncViewerSdkGlobal.h"

#include <QObject>
#include <QStringList>

#include <optional>
struct EncSdkLayerTypeInfo;

/**
 * @brief SDK ENC 数据管理门面。
 *
 * 统一提供 ENC 导入、Cell 管理、完整 WGS84 几何和离散测深查询，不暴露数据库操作。
 * @note 仅在 EncSdk 初始化成功后、shutdown 前使用；不保证所有接口可任意跨线程调用。
 * @note 首次获取实例和加载缓存目录必须在应用主线程执行。
 */
class ENCVIEWER_SDK_EXPORT EncDataManager final : public QObject
{
    Q_OBJECT

signals:
    /** @brief 导入诊断日志，仅供显示；文本内容不是稳定的机器解析协议。 */
    void importLogMessage(const QString &message);

    /** @brief 缓存导入文件进度；current 为已处理数，total 为待生成缓存文件数，不包含 SQL 阶段。 */
    void importProgressChanged(int current, int total);

    /**
     * @brief 一次实际启动的导入任务结束，此时可调用 cells() 获取列表。
     * @param mode 导入模式。
     * @param importedFiles 成功生成缓存的文件数。
     * @param failedFiles 缓存生成失败的文件数。
     * @param skippedFiles 前置过滤的无效、重复或未变化输入数量，不包含 SQL 暂缓任务。
     * @param totalFeatures 本次成功生成缓存的 Feature 总数。
     * @note 统计不受 SQL 成败影响；若内部启用 SQL，信号仍等待整个任务结束。
     */
    void importFinished(EncSdkImportMode mode,
                        int importedFiles,
                        int failedFiles,
                        int skippedFiles,
                        int totalFeatures);

public:
    /**
     * @brief 返回进程内唯一的数据管理门面。
     * @return SDK 管理的实例引用；调用方不得删除、设置父对象或移动其线程。
     */
    static EncDataManager &instance();

    /** @brief 返回初始化时确定的主缓存目录；属性数据库固定为该目录下的 cache.sqlite。 */
    QString cacheDirectory() const;

    /**
     * @brief 在当前主缓存目录基础上追加加载指定缓存目录。
     * @param directoryPath 已存在的缓存目录，不是原始 ENC 文件目录；空路径或不存在时失败。
     * @param errorMessage 可选错误输出；成功时清空，失败时填写原因。
     * @param loadSynchronously 默认为 false，后台扫描；true 在当前调用中扫描，可能阻塞界面。
     * @return 目录已接受或已在列表中时返回 true；异步返回成功不表示扫描完成或所有缓存有效。
     * @note 必须在应用主线程调用；不会切换主缓存目录或主数据库，也不加载追加目录中的数据库。
     */
    bool loadCacheDirectory(const QString &directoryPath,
                            QString *errorMessage = nullptr,
                            bool loadSynchronously = false);

    /** @brief 返回主缓存目录及全部追加加载目录，主目录位于首项。 */
    QStringList cacheDirectories() const;

    /**
     * @brief 同步递归扫描目录中的 `.000` 文件，不解析海图。
     * @param directoryPath 原始 ENC 文件目录。
     * @return 按路径排序、路径去重后的绝对路径列表；未找到文件时为空。
     * @note 大目录扫描可能阻塞当前线程；文件在导入阶段才验证内容。
     */
    QStringList collectChartFiles(const QString &directoryPath) const;

    /**
     * @brief 检查指定文件并启动后台导入，已有导入任务时拒绝新请求。
     * @param filePaths ENC 文件路径列表；无效、路径重复或内容重复文件会被过滤。
     * @param mode 默认为 Append；Rebuild 会先清理主缓存，详见 EncSdkImportMode。
     * @param errorMessage 可选启动错误输出；成功时清空。后台错误通过日志和完成结果报告。
     * @return 请求被接受或没有新文件需要处理时为 true，不代表全部文件导入成功。
     * @note 必须在应用主线程调用；前置检查同步执行，实际解析在后台执行。
     *       实际启动的任务通过 importFinished 报告结果；全部跳过且无待恢复任务时不发射该信号。
     */
    bool importChartFiles(const QStringList &filePaths,
                          EncSdkImportMode mode = EncSdkImportMode::Append,
                          QString *errorMessage = nullptr);

    /**
     * @brief 在后台递归扫描并去重目录中的 ENC 文件，再启动导入。
     * @param directoryPath 原始 ENC 文件目录；不存在时同步失败，空目录在扫描完成后报告失败。
     * @param mode 默认为 Append；Rebuild 会先清理主缓存。
     * @param errorMessage 可选启动错误输出；成功时清空，不承载后台导入错误。
     * @return 请求被接受时为 true，不代表扫描或导入完成。
     * @note 必须在应用主线程调用；扫描期间 isImportRunning 返回 true 并拒绝重复导入。
     *       接受的请求通过 importFinished 报告终态，包括空目录失败和全部文件跳过。
     */
    bool importChartDirectory(const QString &directoryPath,
                              EncSdkImportMode mode = EncSdkImportMode::Append,
                              QString *errorMessage = nullptr);

    /** @brief 返回目录导入准备或导入协调器是否正在运行，不表示其他缓存扫描任务是否完成。 */
    bool isImportRunning() const;

    /** @brief 返回当前已索引且未忽略的 Cell 元数据快照，按 fileName、cellName 排序；不扫描磁盘。 */
    QList<EncSdkCellInfo> cells() const;

    /** @brief 按 ID 返回 Cell 元数据快照，不存在或本次运行已忽略时返回空值。 */
    std::optional<EncSdkCellInfo> cellById(qint64 cellId) const;

    /** @brief 按输入顺序返回 Cell 快照；跳过不存在或已忽略的 ID，保留重复 ID 对应的重复结果。 */
    QList<EncSdkCellInfo> cellsByIds(const QList<qint64> &cellIds) const;

    /** @brief 合并当前 Cell 的 WGS84 包围盒，无有效范围时返回空矩形；跨日期变更线时可能得到较大范围。 */
    QRectF cellExtent() const;

    /**
     * @brief 从本次 SDK 生命周期中移除指定 Cell，可选删除本地缓存。
     * @param cellId 当前已索引的 Cell ID。
     * @param deleteLocalCache 默认为 false，仅本次忽略，保留缓存和数据库，重启后恢复；
     *        true 另删除主目录对应 cache、idx、wd 和数据库记录，不删除原始 ENC。
     * @param errorMessage 可选错误输出，成功时清空，失败时填写原因。
     * @return 成功返回 true；不存在、导入中或试图删除追加目录缓存时返回 false。
     * @note 必须在应用主线程调用；忽略状态不写入磁盘，不会因目录监听或重扫描恢复。
     *       磁盘删除失败可能已完成部分删除，不承诺回滚。
     */
    bool removeCell(qint64 cellId,
                    bool deleteLocalCache = false,
                    QString *errorMessage = nullptr);

    /** @brief 禁止旧参数顺序，防止错误字符串指针隐式转换为 true 而删除磁盘缓存。 */
    bool removeCell(qint64 cellId, QString *errorMessage) = delete;

    /**
     * @brief 同步查询指定 Cell 的 Feature 身份和完整 WGS84 几何，不依赖 SQLite。
     * @param cellId 正 Cell ID。
     * @param errorMessage 失败原因；进入查询时清空。
     * @return 无匹配或已忽略 Cell 返回空集合；失败返回空集合并设置错误。
     * @note 从磁盘 cache 解码，不受当前视口、图层开关或地图 CRS 影响；全量查询可能耗时。
     */
    QList<EncSdkFeatureInfo> featuresByCell(qint64 cellId,
                                           QString *errorMessage = nullptr) const;

    /**
     * @brief 按 Cell 和 S-57 对象类查询完整 WGS84 几何，查询及错误约定同 featuresByCell()。
     * @param cellId Cell 唯一 ID。
     * @param objectClass S-57 对象类 acronym，例如 `DEPARE`；去除首尾空白并转大写。
     */
    QList<EncSdkFeatureInfo> featuresByCellAndObjectClass(
        qint64 cellId,
        const QString &objectClass,
        QString *errorMessage = nullptr) const;

    /**
     * @brief 按 Cell 和精确图层查询完整 WGS84 几何，查询及错误约定同 featuresByCell()。
     * @param cellId Cell 唯一 ID。
     * @param acronym S-57 对象类缩写。
     * @param primitive Point、Line 或 Area 图层类型。
     */
    QList<EncSdkFeatureInfo> featuresByCellAndLayer(
        qint64 cellId,
        const QString &acronym,
        EncSdkPrimitive primitive,
        QString *errorMessage = nullptr) const;

    /**
     * @brief 批量查询多个 Cell、多个精确图层的完整 WGS84 几何，不依赖 SQLite。
     * @param cellIds Cell 唯一 ID 集合。
     * @param layers 图层集合，仅使用 acronym 和 primitive 字段参与筛选。
     * @param errorMessage 查询失败时接收错误信息。
     * @return 按 Cell ID、对象类、图元类型和 Feature ID 排序；重复条件不重复返回。
     * @note 任一缓存读取失败返回空集合并设置错误，不返回部分成功结果。空筛选集合非法。
     */
    QList<EncSdkFeatureInfo> featuresByCellsAndLayers(
        const QList<qint64> &cellIds,
        const QList<EncSdkLayerFilter> &layers,
        QString *errorMessage = nullptr) const;


    /**
     * @brief 按正 ID 查询单个 Feature 的完整 WGS84 几何，不依赖 SQLite。
     * @param featureId Feature 唯一 ID。
     * @param errorMessage 失败原因；进入查询时清空。
     * @return 无匹配返回空值；失败返回空值并设置错误。
     * @note 同步逐 Cell 搜索磁盘 cache，可能耗时；忽略的 Cell 不参与查询。
     */
    std::optional<EncSdkFeatureInfo> featureById(qint64 featureId,
                                                 QString *errorMessage = nullptr) const;

    /**
     * @brief 查询指定经纬度附近最近的 SOUNDG 离散测深点。
     * @param longitude WGS84 经度，范围 [-180, 180]。
     * @param latitude WGS84 纬度，范围 [-90, 90]。
     * @param maximumDistanceMeters 最大搜索距离，单位为米。
     * @param errorMessage 可选错误输出，进入查询时清空；非法参数或缓存读取失败时填写原因。
     * @return 找到时返回测深点、水深和距离；无匹配返回空值且无错误，失败返回空值及错误。
     * @note 同步查询当前未忽略 Cell 的测深缓存，可能触发磁盘读取；距离必须为有限正数。
     *       返回测深点记录，不是查询位置的插值水深，不做潮位修正。
     */
    std::optional<EncSdkWaterDepth> nearestWaterDepth(
        double longitude,
        double latitude,
        double maximumDistanceMeters,
        QString *errorMessage = nullptr) const;

    /**
     * @brief 查询距离指定经纬度最近的全局 SOUNDG 离散测深点。
     * @param longitude WGS84 经度，范围 [-180, 180]。
     * @param latitude WGS84 纬度，范围 [-90, 90]。
     * @param errorMessage 可选错误输出，语义同限定距离的重载。
     * @return 当前已索引且未忽略 Cell 中的最近测深点；无匹配返回空值且无错误。
     * @note 同步查询可能读取全部测深缓存；不限制距离、不插值、不做潮位修正。
     */
    std::optional<EncSdkWaterDepth> nearestWaterDepth(double longitude,
                                                      double latitude,
                                                      QString *errorMessage = nullptr) const;

    /**
     * @brief 同步重新扫描全部已注册缓存目录，提供独立于旧加载接口的就绪检查。
     * @param errorMessage 成功时清空；目录不可读、索引损坏或配套 cache/wd 缺失时返回原因。
     * @return 完成扫描并发布索引时为 true；失败不发布本次不完整扫描结果。
     * @note SDK 初始化后在应用主线程调用，导入或内部批量删除期间拒绝调用。
     *       主目录和已注册目录也会重新扫描；新增目录仍用 loadCacheDirectory 注册。
     *       扫描与已有后台扫描串行，旧扫描不会在本次之后发布先前的索引快照。
     *       检查索引和文件存在/可读性，不预解码所有 cache/wd；内容错误在实际查询时报告。
     *       不修改旧 loadCacheDirectory 的行为，不保证外部进程同时改写文件的一致性。
     */
    bool reloadCacheDirectories(QString *errorMessage = nullptr);

    /**
     * @brief 批量返回 Feature 的完整几何及 DEPARE/SOUNDG 水深，同一 Cell 只读取一次 cache。
     * @param cellIds 正 Cell ID 集合；重复 ID 去重，不存在或已忽略的 Cell 无匹配结果。
     * @param layers 精确图层条件，缩写去空白并转大写，图元必须明确，重复条件去重。
     * @param errorMessage 进入时清空；非法参数、读取失败或读取期间观察到 Cell 移除时设置原因。
     * @return 按 Cell ID、对象类、图元、Feature ID 排序；无匹配为空且无错误，
     *         任一读取失败整批为空并设置错误；属性缺失以结果中的 optional 空值表达。
     * @note 主线程初始化并首次获取本实例后，可并发执行后台只读查询。
     *       建议先 reloadCacheDirectories，生成期间宿主应禁止导入、移除、追加目录和 shutdown。
     *       身份、双精度 WGS84 几何和水深来自同次 Cell 快照，与视口、图层开关及绘制 CRS 无关；
     *       多 Cell 不是全库事务，不承诺外部热覆盖安全。返回值可独立持有，不借用 SDK 内存。
     */
    QList<EncSdkFeatureDepthInfo> featuresWithDepthByCellsAndLayers(
        const QList<qint64> &cellIds,
        const QList<EncSdkLayerFilter> &layers,
        QString *errorMessage = nullptr) const;

private:
    friend class EncDataManagerTestAccess;
    /** @brief 内部图层名称目录，不属于 SDK 公共查询接口。 */
    QList<EncSdkLayerTypeInfo> layerTypes() const;
    /** @brief 构造并连接内部数据管理器。 */
    EncDataManager();

    Q_DISABLE_COPY(EncDataManager)
    EncDataManager(EncDataManager &&) = delete;
    EncDataManager &operator=(EncDataManager &&) = delete;
};
