install(FILES
            app.png
        DESTINATION
            ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/resources/icons)

install(FILES
            app.png
        RENAME
            ${PROJECT_NAME}.png
        DESTINATION
            ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps)
