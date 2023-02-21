add_executable(test_json test_json.cc)

# add_executable(test_parser test_parser.cc)
# add_dependencies(test_parser letter)
# target_include_directories(test_parser PUBLIC ${PROJECT_SOURCE_DIR}/src/)
# target_link_libraries(test_parser PUBLIC letter)


function(ae tmp)
add_executable(${tmp} ${tmp}.cc)
add_dependencies(${tmp} letter)
target_include_directories(${tmp} PUBLIC ${PROJECT_SOURCE_DIR}/src/)
target_link_libraries(${tmp} PUBLIC letter)
endfunction()

ae(test_parser)
ae(mdtest_parser)

