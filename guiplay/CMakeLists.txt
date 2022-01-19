set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Widgets Concurrent REQUIRED)

add_executable(guishashki
	main.cpp
	main_window.cpp
	board_controller.cpp
	board_widget.cpp
	game.cpp
	root.tpp
	icons.qrc
)
target_link_libraries(guishashki board)
target_link_libraries(guishashki Qt5::Widgets)
