set(INKVM_TD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/InkVM.td" "${CMAKE_CURRENT_SOURCE_DIR}/InkVMRegisterInfo.td" "${CMAKE_CURRENT_SOURCE_DIR}/InkVMCallingConv.td" "${CMAKE_CURRENT_SOURCE_DIR}/InkVMInstrInfo.td")

function(inkvm_tablegen OutputFile Flag)
    set(OutputPath "${CMAKE_CURRENT_BINARY_DIR}/${OutputFile}")
    add_custom_command(OUTPUT "${OutputPath}" COMMAND "$<TARGET_FILE:llvm-tblgen>" -I "${CMAKE_CURRENT_SOURCE_DIR}" -I "${LLVM_MAIN_INCLUDE_DIR}" ${Flag} "${CMAKE_CURRENT_SOURCE_DIR}/InkVM.td" -o "${OutputPath}" DEPENDS llvm-tblgen ${INKVM_TD_FILES} COMMENT "Building InkVM TableGen file ${OutputFile}" VERBATIM)
endfunction()

inkvm_tablegen(InkVMGenRegisterInfo.inc -gen-register-info)
inkvm_tablegen(InkVMGenInstrInfo.inc -gen-instr-info)
inkvm_tablegen(InkVMGenSubtargetInfo.inc -gen-subtarget)
inkvm_tablegen(InkVMGenDAGISel.inc -gen-dag-isel)
inkvm_tablegen(InkVMGenAsmWriter.inc -gen-asm-writer)

set(INKVM_TABLEGEN_OUTPUTS "${CMAKE_CURRENT_BINARY_DIR}/InkVMGenRegisterInfo.inc" "${CMAKE_CURRENT_BINARY_DIR}/InkVMGenInstrInfo.inc" "${CMAKE_CURRENT_BINARY_DIR}/InkVMGenSubtargetInfo.inc" "${CMAKE_CURRENT_BINARY_DIR}/InkVMGenDAGISel.inc" "${CMAKE_CURRENT_BINARY_DIR}/InkVMGenAsmWriter.inc")
add_custom_target(ink_llvm_inkvm_tablegen DEPENDS ${INKVM_TABLEGEN_OUTPUTS})

set(INKVM_SOURCES TargetInfo/InkVMTargetInfo.cpp MCTargetDesc/InkVMMCAsmInfo.cpp MCTargetDesc/InkVMInstPrinter.cpp MCTargetDesc/InkVMMCTargetDesc.cpp InkVMRegisterInfo.cpp InkVMInstrInfo.cpp InkVMFrameLowering.cpp InkVMISelLowering.cpp InkVMISelDAGToDAG.cpp InkVMSubtarget.cpp InkVMAsmPrinter.cpp InkVMTargetMachine.cpp InkVMTarget.cpp)

add_library(ink_llvm_inkvm STATIC ${INKVM_SOURCES} ${INKVM_TABLEGEN_OUTPUTS})
add_library(ink::llvm_inkvm ALIAS ink_llvm_inkvm)
add_dependencies(ink_llvm_inkvm ink_llvm_inkvm_tablegen)

target_compile_features(ink_llvm_inkvm PUBLIC cxx_std_23)
target_include_directories(ink_llvm_inkvm PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(ink_llvm_inkvm SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_options(ink_llvm_inkvm PUBLIC ${LLVM_DEFINITIONS})
target_link_libraries(ink_llvm_inkvm PUBLIC LLVMCodeGen LLVMCore LLVMMC LLVMSupport LLVMTarget LLVMTargetParser PRIVATE LLVMAnalysis LLVMAsmPrinter LLVMCodeGenTypes LLVMSelectionDAG)
ink_disable_exceptions(ink_llvm_inkvm)

if(MSVC)
    target_compile_options(ink_llvm_inkvm PRIVATE /utf-8 /W4 /permissive- /bigobj)
else()
    target_compile_options(ink_llvm_inkvm PRIVATE -Wall -Wextra -Wpedantic)
endif()
