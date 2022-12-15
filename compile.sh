glslc vertex.vert -o vert.spv
glslc fragment.frag -o frag.spv

mkdir -p build/shaders
rm build/shaders/*.spv
mv *.spv build/shaders