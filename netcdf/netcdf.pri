# netCDF-C（仅 MSVC Release，与 ENC SDK 同一工具链）。
# 默认路径：仓库旁 ../../WeatherLib/netCDF4.9.3
# 可用 NETCDF_ROOT 覆盖。只链 netcdf/hdf5，不链 WeatherLib。

isEmpty(NETCDF_ROOT) {
    NETCDF_ROOT = $$clean_path($$PROJECT_ROOT/../../WeatherLib/netCDF4.9.3)
}

NETCDF_LIB = $$NETCDF_ROOT/lib/netcdf.lib
NETCDF_DLL = $$NETCDF_ROOT/bin/netcdf.dll

win32-msvc* {
    exists($$NETCDF_LIB):exists($$NETCDF_DLL) {
        CONFIG(release, debug|release) {
            DEFINES += SHORE_HAS_NETCDF
            INCLUDEPATH += $$NETCDF_ROOT/include
            LIBS += $$shell_path($$NETCDF_LIB)
            NETCDF_BIN = $$NETCDF_ROOT/bin
            NETCDF_DLL_DST = $$shell_path($$BIN_OUTPUT_DIR)
            QMAKE_POST_LINK += $$escape_expand(\\n\\t) $$QMAKE_COPY /Y $$shell_quote($$shell_path($$NETCDF_BIN/netcdf.dll)) $$shell_quote($$NETCDF_DLL_DST)
            QMAKE_POST_LINK += $$escape_expand(\\n\\t) $$QMAKE_COPY /Y $$shell_quote($$shell_path($$NETCDF_BIN/hdf5.dll)) $$shell_quote($$NETCDF_DLL_DST)
            QMAKE_POST_LINK += $$escape_expand(\\n\\t) $$QMAKE_COPY /Y $$shell_quote($$shell_path($$NETCDF_BIN/hdf5_hl.dll)) $$shell_quote($$NETCDF_DLL_DST)
            QMAKE_POST_LINK += $$escape_expand(\\n\\t) $$QMAKE_COPY /Y $$shell_quote($$shell_path($$NETCDF_BIN/zlib1.dll)) $$shell_quote($$NETCDF_DLL_DST)
            QMAKE_POST_LINK += $$escape_expand(\\n\\t) $$QMAKE_COPY /Y $$shell_quote($$shell_path($$NETCDF_BIN/libcurl.dll)) $$shell_quote($$NETCDF_DLL_DST)
            message(netCDF: $$NETCDF_ROOT (MSVC Release))
        } else {
            message(netCDF skipped in Debug)
        }
    } else {
        warning(netCDF not found at $$NETCDF_ROOT)
    }
}
