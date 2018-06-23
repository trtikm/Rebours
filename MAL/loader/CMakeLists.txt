set(THIS_TARGET_NAME loader)


add_library(${THIS_TARGET_NAME}
    ./include/rebours/MAL/loader/address.hpp

    ./include/rebours/MAL/loader/platform.hpp
    ./src/platform.cpp

    ./include/rebours/MAL/loader/sections_table.hpp
    ./src/sections_table.cpp

    ./include/rebours/MAL/loader/special_sections.hpp
    ./src/special_sections.cpp
    ./include/rebours/MAL/loader/special_sections/special_section_properties.hpp
    ./src/special_sections/special_section_properties.cpp
    ./include/rebours/MAL/loader/special_sections/exceptions.hpp
    ./src/special_sections/exceptions.cpp
    ./include/rebours/MAL/loader/special_sections/load_configuration_structure.hpp
    ./src/special_sections/load_configuration_structure.cpp
    ./include/rebours/MAL/loader/special_sections/thread_local_storage_initialisers.hpp
    ./src/special_sections/thread_local_storage_initialisers.cpp
    ./include/rebours/MAL/loader/special_sections/elf_tls.hpp
    ./src/special_sections/elf_tls.cpp

    ./include/rebours/MAL/loader/file_props.hpp
    ./src/file_props.cpp

    ./include/rebours/MAL/loader/descriptor.hpp
    ./src/descriptor.cpp

    ./include/rebours/MAL/loader/dependencies_graph.hpp
    ./src/dependencies_graph.cpp

    ./include/rebours/MAL/loader/init_fini.hpp

    ./include/rebours/MAL/loader/relocations.hpp
    ./src/relocations.cpp

    ./include/rebours/MAL/loader/warnings.hpp

    ./include/rebours/MAL/loader/dump.hpp
    ./src/dump.cpp

    ./include/rebours/MAL/loader/load.hpp
    ./src/load.cpp

    ./include/rebours/MAL/loader/detail/set_file_property.hpp
    ./include/rebours/MAL/loader/detail/std_pair_hash.hpp

    ./include/rebours/MAL/loader/detail/address_fixing.hpp
    ./src/detail/address_fixing.cpp

    ./include/rebours/MAL/loader/detail/mutable_sections_table.hpp
    ./src/detail/mutable_sections_table.cpp

    ./include/rebours/MAL/loader/detail/abi_loaders.hpp
    ./src/detail/abi_loaders/ld_linux_x86_64_so_2.cpp
    ./src/detail/abi_loaders/ld_linux_x86_32_so_2.cpp

    ./include/rebours/MAL/loader/detail/load_elf.hpp
    ./src/detail/load_elf.cpp

    ./include/rebours/MAL/loader/detail/load_props_elf.hpp
    ./src/detail/load_props_elf.cpp

    ./src/detail/load_elf_64bit.cpp
    ./src/detail/load_elf_32bit.cpp

    ./include/rebours/MAL/loader/detail/load_pe.hpp
    ./src/detail/load_pe.cpp

    ./include/rebours/MAL/loader/detail/load_props_pe.hpp
    ./src/detail/load_props_pe.cpp

    ./src/detail/load_pe_64bit.cpp
    ./src/detail/load_pe_32bit.cpp

    ./include/rebours/MAL/loader/detail/load_mach.hpp
    ./src/detail/load_mach.cpp

    ./include/rebours/MAL/loader/detail/load_props_mach.hpp
    ./src/detail/load_props_mach.cpp

    ./src/detail/load_mach_64bit.cpp
    ./src/detail/load_mach_32bit.cpp
    )


set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib/MAL")
