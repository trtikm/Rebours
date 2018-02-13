#include <rebours/MAL/prologue/builder.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/msgstream.hpp>
#include <rebours/utility/development.hpp>
#include <algorithm>

namespace mal { namespace prologue { namespace detail { namespace x86_64_Linux { namespace {


uint64_t  args_start(descriptor::storage const&  D)
{
    return D.start_address_of_temporaries() + 64ULL;
}


void  build_start_component(descriptor::storage const&  D, microcode::program_component&  C, microcode::annotations&  A,  microcode::program const&  P)
{
    microcode::program_component::node_id  node = C.entry();

    std::string const  root_file = D.file_descriptor()->dependencies_of_loaded_files()->root_file();
    std::string  loader;
    {
        loader::file_property_map const&  fprops = *D.file_descriptor()->files_table()->at(root_file)->property_map();
        auto const  it = fprops.find(loader::file_properties::abi_loader());
        INVARIANT(it != fprops.cend());
        loader = it->second;
    }

    microcode::append({
        { node,{
            { "PROGRAM.NAME", P.name() },
            { "COMPONENT.NAME", C.name() },
            { "CPU.ARCHITECTURE", (msgstream() << D.processor() << msgstream::end()) },
            { "OS.TYPE", (msgstream() << D.system() << msgstream::end()) },
            { "OS.LOADER", loader },
            { "ASM.NOTATION", "INTEL" },
            }}
        },A);

    for (auto adr_sec : *D.file_descriptor()->sections_table())
    {
        loader::section_ptr const  section = adr_sec.second;

        microcode::instruction const  instruction = microcode::create_MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC(
                    D.start_address_of_temporaries(),
                    section->start_address(),
                    section->has_read_access(),
                    section->has_write_access(),
                    section->has_execute_access(),
                    section->is_in_big_endian(),
                    !section->has_const_endian(),
                    section->end_address()-section->start_address()
                    );

        node = C.insert_sequence(node,{instruction});
    }

    for (mal::descriptor::stack_section const&  ssec : D.stack_sections())
    {
        microcode::instruction const  instruction = microcode::create_MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC(
                    D.start_address_of_temporaries(),
                    ssec.start_address(),
                    ssec.has_read_access(),
                    ssec.has_write_access(),
                    ssec.has_execute_access(),
                    ssec.is_in_big_endian(),
                    !ssec.has_const_endian(),
                    ssec.end_address() - ssec.start_address()
                    );

        node = C.insert_sequence(node,{instruction});
    }

    node = C.insert_sequence(node,{
                microcode::create_MEMORYMANAGEMENT__REG_ASGN_MEM_STATIC(
                        D.start_address_of_temporaries(),
                        D.heap_start(),
                        true,
                        true,
                        false,
                        false,
                        false,
                        D.tls_template_content().size()
                        ),
                });

    node = C.insert_sequence(node,{
                microcode::create_INPUTOUTPUT__REG_ASGN_STREAM_OPEN_NUMBER(
                        D.start_address_of_temporaries(),1U,1ULL
                        ),
                microcode::create_INPUTOUTPUT__REG_ASGN_STREAM_OPEN_NUMBER(
                        D.start_address_of_temporaries(),2U,2ULL
                        ),
                microcode::create_INPUTOUTPUT__REG_ASGN_STREAM_OPEN_NUMBER(
                        D.start_address_of_temporaries(),2U,3ULL
                        ),
                microcode::create_INPUTOUTPUT__REG_ASGN_STREAM_OPEN_NUMBER(
                        D.start_address_of_temporaries(),2U,4ULL
                        ),
                 microcode::create_MODULARITY__CALL(
                        P.component(1U).entry()
                        ),
                 microcode::create_MODULARITY__CALL(
                        P.component(11U).entry()
                        ),
                });

    if (D.has_tls_template())
    {
        node = C.insert_sequence(node,{
                    microcode::create_MODULARITY__CALL(
                            P.component(12U).entry()
                            )
                    });
    }

    node = C.insert_sequence(node,{
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        8U,
                        D.registers_to_ranges().at("rdx").first,
                        0ULL
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        8U,
                        D.registers_to_ranges().at("fs").first,
                        D.heap_start() + D.tls_template_offset()
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        8U,
                        D.registers_to_ranges().at("rip").first,
                        D.file_descriptor()->entry_point()
                        ),
                });
}

void  build_initial_code_and_data_sections(descriptor::storage const&  D, microcode::program_component&  C)
{
    microcode::program_component::node_id  node = C.entry();
    for (auto adr_sec : *D.file_descriptor()->sections_table())
    {
        loader::section_ptr const  section = adr_sec.second;

        microcode::instruction const  instruction = microcode::create_DATATRANSFER__DEREF_ADDRESS_ASGN_DATA(
                    section->start_address(),
                    *section->content()
                    );

        node = C.insert_sequence(node,{instruction});
    }
}

void  build_read_count(descriptor::storage const&  D, microcode::program_component&  C)
{
    C.insert_sequence(C.entry(),{
                microcode::create_INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER(
                        D.start_address_of_temporaries() + 8ULL,
                        1ULL
                        ),
                microcode::create_INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER(
                        D.start_address_of_temporaries() + 9ULL,
                        1ULL
                        ),
                microcode::create_INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER(
                        D.start_address_of_temporaries() + 10ULL,
                        1ULL
                        ),
                microcode::create_INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER(
                        D.start_address_of_temporaries() + 11ULL,
                        1ULL
                        ),
                microcode::create_INDIRECTCOPY__REG_REG_ASGN_REG(
                        4U,
                        8U,
                        D.start_address_of_temporaries() + 0ULL,
                        D.start_address_of_temporaries() + 8ULL
                        ),
                });
}

void  build_read_strings(descriptor::storage const&  D, microcode::program_component&  C, microcode::annotations&  A)
{
    microcode::program_component::node_id const  read_next_string =
        C.insert_sequence(C.entry(),{
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        4U,
                        D.start_address_of_temporaries() + 8ULL,
                        0ULL
                        ),
                });

