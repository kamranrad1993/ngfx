#include <vector>
#include <string>
#include "ngfx/core/FileUtil.h"
#include "ngfx/graphics/ShaderTools.h"
#include "ngfx/graphics/ShaderTools.h"
using namespace std;
using namespace ngfx;

int main(int argc, char** argv) {
    /*const vector<string> paths = { "ngfx/data/shaders", "nodegl/data/shaders", "nodegl/pynodegl-utils/pynodegl_utils/examples/shaders" };*/
    auto argc_str = to_string(argc);
    vector<string> glslFiles;
    string outDir;
    if (argc > 3) {
        vector<string> paths;
        vector<string> extensions;
        paths.assign(argv + 1, argv + 2);
        extensions.assign(argv + 2, argv + argc);
        glslFiles = FileUtil::findFiles(paths, extensions);
        outDir = "data";
    } else {
        const vector<string> paths = { "ngfx/data/shaders"};
        const vector<string> extensions = {".vert", ".frag", ".comp"};
        glslFiles = FileUtil::findFiles(paths, extensions);
        outDir = "ngfx/build/data";
    }
    ShaderTools shaderTools;
    auto spvFiles = shaderTools.compileShaders(glslFiles, outDir, ngfx::ShaderTools::Format::FORMAT_GLSL);
    auto spvMapFiles = shaderTools.generateShaderMaps(glslFiles, outDir, ngfx::ShaderTools::Format::FORMAT_GLSL);
    return 0;
}


