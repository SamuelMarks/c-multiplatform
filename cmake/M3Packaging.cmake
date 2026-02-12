set(M3_PACKAGING_ROOT "${CMAKE_CURRENT_LIST_DIR}/../packaging")
get_filename_component(M3_PACKAGING_ROOT "${M3_PACKAGING_ROOT}" ABSOLUTE)

set(CPACK_PACKAGE_NAME "LibM3C")
set(CPACK_PACKAGE_VENDOR "LibM3C")
set(CPACK_PACKAGE_CONTACT "support@libm3c.local")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_FILE_NAME "LibM3C-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "LibM3C")

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
elseif(WIN32)
    set(CPACK_GENERATOR "WIX")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)

set(M3_IOS_PROJECT "" CACHE PATH "Path to Xcode .xcodeproj for IPA packaging")
set(M3_IOS_WORKSPACE "" CACHE PATH "Path to Xcode .xcworkspace for IPA packaging")
set(M3_IOS_SCHEME "" CACHE STRING "Xcode scheme for IPA packaging")
set(M3_IOS_EXPORT_OPTIONS "${M3_PACKAGING_ROOT}/ios/exportOptions.plist" CACHE PATH "Export options plist for IPA packaging")

add_custom_target(m3_package_android
    COMMAND ${CMAKE_COMMAND} -DM3_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${M3_PACKAGING_ROOT}/android/package.cmake
    COMMENT "Packaging Android APK/AAB"
)

add_custom_target(m3_package_ios_xcframework
    COMMAND ${CMAKE_COMMAND} -DM3_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${M3_PACKAGING_ROOT}/ios/build_xcframework.cmake
    COMMENT "Packaging iOS XCFramework"
)

add_custom_target(m3_package_ios_ipa
    COMMAND ${CMAKE_COMMAND}
        -DM3_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -DIOS_PROJECT=${M3_IOS_PROJECT}
        -DIOS_WORKSPACE=${M3_IOS_WORKSPACE}
        -DIOS_SCHEME=${M3_IOS_SCHEME}
        -DIOS_EXPORT_OPTIONS_PLIST=${M3_IOS_EXPORT_OPTIONS}
        -P ${M3_PACKAGING_ROOT}/ios/archive_app.cmake
    COMMENT "Packaging iOS IPA"
)

add_custom_target(m3_package_web
    COMMAND ${CMAKE_COMMAND} -DM3_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${M3_PACKAGING_ROOT}/web/build_web.cmake
    COMMENT "Packaging Web PWA"
)

add_custom_target(m3_package_desktop
    COMMAND ${CMAKE_COMMAND} -DM3_BINARY_DIR=${CMAKE_BINARY_DIR}
        -DM3_CPACK_GENERATOR=${CPACK_GENERATOR}
        -P ${M3_PACKAGING_ROOT}/desktop/package.cmake
    COMMENT "Packaging desktop installers"
)

add_custom_target(m3_package_sdl3
    COMMAND ${CMAKE_COMMAND} -DM3_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${M3_PACKAGING_ROOT}/sdl3/build_sdl3.cmake
    COMMENT "Packaging SDL3 builds"
)