    microcode::program_component::node_id  n =
        C.insert_sequence(read_next_string,{
                microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                        4U,
                        D.start_address_of_temporaries() + 16ULL,
                        D.start_address_of_temporaries() + 8ULL,
                        D.start_address_of_temporaries() + 12ULL
                        ),
            });

    microcode::program_component::node_id const  read_string =
            C.insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,4U,D.start_address_of_temporaries() + 16ULL,n).first;


    n = C.insert_sequence(read_string,{
                microcode::create_INPUTOUTPUT__REG_ASGN_STREAM_READ_NUMBER(
                        D.start_address_of_temporaries() + 16ULL,
                        1ULL
                        ),
                microcode::create_INDIRECTCOPY__REG_REG_ASGN_REG(
                        1U,
                        8U,
                        D.start_address_of_temporaries() + 0ULL,
                        D.start_address_of_temporaries() + 16ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8ULL,
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries(),
                        1ULL
                        ),
                });

    microcode::program_component::node_id const  string_end =
            C.insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,1U,D.start_address_of_temporaries() + 16ULL,n,{read_string,0ULL}).second;

    C.insert_sequence(string_end,{
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        4ULL,
                        D.start_address_of_temporaries() + 8ULL,
                        D.start_address_of_temporaries() + 8ULL,
                        1ULL
                        ),
                },read_next_string);

    microcode::append({
        {read_next_string,{{"LABELNAME","PROLOGUE_read_next_string"}}},
        {read_string,{{"LABELNAME","PROLOGUE_read_string"}}},
        {string_end,{{"LABELNAME","PROLOGUE_string_end"}}},
        },A);
}

void  build_read_input_data(descriptor::storage const&  D, microcode::program_component&  C,  microcode::program const&  P)
{
    C.insert_sequence(C.entry(),{
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        8U,
                        D.start_address_of_temporaries() + 0ULL,
                        args_start(D)
                        ),
                microcode::create_MODULARITY__CALL(
                        P.component(2ULL).entry()
                        ),
                microcode::create_INDIRECTCOPY__REG_ASGN_REG_REG(
                        4U,
                        8U,
                        D.start_address_of_temporaries() + 12ULL,
                        D.start_address_of_temporaries() + 0ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8ULL,
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries(),
                        4ULL
                        ),
                microcode::create_MODULARITY__CALL(
                        P.component(3ULL).entry()
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        8U,
                        args_start(D) - 8ULL,
                        D.start_address_of_temporaries()
                        ),
                microcode::create_MODULARITY__CALL(
                        P.component(2ULL).entry()
                        ),
                microcode::create_INDIRECTCOPY__REG_ASGN_REG_REG(
                        4U,
                        8U,
                        D.start_address_of_temporaries() + 12ULL,
                        D.start_address_of_temporaries() + 0ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                        4ULL,
                        D.start_address_of_temporaries() + 12ULL,
                        D.start_address_of_temporaries() + 12ULL,
                        2ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8ULL,
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries(),
                        4ULL
                        ),
                microcode::create_MODULARITY__CALL(
                        P.component(3ULL).entry()
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        8U,
                        args_start(D) - 16ULL,
                        D.start_address_of_temporaries()
                        ),
                });

}

