# LLVM include path and compiler settings.
if(WIN32)
	# Append LLVM paths for the configurations
	SET(CMAKE_CXX_FLAGS_RELEASE			"${CMAKE_CXX_FLAGS_RELEASE}			/I\"${llvmdir}/include\"")
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO	"${CMAKE_CXX_FLAGS_RELWITHDEBINFO}	/I\"${llvmdir}/include\"")
	SET(CMAKE_CXX_FLAGS_DEBUG			"${CMAKE_CXX_FLAGS_DEBUG}			/I\"${llvmdir}_debug/include\"")
	SET(CMAKE_CXX_FLAGS_SDKDEBUG		"${CMAKE_CXX_FLAGS_SDKDEBUG}		/I\"${llvmdir}_debug/include\"")
else()
	SET(CMAKE_CXX_FLAGS	"${CMAKE_CXX_FLAGS} -I${llvmdir}/include -I/usr/local/include")
endif()


# Set TARGET_LLVM_VERSION
if(INDIGO_LLVM_VERSION STREQUAL "8.0.0")
	add_definitions("-DTARGET_LLVM_VERSION=80")
elseif(INDIGO_LLVM_VERSION STREQUAL "6.0.0")
	add_definitions("-DTARGET_LLVM_VERSION=60")
elseif(INDIGO_LLVM_VERSION STREQUAL "3.6")
	add_definitions("-DTARGET_LLVM_VERSION=36")
elseif(INDIGO_LLVM_VERSION STREQUAL "3.4")
	add_definitions("-DTARGET_LLVM_VERSION=34")
else()
	MESSAGE("Unsupported LLVM version ${INDIGO_LLVM_VERSION}")
endif()


# LLVM linker settings.
if(WIN32)
	
	SET(LLVM_LINK_FLAGS_RELEASE			"/LIBPATH:\"${llvmdir}/lib\"")
	SET(LLVM_LINK_FLAGS_RELWITHDEBINFO	"/LIBPATH:\"${llvmdir}/lib\"")
	SET(LLVM_LINK_FLAGS_DEBUG			"/LIBPATH:\"${llvmdir}_debug/lib\"")
	SET(LLVM_LINK_FLAGS_SDKDEBUG		"/LIBPATH:\"${llvmdir}_debug/lib\"")
	
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE			"${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${LLVM_LINK_FLAGS_RELEASE}")
	SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO	"${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} ${LLVM_LINK_FLAGS_RELWITHDEBINFO}")
	SET(CMAKE_EXE_LINKER_FLAGS_DEBUG			"${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${LLVM_LINK_FLAGS_DEBUG}")
	SET(CMAKE_EXE_LINKER_FLAGS_SDKDEBUG			"${CMAKE_EXE_LINKER_FLAGS_SDKDEBUG} ${LLVM_LINK_FLAGS_SDKDEBUG}")
	
	SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE			"${CMAKE_MODULE_LINKER_FLAGS_RELEASE} ${LLVM_LINK_FLAGS_RELEASE}")
	SET(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO	"${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} ${LLVM_LINK_FLAGS_RELWITHDEBINFO}")
	SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG				"${CMAKE_MODULE_LINKER_FLAGS_DEBUG} ${LLVM_LINK_FLAGS_DEBUG}")
	SET(CMAKE_MODULE_LINKER_FLAGS_SDKDEBUG			"${CMAKE_MODULE_LINKER_FLAGS_SDKDEBUG} ${LLVM_LINK_FLAGS_SDKDEBUG}")
	
	if(INDIGO_LLVM_VERSION STREQUAL "6.0.0")
		SET(LLVM_LIBS
			LLVMAnalysis.lib
			LLVMAsmParser.lib
			LLVMAsmPrinter.lib
			LLVMBinaryFormat.lib
			LLVMBitReader.lib
			LLVMBitWriter.lib
			LLVMCodeGen.lib
			LLVMCore.lib
			LLVMCoroutines.lib
			LLVMCoverage.lib
			LLVMDebugInfoCodeView.lib
			LLVMDebugInfoDWARF.lib
			LLVMDebugInfoMSF.lib
			LLVMDebugInfoPDB.lib
			LLVMDemangle.lib
			LLVMDlltoolDriver.lib
			LLVMExecutionEngine.lib
			LLVMFuzzMutate.lib
			LLVMGlobalISel.lib
			LLVMInstCombine.lib
			LLVMInstrumentation.lib
			LLVMInterpreter.lib
			LLVMipo.lib
			LLVMIRReader.lib
			LLVMLibDriver.lib
			LLVMLineEditor.lib
			LLVMLinker.lib
			LLVMLTO.lib
			LLVMMC.lib
			LLVMMCDisassembler.lib
			LLVMMCJIT.lib
			LLVMMCParser.lib
			LLVMObjCARCOpts.lib
			LLVMObject.lib
			LLVMObjectYAML.lib
			LLVMOption.lib
			LLVMOrcJIT.lib
			LLVMPasses.lib
			LLVMProfileData.lib
			LLVMRuntimeDyld.lib
			LLVMScalarOpts.lib
			LLVMSelectionDAG.lib
			LLVMSupport.lib
			LLVMSymbolize.lib
			LLVMTableGen.lib
			LLVMTarget.lib
			LLVMTransformUtils.lib
			LLVMVectorize.lib
			LLVMWindowsManifest.lib
			LLVMX86AsmParser.lib
			LLVMX86AsmPrinter.lib
			LLVMX86CodeGen.lib
			LLVMX86Desc.lib
			LLVMX86Disassembler.lib
			LLVMX86Info.lib
			LLVMX86Utils.lib
			LLVMXRay.lib
		)
	elseif(LLVM_VERSION STREQUAL "3.6")
		SET(LLVM_LIBS
			LLVMAnalysis.lib LLVMAsmParser.lib LLVMAsmPrinter.lib LLVMBitReader.lib LLVMBitWriter.lib LLVMCodeGen.lib LLVMCore.lib LLVMExecutionEngine.lib LLVMInstCombine.lib LLVMInstrumentation.lib LLVMInterpreter.lib LLVMipa.lib LLVMipo.lib 			
			LLVMLinker.lib LLVMMC.lib LLVMMCDisassembler.lib LLVMMCJIT.lib LLVMMCParser.lib LLVMObject.lib LLVMRuntimeDyld.lib LLVMScalarOpts.lib LLVMSelectionDAG.lib LLVMSupport.lib LLVMTarget.lib LLVMTransformUtils.lib LLVMVectorize.lib LLVMX86AsmParser.lib LLVMX86AsmPrinter.lib LLVMX86CodeGen.lib LLVMX86Desc.lib LLVMX86Disassembler.lib LLVMX86Info.lib LLVMX86Utils.lib
		)
	elseif(LLVM_VERSION STREQUAL "3.4")
		SET(LLVM_LIBS
			LLVMAnalysis.lib LLVMAsmParser.lib LLVMAsmPrinter.lib LLVMBitReader.lib LLVMBitWriter.lib LLVMCodeGen.lib LLVMCore.lib LLVMExecutionEngine.lib LLVMInstCombine.lib LLVMInstrumentation.lib LLVMInterpreter.lib LLVMipa.lib LLVMipo.lib 
			LLVMJIT.lib 
			LLVMLinker.lib LLVMMC.lib LLVMMCDisassembler.lib LLVMMCJIT.lib LLVMMCParser.lib LLVMObject.lib LLVMRuntimeDyld.lib LLVMScalarOpts.lib LLVMSelectionDAG.lib LLVMSupport.lib LLVMTarget.lib LLVMTransformUtils.lib LLVMVectorize.lib LLVMX86AsmParser.lib LLVMX86AsmPrinter.lib LLVMX86CodeGen.lib LLVMX86Desc.lib LLVMX86Disassembler.lib LLVMX86Info.lib LLVMX86Utils.lib
		)
	else()
		MESSAGE("Unsupported LLVM version ${LLVM_VERSION}")
	endif()
		
	#SET(CMAKE_EXE_LINKER_FLAGS		"${CMAKE_EXE_LINKER_FLAGS} ${LLVM_LIBS}")
	#SET(CMAKE_MODULE_LINKER_FLAGS	"${CMAKE_MODULE_LINKER_FLAGS} ${LLVM_LIBS}")
	
	target_link_libraries(${CURRENT_TARGET} 
		${LLVM_LIBS})

