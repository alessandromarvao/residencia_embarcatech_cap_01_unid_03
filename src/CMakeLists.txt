# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

add_executable(three_tasks
    main.c
)

# Corrige a saída para build/ em vez de build/src/
set_target_properties(three_tasks PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_sources(three_tasks
    PRIVATE
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

target_include_directories(three_tasks PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../config
    ${CMAKE_CURRENT_LIST_DIR}/../tasks
    ${CMAKE_CURRENT_LIST_DIR}/../drivers
)

target_link_libraries(three_tasks 
    pico_stdlib 
    FreeRTOS-Kernel
    hardware_irq
)

pico_add_extra_outputs(three_tasks)
