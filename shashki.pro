QT = core gui widgets concurrent
TEMPLATE = app
TARGET = shashki
CONFIG += c++11

SOURCES = \
action_builder.cpp \
action_tree.cpp \
ai_player.cpp \
board_state.cpp \
board_widget.cpp \
cell.cpp \
direction.cpp \
game.cpp \
gui_player.cpp \
history_widget.cpp \
main.cpp \
main_window.cpp \
position.cpp \
role.cpp \
segment.cpp

HEADERS = \
action_builder.h \
action_tree.h \
ai_player.h \
board_state.h \
board_widget.h \
cell.h \
direction.h \
game.h \
gui_player.h \
history_widget.h \
main_window.h \
player.h \
position.h \
role.h \
segment.h

RESOURCES += \
    icons.qrc
