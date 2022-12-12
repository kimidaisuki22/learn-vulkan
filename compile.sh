glslc vertex.vert -o vert.spv
glslc fragment.frag -o frag.spv

mkdir -p build/shaders
mv *.spv build/shaders