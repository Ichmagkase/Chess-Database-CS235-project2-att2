cmake_minimum_required(VERSION 3.10)

project(ChessParser)

add_executable(ChessParser)

target_sources(ChessParser
  PRIVATE
    main.c
)
