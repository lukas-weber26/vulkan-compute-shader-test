#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdio.h>
#include <vulkan/vulkan_core.h>

int main() {
	//need something like this but the values seem questionable and there might be more...
	VkInstance instance;	
	VkApplicationInfo app_info;
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;
	app_info.pApplicationName = "Compute_test";
	app_info.applicationVersion= VK_MAKE_VERSION(1,0,0);
	app_info.pEngineName = "none";
	app_info.engineVersion = VK_MAKE_VERSION(1,0,0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	const char * layers [1] = {"VK_LAYER_KHRONOS_validation"};

	VkInstanceCreateInfo create_info; 
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.enabledExtensionCount = 0;
	create_info.pApplicationInfo = &app_info;
	create_info.enabledLayerCount = 1;
	create_info.ppEnabledLayerNames = layers;

	VkResult result;
	if ((result =vkCreateInstance(&create_info, NULL, &instance)) != VK_SUCCESS) {
		printf("Vulkan error. %d\n", result == VK_ERROR_EXTENSION_NOT_PRESENT);
		exit(0);
	};

	//this is back on track lol
	uint32_t physical_device_count = 0;
	vkEnumeratePhysicalDevices(instance, &physical_device_count, 0);
	VkPhysicalDevice * const physical_devices = malloc(physical_device_count * sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices);
	
	for (int i = 0; i < physical_device_count; i++) {
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(physical_devices[i], &device_properties);
		printf("Name: %s, Version: %d, Memsize: %d KB.\nRunning:\n",device_properties.deviceName, VK_VERSION_MAJOR(device_properties.apiVersion), device_properties.limits.maxComputeSharedMemorySize / 1024);
		
		uint32_t queue_family_properties_count= 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_properties_count,0);
		VkQueueFamilyProperties* const queue_family_properties = malloc(sizeof(VkQueueFamilyProperties) * queue_family_properties_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_properties_count, queue_family_properties);

		for (uint32_t queue_family_index = 0; queue_family_index  < queue_family_properties_count; queue_family_index ++) {

			int graphics = 0;
			int compute = 0;

			if (queue_family_properties[queue_family_index ].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				graphics = 1;
			}
			if (queue_family_properties[queue_family_index ].queueFlags & VK_QUEUE_COMPUTE_BIT) {
				compute= 1;
			}
			
			printf("Queue family supports: graphics %d, compute %d.\n", graphics, compute);

			if (compute == 1) {
				printf("Continuing tests.\n");
				
				VkDeviceQueueCreateInfo device_queue_create_info;
				const float queue_priority = 1.0f;
				device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				device_queue_create_info.pNext = NULL;
				device_queue_create_info.queueFamilyIndex= queue_family_index;	
				device_queue_create_info.flags = 0;
				device_queue_create_info.queueCount= 1;
				device_queue_create_info.pQueuePriorities = &queue_priority;
			
				VkDeviceCreateInfo device_create_info;	
				device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				device_create_info.pNext = NULL;	
				device_create_info.queueCreateInfoCount = 1;
				device_create_info.pQueueCreateInfos = &device_queue_create_info;
				device_create_info.enabledLayerCount = 1;
				device_create_info.flags = 0;
				device_create_info.ppEnabledLayerNames= layers;
				device_create_info.pEnabledFeatures = NULL;
				device_create_info.enabledExtensionCount= 0;

				VkDevice device;
				VkQueue queue;
				VkResult success;

				if ((success=vkCreateDevice(physical_devices[i], &device_create_info, NULL, &device)) != VK_SUCCESS) {
					printf("Vulkan failed to create device.\n");
					exit(0);
				};
				
				vkGetDeviceQueue(device, queue_family_index, 0, &queue);
				
				VkPhysicalDeviceMemoryProperties memory_properties;
				vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory_properties);

				int num_elements = 10;
				const VkDeviceSize buffer_size = num_elements * sizeof(int32_t);
				const VkDeviceSize memory_size = buffer_size * 2;

				for (int k = 0; k < memory_properties.memoryTypeCount; k++) {
					const VkMemoryType memory_type = memory_properties.memoryTypes[k];

					if ((VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & memory_type.propertyFlags) &&
						(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & memory_type.propertyFlags) &&
						(memory_size < memory_properties.memoryHeaps[memory_type.heapIndex].size)) {

						const uint32_t contant_queue_family_index = queue_family_index; 

						VkBufferCreateInfo buffer_info;	
						buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
						buffer_info.pNext = NULL;
						buffer_info.flags = 0;
						buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
						buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
						buffer_info.size = buffer_size;
						buffer_info.queueFamilyIndexCount = 1;
						buffer_info.pQueueFamilyIndices = &contant_queue_family_index;
	
						VkBuffer buffer_one;
						VkBuffer buffer_two;

						if (vkCreateBuffer(device, &buffer_info, NULL, &buffer_one) != VK_SUCCESS)  {
							printf("Failed to create buffer.\n");
							exit(0);
						}	

						if (vkCreateBuffer(device, &buffer_info, NULL, &buffer_two) != VK_SUCCESS)  {
							printf("Failed to create buffer.\n");
							exit(0);
						}	

						//adequate memory found
						VkMemoryAllocateInfo memory_info;
						memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
						memory_info.pNext = NULL;
						memory_info.allocationSize = memory_size;
						memory_info.memoryTypeIndex = k;

						VkDeviceMemory memory;

						if (vkAllocateMemory(device, &memory_info, NULL, &memory) != VK_SUCCESS) {
							printf("Failed to allocate memory\n"); 
							exit(0);
						}
				
						if (vkBindBufferMemory(device, buffer_one, memory, 0) != VK_SUCCESS){
							printf("Failed to bind buffer one\n"); 
							exit(0);
						}
						
						if (vkBindBufferMemory(device, buffer_two, memory, 0) != VK_SUCCESS){
							printf("Failed to bind buffer two\n"); 
							exit(0);
						}

						//time to read the shader into a buffer
						uint32_t shader_buffer [1024];
						int size;
						FILE * fp = fopen("./my_compute_shader.spv", "r");
						size = fread(&shader_buffer, 512, sizeof(uint32_t), fp);
						fclose(fp);
				
						if (size == -1) {
							printf("Read error.\n");
							exit(0);
						}	
						
						VkShaderModuleCreateInfo shader_module_info;
						shader_module_info.flags = 0;
						shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
						shader_module_info.pCode = shader_buffer;
						shader_module_info.pNext = NULL;
						shader_module_info.codeSize = size * sizeof(uint32_t);

						VkShaderModule shader_module;
						if (vkCreateShaderModule(device, &shader_module_info, NULL, &shader_module) != VK_SUCCESS) {
							printf("Failed to create shader module\n"); 
							exit(0);
						}

						break;
					}

				}

			}

		}

		
	}

		
	
}
