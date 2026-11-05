# Live comparisons require the original PS2 program open in Ghidra. Keep them
# explicit so a normal game build does not depend on a local analysis service.
find_program(KYA_VERIFY_PYTHON NAMES python python3)
option(KYA_ENABLE_FUNCTION_VERIFICATION_TESTS "Register offline function verification regressions with CTest" OFF)
option(KYA_ENABLE_LIVE_FUNCTION_VERIFICATION_TESTS "Register live Ghidra comparisons with CTest (requires PS2 bridge)" OFF)
if(KYA_VERIFY_PYTHON)
    add_custom_target(verify-functions
        COMMAND "${KYA_VERIFY_PYTHON}" "${PROJECT_SOURCE_DIR}/tools/function_verify/check.py" --build-dir "${CMAKE_BINARY_DIR}" --output-root "${CMAKE_BINARY_DIR}/verification"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        USES_TERMINAL
        COMMENT "Verify compiled Windows functions against original PS2 P-code")
    add_custom_target(verify-functions-offline
        COMMAND "${KYA_VERIFY_PYTHON}" "${PROJECT_SOURCE_DIR}/tools/function_verify/check.py" --offline --output-root "${CMAKE_BINARY_DIR}/verification"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        USES_TERMINAL
        COMMENT "Run offline function verification regressions")
    if(KYA_ENABLE_FUNCTION_VERIFICATION_TESTS)
        add_test(NAME FunctionVerifyOffline
            COMMAND "${KYA_VERIFY_PYTHON}" "${PROJECT_SOURCE_DIR}/tools/function_verify/check.py" --offline --regression-tier fast --output-root "${CMAKE_BINARY_DIR}/verification/offline-fast")
        # Bounded fighter heap/registry partitions execute complete resolver
        # bodies; the full offline suite takes several minutes.
        set_tests_properties(FunctionVerifyOffline PROPERTIES LABELS "verification;offline;fast" TIMEOUT 900)
        add_test(NAME FunctionVerifyRetained
            COMMAND "${KYA_VERIFY_PYTHON}" "${PROJECT_SOURCE_DIR}/tools/function_verify/check.py" --offline --regression-tier retained --output-root "${CMAKE_BINARY_DIR}/verification/offline-retained")
        set_tests_properties(FunctionVerifyRetained PROPERTIES LABELS "verification;offline;retained" TIMEOUT 1200)
        add_test(NAME FunctionVerifyAnimation
            COMMAND "${KYA_VERIFY_PYTHON}" "${PROJECT_SOURCE_DIR}/tools/function_verify/check.py" --offline --regression-tier animation --output-root "${CMAKE_BINARY_DIR}/verification/offline-animation")
        set_tests_properties(FunctionVerifyAnimation PROPERTIES LABELS "verification;offline;animation" TIMEOUT 1800)
    endif()
    if(KYA_ENABLE_LIVE_FUNCTION_VERIFICATION_TESTS)
        add_test(NAME FunctionVerifyLive
            COMMAND "${KYA_VERIFY_PYTHON}" "${PROJECT_SOURCE_DIR}/tools/function_verify/check.py" --build-dir "${CMAKE_BINARY_DIR}" --output-root "${CMAKE_BINARY_DIR}/verification")
        set_tests_properties(FunctionVerifyLive PROPERTIES LABELS "verification;live" TIMEOUT 1800)
    endif()
elseif(KYA_ENABLE_FUNCTION_VERIFICATION_TESTS OR KYA_ENABLE_LIVE_FUNCTION_VERIFICATION_TESTS)
    message(FATAL_ERROR "Function verification requires Python and tools/function_verify/requirements.txt")
endif()