void  build_compute_stack_pointers(descriptor::storage const&  D, microcode::program_component&  C)
{
        C.insert_sequence(C.entry(),{
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        8U,
                        D.start_address_of_temporaries(),
                        args_start(D) - 16ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8ULL,
                        args_start(D) - 24ULL,
                        D.start_address_of_temporaries(),
                        D.stack_sections().back().end_address() + args_start(D) + 5ULL
                        ),
                microcode::create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(
                        4U,
                        D.start_address_of_temporaries(),
                        args_start(D)
                        ),
                microcode::create_INDIRECTCOPY__REG_ASGN_REG_REG(
                        4U,
                        8U,
                        D.start_address_of_temporaries() + 8ULL,
                        args_start(D) - 8ULL
                        ),
                microcode::create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(
                        4U,
                        D.start_address_of_temporaries() + 8ULL,
                        D.start_address_of_temporaries() + 8ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        8ULL,
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries() + 8ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_TIMES_NUMBER(
                        8ULL,
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries(),
                        8ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8ULL,
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries(),
                        32ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_NOT_REG(
                        8U,
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries()
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_REG(
                        8ULL,
                        D.start_address_of_temporaries(),
                        D.start_address_of_temporaries(),
                        args_start(D) - 24ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8ULL,
                        D.registers_to_ranges().at("rsp").first,
                        D.start_address_of_temporaries(),
                        1ULL
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_AND_NUMBER(
                        1ULL,
                        D.registers_to_ranges().at("rsp").second - 1ULL,
                        D.registers_to_ranges().at("rsp").second - 1ULL,
                        0xf0ULL
                        ),
                });
}

void  build_copy_string_to_stack(descriptor::storage const&  D, microcode::program_component&  C)
{
    microcode::program_component::node_id const  n =
        C.insert_sequence(C.entry(),{
                microcode::create_INDIRECTCOPY__REG_ASGN_REG_REG(
                        1U,
                        8U,
                        D.start_address_of_temporaries() + 32ULL,
                        D.start_address_of_temporaries() + 16ULL
                        ),
                microcode::create_DATATRANSFER__DEREF_REG_ASGN_REG(
                        1U,
                        D.start_address_of_temporaries() + 8ULL,
                        D.start_address_of_temporaries() + 32ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8ULL,
                        D.start_address_of_temporaries() + 8ULL,
                        D.start_address_of_temporaries() + 8ULL,
                        1ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8ULL,
                        D.start_address_of_temporaries() + 16ULL,
                        D.start_address_of_temporaries() + 16ULL,
                        1ULL
                        ),
                });

    C.insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,1U,D.start_address_of_temporaries() + 32ULL,n,{C.entry(),0ULL});
}

void  build_copy_params_to_stack(descriptor::storage const&  D, microcode::program_component&  C, microcode::annotations&  A, microcode::program const&  P)
{
    microcode::program_component::node_id const  copy_next_param =
        C.insert_sequence(C.entry(),{
                microcode::create_TYPECASTING__REG_ASGN_ZERO_EXTEND_REG(
                        4U,
                        D.start_address_of_temporaries() + 24ULL,
                        args_start(D)
                        ),
                microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                        8U,
                        D.registers_to_ranges().at("rsp").first,
                        D.start_address_of_temporaries() + 24ULL
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        4U,
                        D.start_address_of_temporaries() + 24ULL,
                        0ULL
                        ),
                });

    microcode::program_component::node_id  n =
        C.insert_sequence(copy_next_param,{
                microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                        4U,
                        D.start_address_of_temporaries() + 28ULL,
                        D.start_address_of_temporaries() + 24ULL,
                        args_start(D)
                        ),
                });

    microcode::program_component::node_id const  copy_param =
            C.insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,4U,D.start_address_of_temporaries() + 28ULL,n).first;

    C.insert_sequence(copy_param,{
            microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                    8U,
                    D.start_address_of_temporaries() + 0ULL,
                    D.start_address_of_temporaries() + 8ULL
                    ),
            microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                    8ULL,
                    D.start_address_of_temporaries() + 0ULL,
                    D.start_address_of_temporaries() + 0ULL,
                    8ULL
                    ),
            microcode::create_MODULARITY__CALL(
                    P.component(6ULL).entry()
                    ),
            microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                    4ULL,
                    D.start_address_of_temporaries() + 24ULL,
                    D.start_address_of_temporaries() + 24ULL,
                    1ULL
                    ),
            },copy_next_param);

    microcode::append({
        {copy_next_param,{{"LABELNAME","PROLOGUE_copy_next_param"}}},
        {copy_param,{{"LABELNAME","PROLOGUE_copy_param"}}},
        },A);
}

