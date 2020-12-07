set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests/build)
set(CTEST_BINARY_DIRECTORY ${CMAKE_BINARY_DIR}/tests/build)

file(GLOB files "*.c")

find_program (BASH_PROGRAM bash)

foreach(file ${files})
	string(REGEX REPLACE "(^.*/|\\.[^.]*$)" "" file_without_ext ${file})
	add_executable(${file_without_ext} ${file})
	add_test(
        NAME ${file_without_ext}
        COMMAND sh -c "${BASH_PROGRAM} test.sh $<TARGET_FILE:${file_without_ext}>"
    )
    add_dependencies(${file_without_ext} cc)
	set_tests_properties(${file_without_ext}
		PROPERTIES
		TIMEOUT 120)
endforeach()