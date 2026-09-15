# 预编译库按 Qt 版本 + qmake spec 分目录。
# 优先：<root>/6.11.1/win32-msvc
# 兼容旧布局：<root>/win32-g++
DEPEN_KIT_PRI_LOADED = 1
DEPEN_QT_VER = $$QT_VERSION
DEPEN_SPEC = $$[QMAKE_XSPEC]

defineReplace(depenKitDir) {
    root = $$1
    spec = $$[QMAKE_XSPEC]
    kit = $$root/$$QT_VERSION/$$spec
    legacy = $$root/$$spec
    exists($$kit/MANIFEST.txt) {
        return($$kit)
    }
    exists($$legacy/libqcustomplot.a)|exists($$legacy/libqcustomplotd.a)|exists($$legacy/qcustomplot.lib)|exists($$legacy/qcustomplotd.lib) {
        return($$legacy)
    }
    exists($$legacy/libQt6Mqtt.a)|exists($$legacy/Qt6Mqtt.lib)|exists($$legacy/Qt6Mqttd.lib) {
        return($$legacy)
    }
    return()
}
