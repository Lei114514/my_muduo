set(BASE_SOURCES 
    CurrentThread.cc
    Logger.cc    
    Timestamp.cc
    Thread.cc
    #每實現一個就新增一個
)

add_library(Base STATIC ${BASE_SOURCES})


target_include_directories(Base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

