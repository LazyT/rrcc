qt5_add_translation(rrcc_QM_FILES
    rrcc_de.ts)
add_custom_target(translations ALL DEPENDS ${rrcc_QM_FILES})

install(FILES
            ${rrcc_QM_FILES}
        DESTINATION
            ${CMAKE_INSTALL_DATADIR}/rrcc/resources/lang)
