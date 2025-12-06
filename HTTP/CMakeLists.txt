set(HTTP_SOURCES 
    HttpRequest.cc
    HttpServer.cc
    HttpResponse.cc
    #每實現一個就新增一個
)

add_library(HTTP STATIC ${HTTP_SOURCES})

target_include_directories(HTTP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(HTTP PUBLIC Base)