elseif(APPLE)
	# get the llvm libs
	# NOTE: LLVM 3.6+ requires --system-libs also.
	execute_process(COMMAND "${llvmdir}/bin/llvm-config" "--ldflags" "--system-libs" "--libs" "all" OUTPUT_VARIABLE LLVM_LIBS_OUT OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(REPLACE "\n" " " LLVM_LIBS_FINAL ${LLVM_LIBS_OUT})
		
	MESSAGE("LLVM_LIBS_FINAL: ${LLVM_LIBS_FINAL}")
		
	target_link_libraries(${CURRENT_TARGET} 
		${LLVM_LIBS_FINAL})

else()
	# get the llvm libs
	# NOTE: LLVM 3.6+ requires --system-libs also.
	execute_process(COMMAND "${llvmdir}/bin/llvm-config" "--ldflags" "--system-libs" OUTPUT_VARIABLE LLVM_LIBS_OUT OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(REPLACE "\n" " " LLVM_LIBS_FINAL ${LLVM_LIBS_OUT})
		
	# Manually append LLVM library name (We will link against libLLVM-6.0.so)
	set(LLVM_LIBS_FINAL "${LLVM_LIBS_FINAL} -lLLVM-6.0")
	
	MESSAGE("LLVM_LIBS_FINAL: ${LLVM_LIBS_FINAL}")
		
	target_link_libraries(${CURRENT_TARGET} 
		${LLVM_LIBS_FINAL})
	
	# get the llvm flags
	#execute_process(COMMAND "${llvmdir}/bin/llvm-config" "--ldflags" OUTPUT_VARIABLE LLVM_FLAGS_OUT OUTPUT_STRIP_TRAILING_WHITESPACE)
	#string(REPLACE "\n" " " LLVM_FLAGS_FINAL ${LLVM_FLAGS_OUT})
	
	#SET(CMAKE_EXE_LINKER_FLAGS		"${CMAKE_EXE_LINKER_FLAGS} ${LLVM_FLAGS_FINAL}")
	#SET(CMAKE_MODULE_LINKER_FLAGS	"${CMAKE_MODULE_LINKER_FLAGS} ${LLVM_FLAGS_FINAL}")
endif()
