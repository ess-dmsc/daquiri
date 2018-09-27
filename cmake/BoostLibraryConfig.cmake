set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
#set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_SHARED_LIBS ON)

find_package(Boost 1.41 COMPONENTS system algorithm lexical_cast REQUIRED)

add_definitions(-DBOOST_ALL_DYN_LINK)
