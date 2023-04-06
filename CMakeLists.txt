cmake_minimum_required(VERSION 3.14)
project(MyOperatingSystem CXX ASM)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} --32")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -melf_i386 -T ${CMAKE_SOURCE_DIR}/linker.ld")

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.s")

foreach(source ${SOURCES})
    file(RELATIVE_PATH source_rel "${CMAKE_SOURCE_DIR}/src" "${source}")
    string(REGEX REPLACE "[.](cpp|s)$" ".o" object_rel "${source_rel}")
    string(REGEX REPLACE "/" "_" target_name "${object_rel}")
    set(object "${CMAKE_BINARY_DIR}/${object_rel}")
    if("${source}" MATCHES "[.]cpp$")
        add_custom_command(
            OUTPUT "${object}"
            COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS} -c -o "${object}" "${source}"
            DEPENDS "${source}"
            COMMENT "Compiling ${source_rel}"
            VERBATIM
        )
    else()
        add_custom_command(
            OUTPUT "${object}"
            COMMAND ${CMAKE_ASM_COMPILER} ${CMAKE_ASM_FLAGS} -o "${object}" "${source}"
            DEPENDS "${source}"
            COMMENT "Assembling ${source_rel}"
            VERBATIM
        )
    endif()
    add_custom_target("${target_name}" DEPENDS "${object}")
    list(APPEND OBJECTS "${object}")
endforeach()

add_executable(mykernel.bin ${SOURCES})
add_dependencies(mykernel.bin ${OBJECTS})

add_custom_command(
    OUTPUT mykernel.iso
    COMMAND ${CMAKE_COMMAND} -E make_directory iso
    COMMAND ${CMAKE_COMMAND} -E make_directory iso/boot
    COMMAND ${CMAKE_COMMAND} -E make_directory iso/boot/grub
    COMMAND ${CMAKE_COMMAND} -E copy mykernel.bin iso/boot/mykernel.bin
    COMMAND ${CMAKE_COMMAND} -E echo "set timeout=0"                      > iso/boot/grub/grub.cfg
    COMMAND ${CMAKE_COMMAND} -E echo "set default=0"                     >> iso/boot/grub/grub.cfg
    COMMAND ${CMAKE_COMMAND} -E echo ""                                  >> iso/boot/grub/grub.cfg
    COMMAND ${CMAKE_COMMAND} -E echo "menuentry \"My Operating System\" {" >> iso/boot/grub/grub.cfg
    COMMAND ${CMAKE_COMMAND} -E echo "  multiboot /boot/mykernel.bin"    >> iso/boot/grub/grub.cfg
    COMMAND ${CMAKE_COMMAND} -E echo "  boot"                            >> iso/boot/grub/grub.cfg
    COMMAND ${CMAKE_COMMAND} -E echo "}"                                 >> iso/boot/grub/grub.cfg
    COMMAND grub-mkrescue --output=mykernel.iso iso
    COMMAND ${CMAKE_COMMAND} -E remove_directory iso
    DEPENDS mykernel.bin
)

add_custom_target(run
    COMMAND killall VirtualBoxVM && sleep 1 || true
    COMMAND VirtualBoxVM --startvm 'My Operating System' &
    DEPENDS mykernel.iso
)

add_custom_target(install
       COMMAND sudo cp ${CMAKE_BINARY_DIR}/mykernel.bin /boot/mykernel.bin
    DEPENDS mykernel.bin
)

add_custom_target(clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/obj"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/mykernel.bin"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/mykernel.iso"
)
