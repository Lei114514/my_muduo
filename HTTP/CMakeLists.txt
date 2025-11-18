set(HTTP_SOURCES 
    
    #每實現一個就新增一個
)

add_library(Net STATIC ${HTTP_SOURCES})

target_include_directories(Net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Net PUBLIC Base)