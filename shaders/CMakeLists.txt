file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

set(ALL_SPV_OUTPUTS "")

set(SHADER_SRC "${CMAKE_CURRENT_SOURCE_DIR}")
set(SHADER_DST "${CMAKE_BINARY_DIR}/shaders")

macro(compile_shader SRC_FILE DST_FILE)
    add_custom_command(
        OUTPUT "${SHADER_DST}/${DST_FILE}"
        COMMAND glslc "${SHADER_SRC}/${SRC_FILE}" -o "${SHADER_DST}/${DST_FILE}"
        DEPENDS "${SHADER_SRC}/${SRC_FILE}"
    )

    list(APPEND ALL_SPV_OUTPUTS "${SHADER_DST}/${DST_FILE}")
endmacro()

compile_shader("comp.vert" "compVert.spv")
compile_shader("comp.frag" "compFrag.spv")
compile_shader("shader.vert" "vert.spv")
compile_shader("shader.frag" "frag.spv")
compile_shader("shader.comp" "comp.spv")
compile_shader("screen.vert" "screenVert.spv")
compile_shader("synthwave.frag" "synthwaveFrag.spv")
compile_shader("text2d.vert" "text2dV.spv")
compile_shader("text.frag" "textF.spv")
compile_shader("gltfShader.vert" "gltfVert.spv")
compile_shader("gltfShader.frag" "gltfFrag.spv")
compile_shader("rec.vert" "recVert.spv")
compile_shader("rec.frag" "recFrag.spv")
compile_shader("texture.frag" "textureFrag.spv")
compile_shader("texture.comp" "textureComp.spv")

add_custom_target(shaders DEPENDS ${ALL_SPV_OUTPUTS})
