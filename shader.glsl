#version 450

layout(set = 0, binding = 0) buffer InBuffer {
    int InData[]; // Define the input buffer
};

layout(set = 0, binding = 1) buffer OutBuffer {
    int OutData[]; // Define the output buffer
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) // Define the local workgroup size
void main() {
    uint id = gl_GlobalInvocationID.x; // Get the global invocation ID
    OutData[id] = InData[id] * InData[id]; // Perform the computation
}