void  build_copy_env_to_stack(descriptor::storage const&  D, microcode::program_component&  C, microcode::annotations&  A, microcode::program const&  P)
{
    microcode::program_component::node_id const  copy_next_var =
        C.insert_sequence(C.entry(),{
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        4U,
                        D.start_address_of_temporaries() + 24ULL,
                        0ULL
                        ),
                microcode::create_INDIRECTCOPY__REG_ASGN_REG_REG(
                        4U,
                        8U,
                        D.start_address_of_temporaries() + 28ULL,
                        D.start_address_of_temporaries() + 16ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8ULL,
                        D.start_address_of_temporaries() + 16ULL,
                        D.start_address_of_temporaries() + 16ULL,
                        4ULL
                        ),
                });

    microcode::program_component::node_id  n =
        C.insert_sequence(copy_next_var,{
                microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                        4U,
                        D.start_address_of_temporaries() + 32ULL,
                        D.start_address_of_temporaries() + 24ULL,
                        D.start_address_of_temporaries() + 28ULL
                        ),
                });

    microcode::program_component::node_id const  copy_var =
            C.insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,4U,D.start_address_of_temporaries() + 32ULL,n).first;

    C.insert_sequence(copy_var,{
            microcode::create_DATATRANSFER__DEREF_INV_REG_ASGN_REG(
                    8U,
                    D.start_address_of_temporaries() + 0ULL,
                    D.start_address_of_temporaries() + 8ULL
                    ),
            microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                    8ULL,
                    D.start_address_of_temporaries() + 0ULL,
                    D.start_address_of_temporaries() + 0ULL,
                    8ULL
                    ),
            microcode::create_MODULARITY__CALL(
                    P.component(6ULL).entry()
                    ),
            microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                    8ULL,
                    D.start_address_of_temporaries() + 32ULL,
                    D.start_address_of_temporaries() + 8ULL,
                    0xffffffffffffffffULL
                    ),
            microcode::create_DATATRANSFER__DEREF_REG_ASGN_NUMBER(
                    1U,
                    D.start_address_of_temporaries() + 32ULL,
                    0x3dULL
                    ),
//            microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
//                    8ULL,
//                    D.start_address_of_temporaries() + 8ULL,
//                    D.start_address_of_temporaries() + 8ULL,
//                    1ULL
//                    ),
            microcode::create_MODULARITY__CALL(
                    P.component(6ULL).entry()
                    ),
            microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                    4ULL,
                    D.start_address_of_temporaries() + 24ULL,
                    D.start_address_of_temporaries() + 24ULL,
                    1ULL
                  ),
            },copy_next_var);

    microcode::append({
        {copy_next_var,{{"LABELNAME","PROLOGUE_copy_next_var"}}},
        {copy_var,{{"LABELNAME","PROLOGUE_copy_var"}}},
        },A);
}

