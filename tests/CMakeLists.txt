macro(run_tests only_compile)

file(GLOB files "*.c")

foreach(file ${files})
	string(REGEX REPLACE "(^.*/|\\.[^.]*$)" "" file_without_ext ${file})
	add_executable(${file_without_ext} EXCLUDE_FROM_ALL ${file})
	add_test(
        NAME ${file_without_ext}
        COMMAND ../test.sh "$<TARGET_FILE:${file_without_ext}>" ${only_compile}
    )
	add_dependencies(build_tests ${file_without_ext})
    add_dependencies(${file_without_ext} cc)
	set_tests_properties(${file_without_ext}
		PROPERTIES
		TIMEOUT 120)
endforeach()

endmacro()

add_subdirectory(run)
add_subdirectory(compile)