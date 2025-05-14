project(libmpeg2)

set(SOURCES
    libmpeg2/libmpeg2/alloc.c
    libmpeg2/libmpeg2/cpu_accel.c
    libmpeg2/libmpeg2/cpu_state.c
    libmpeg2/libmpeg2/decode.c
    libmpeg2/libmpeg2/header.c
    libmpeg2/libmpeg2/idct.c
    libmpeg2/libmpeg2/idct_alpha.c
    libmpeg2/libmpeg2/idct_altivec.c
    libmpeg2/libmpeg2/idct_mmx.c
    libmpeg2/libmpeg2/motion_comp.c
    libmpeg2/libmpeg2/motion_comp_alpha.c
    libmpeg2/libmpeg2/motion_comp_altivec.c
    libmpeg2/libmpeg2/motion_comp_arm.c
    libmpeg2/libmpeg2/motion_comp_mmx.c
    libmpeg2/libmpeg2/motion_comp_vis.c
    libmpeg2/libmpeg2/slice.c
    libmpeg2/libmpeg2/convert/rgb.c
    libmpeg2/libmpeg2/convert/rgb_mmx.c
    libmpeg2/libmpeg2/convert/rgb_vis.c
    libmpeg2/libmpeg2/convert/uyvy.c
)

add_library(libmpeg2 ${SOURCES})

target_include_directories(libmpeg2 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

