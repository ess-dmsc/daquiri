set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
#set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_SHARED_LIBS ON)

# deprecate?
add_definitions(-DBOOST_LOG_DYN_LINK)

#find_package(Boost 1.41 COMPONENTS
find_package(Boost COMPONENTS
  system thread timer date_time log log_setup
  REQUIRED)

