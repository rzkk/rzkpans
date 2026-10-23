file(GLOB_RECURSE PANS_SOURCES CONFIGURE_DEPENDS *.cpp *.cc *.c)
file(GLOB_RECURSE PANS_HEADERS CONFIGURE_DEPENDS *.h *.hpp)


add_library(pans STATIC ${PANS_SOURCES} ${PANS_HEADERS})  # 生成 libpans.a

target_include_directories(pans
    PUBLIC   # pans 公开的头文件
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE  # pans 不公开的头文件
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(pans PUBLIC pans_options) # 继承所有编译/链接选项


# 主库可能需要更严格的编译选项
target_compile_options(pans PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
        -Werror          # 将警告视为错误（主库严格要求）
    >
)

# 库的安装配置
install(TARGETS pans
    EXPORT pansTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${PANS_HEADERS} 
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pans
)

message(STATUS "--------37 ${CMAKE_INSTALL_INCLUDEDIR}")