void  build_set_alignment_zeros(descriptor::storage const&  D, microcode::program_component&  C, microcode::annotations&  A)
{
    microcode::program_component::node_id const  n =
        C.insert_sequence(C.entry(),{
                microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                        8U,
                        D.start_address_of_temporaries() + 16ULL,
                        D.start_address_of_temporaries() + 0ULL,
                        args_start(D) - 24ULL
                        ),
                });

    microcode::program_component::node_id const  clear_byte =
            C.insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,8U,D.start_address_of_temporaries() + 16ULL,n).first;

    C.insert_sequence(clear_byte,{
            microcode::create_DATATRANSFER__DEREF_REG_ASGN_NUMBER(
                    1U,
                    D.start_address_of_temporaries() + 0ULL,
                    0ULL
                    ),
            microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                    8ULL,
                    D.start_address_of_temporaries() + 0ULL,
                    D.start_address_of_temporaries() + 0ULL,
                    1ULL
                    ),
            },C.entry());

    microcode::append({{clear_byte,{{"LABELNAME","PROLOGUE_clear_byte"}}}},A);
}

void  build_havoc_used_temporaries(descriptor::storage const&  D, microcode::program_component&  C, microcode::annotations&  A)
{
    microcode::program_component::node_id const  havoc_input =
        C.insert_sequence(C.entry(),{
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        8U,
                        D.start_address_of_temporaries() + 0ULL,
                        args_start(D)
                        ),
                });

    microcode::program_component::node_id const  n =
        C.insert_sequence(havoc_input,{
                microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                        8U,
                        D.start_address_of_temporaries() + 8ULL,
                        D.start_address_of_temporaries() + 0ULL,
                        args_start(D) - 16ULL
                        ),
                });

    auto const  havoc_byte_remainig =
            C.insert_branching(microcode::GIK::GUARDS__REG_NOT_EQUAL_TO_ZERO,8U,D.start_address_of_temporaries() + 8ULL,n);

    C.insert_sequence(havoc_byte_remainig.first,{
            microcode::create_HAVOC__REG_REG_ASGN_HAVOC(
                    8U,
                    1U,
                    D.start_address_of_temporaries() + 0ULL
                    ),
            microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                    8ULL,
                    D.start_address_of_temporaries() + 0ULL,
                    D.start_address_of_temporaries() + 0ULL,
                    1ULL
                    ),
            },havoc_input);

    C.insert_sequence(havoc_byte_remainig.second,{
            microcode::create_HAVOC__REG_ASGN_HAVOC(
                    args_start(D) - D.start_address_of_temporaries(),
                    D.start_address_of_temporaries() + 0ULL
                    ),
            });

    microcode::append({
        {havoc_input,{{"LABELNAME","PROLOGUE_havoc_input"}}},
        {havoc_byte_remainig.first,{{"LABELNAME","PROLOGUE_havoc_byte"}}},
        {havoc_byte_remainig.second,{{"LABELNAME","PROLOGUE_havoc_remainig"}}},
        },A);
}


void  build_init_stack(descriptor::storage const&  D, microcode::program_component&  C, microcode::program const&  P)
{
    C.insert_sequence(C.entry(),{
                microcode::create_MODULARITY__CALL(
                        P.component(4ULL).entry()
                        ),
                microcode::create_MODULARITY__CALL(
                        P.component(5ULL).entry()
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8ULL,
                        D.start_address_of_temporaries() + 0ULL,
                        D.registers_to_ranges().at("rsp").first,
                        8ULL
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_REG(
                        8U,
                        D.start_address_of_temporaries() + 8ULL,
                        args_start(D) - 24ULL
                        ),
                microcode::create_SETANDCOPY__REG_ASGN_NUMBER(
                        8U,
                        D.start_address_of_temporaries() + 16ULL,
                        args_start(D) + 4ULL
                        ),
                microcode::create_MODULARITY__CALL(
                        P.component(7ULL).entry()
                        ),
                microcode::create_DATATRANSFER__DEREF_REG_ASGN_NUMBER(
                        8U,
                        D.start_address_of_temporaries() + 0ULL,
                        0ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8ULL,
                        D.start_address_of_temporaries() + 0ULL,
                        D.start_address_of_temporaries() + 0ULL,
                        8ULL
                        ),
                microcode::create_MODULARITY__CALL(
                        P.component(8ULL).entry()
                        ),
                microcode::create_BITOPERATIONS__REG_ASGN_REG_XOR_REG(
                        8U,
                        D.start_address_of_temporaries() + 16ULL,
                        D.start_address_of_temporaries() + 8ULL,
                        args_start(D) - 16ULL
                        ),


                microcode::create_DATATRANSFER__DEREF_REG_ASGN_NUMBER(
                        8U,
                        D.start_address_of_temporaries() + 0ULL,
                        0ULL
                        ),
                microcode::create_INTEGERARITHMETICS__REG_ASGN_REG_PLUS_NUMBER(
                        8ULL,
                        D.start_address_of_temporaries() + 0ULL,
                        D.start_address_of_temporaries() + 0ULL,
                        8ULL
                        ),
                microcode::create_MODULARITY__CALL(
                        P.component(9ULL).entry()
                        ),
                microcode::create_MODULARITY__CALL(
                        P.component(10ULL).entry()
                        ),
                });
}


void build_init_tls_section(descriptor::storage const&  D, microcode::program_component&  C)
{
    std::vector<uint8_t>  content(D.tls_template_content());
    for (auto offset : D.tls_template_relocations())
    {
        INVARIANT(offset + 8ULL <= content.size());

        uint64_t  value;
        uint8_t* const  b = reinterpret_cast<uint8_t*>(&value);
        uint8_t* const  e = b + sizeof(value);

        std::copy(content.data() + offset, content.data() + offset + sizeof(value),b);
        if (!is_this_little_endian_machine())
            std::reverse(b,e);

        value += D.heap_start();
        if (!is_this_little_endian_machine())
            std::reverse(b,e);

        std::copy(b,e,content.data() + offset);
    }

    C.insert_sequence(C.entry(),{
                microcode::create_DATATRANSFER__DEREF_ADDRESS_ASGN_DATA(
                        D.heap_start(),
                        content
                        )
                });
}


}}}}}

