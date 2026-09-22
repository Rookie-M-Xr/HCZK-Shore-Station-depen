# netCDF-C 4.9.3（仅 MSVC Release）。
# 默认：depen/netcdf/4.9.3/<qmake-spec>/
# 可用 NETCDF_ROOT 覆盖。只链 netcdf，运行时再带 hdf5 / zlib / libcurl。

isEmpty(DEPEN_KIT_PRI_LOADED): include($$PWD/../kit.pri)

isEmpty(NETCDF_ROOT) {
    NETCDF_ROOT = $$clean_path($$PWD/4.9.3/$$[QMAKE_XSPEC])
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
