add_executable(sf_transpile
    transpile_main.cpp
 )

target_compile_features(sf_transpile PUBLIC cxx_std_20)

target_link_libraries(
    sf_transpile
)

# Optional install step if you later want `make install`
install(TARGETS sf_transpile RUNTIME DESTINATION tools)