namespace mal { namespace prologue { namespace detail { namespace x86_64_Linux {


std::pair<std::unique_ptr<microcode::program>,std::unique_ptr<microcode::annotations> >  build(descriptor::storage const&  description)
{
    std::vector<std::string> component_names = {
        "init_code_and_data_sections",
        "read_count",
        "read_strings",
        "read_input_data",
        "compute_stack_pointers",
        "copy_string_to_stack",
        "copy_params_to_stack",
        "copy_env_to_stack",
        "set_alignment_zeros",
        "havoc_used_temporaries",
        "init_stack",
    };

    std::unique_ptr<microcode::program>  program( new microcode::program("Prologue","MAIN") );
    std::unique_ptr<microcode::annotations>  annotations{ new microcode::annotations };

    for (std::string const&  name : component_names)
    {
        program->push_back(std::make_shared<microcode::program_component>(program->name(),name));
        uint64_t const i = program->num_components() - 1ULL;
        microcode::append({
            { program->component(i).entry(), {
                  {"COMPONENT.NAME", program->component(i).name()},
                  {"LABELNAME", (msgstream() << "PROLOGUE_" << name << msgstream::end()) }
                  } }
            },*annotations);
    }

    INVARIANT(program->num_components() == component_names.size() + 1ULL);

    if (description.has_tls_template())
    {
        std::string const  name = "init_tls_section";
        program->push_back(std::make_shared<microcode::program_component>(program->name(),name));
        uint64_t const i = program->num_components() - 1ULL;
        microcode::append({
            { program->component(i).entry(), {
                  {"COMPONENT.NAME", program->component(i).name()},
                  {"LABELNAME", (msgstream() << "PROLOGUE_" << name << msgstream::end()) }
                  } }
            },*annotations);
    }

    build_start_component(description,program->component(0ULL),*annotations,*program);
    build_initial_code_and_data_sections(description,program->component(1ULL));
    build_read_count(description,program->component(2ULL));
    build_read_strings(description,program->component(3ULL),*annotations);
    build_read_input_data(description,program->component(4ULL),*program);
    build_compute_stack_pointers(description,program->component(5ULL));
    build_copy_string_to_stack(description,program->component(6ULL));
    build_copy_params_to_stack(description,program->component(7ULL),*annotations,*program);
    build_copy_env_to_stack(description,program->component(8ULL),*annotations,*program);
    build_set_alignment_zeros(description,program->component(9ULL),*annotations);
    build_havoc_used_temporaries(description,program->component(10ULL),*annotations);
    build_init_stack(description,program->component(11ULL),*program);
    build_init_tls_section(description,program->component(12ULL));

    return {std::move(program),std::move(annotations)};
}


}}}}
