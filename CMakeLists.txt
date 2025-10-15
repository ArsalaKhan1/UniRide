cmake_minimum_required(VERSION 3.10)
project(RideShare)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(RideShare
    main.cpp
    User.cpp
    Ride.cpp
    RideSystem.cpp
    Request.cpp
    RequestQueue.cpp
    OTPverification.cpp
    ChatFeature.cpp
)
