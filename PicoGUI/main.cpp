#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>


const unsigned int WIDTH = 800;
const  unsigned int HEIGHT = 600;
const std::vector<const char*> validationLayers =
{
  "VK_LAYER_KHRONOS_validation"
};


// Enable / Disable Validation layer
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif





// Create Validation Layer / Debug Mode --------------------------------------------------------------------------------
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}




// Dispose Validation Layer /  debug mode --------------------------------------------------------------------------------------------
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyUtilsMessengerEXT");

	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}





struct QueueFamilyIndices
{

	std::optional<unsigned int> graphicsFamily;
	std::optional<unsigned int> presentFamily;
	 

	bool IsCompleted()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
 
	}
 };



// Class --------------------------------------------------------------------------------------------
class TriangleApp
{
public:

	void Run()
	{
		InitWindow();
		InitVulkan();
		MainLoop();
		Cleanup();
	}



private:
	GLFWwindow* _window;
	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debugMessenger;
	VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;

	VkDevice _device;

	VkQueue _graphicsQueue;

	VkSurfaceKHR _surface;
	VkQueue _presentQueue;



	// InitWindow ----------------------------------------------------------------------------------------
	void InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}





	// Init Vulkan ----------------------------------------------------------------------------------------
	void InitVulkan()
	{
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice(); 
		CreateLogicalDevice();
	}




	// Main Loop ----------------------------------------------------------------------------------------
	void MainLoop()
	{
		while (!glfwWindowShouldClose(_window))
		{
			glfwPollEvents();
		}
	}




    // Cleanup --------------------------------------------------------------------------------------
	void Cleanup()
	{
		vkDestroyDevice(_device, nullptr);

		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT( _instance, _debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyInstance(_instance, nullptr);
		glfwDestroyWindow(_window);
		glfwTerminate();
	}


	// Create Instance ------------------------------------------------------------------------------
	void CreateInstance()
	{

		if (enableValidationLayers && !CheckValidationLayerSupport())
		{
			throw std::runtime_error("Validation layers requested, but not avaible");
		}



		// Info
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "ENGINE1";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Instance
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;


		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<unsigned int>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<unsigned int>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		

		// Create Instance
		if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Vulkan Instance!"); // Throw error
		}

		//std::vector<VkExtensionProperties> extensions(glfwExtensionCount);
		//vkEnumerateInstanceExtensionProperties(nullptr, &glfwExtensionCount, extensions.data());

		//std::cout << "Aviable Extensions: " << std::endl;  

		//// Print the number of extensionsin the console
		//for (const auto& extension : extensions)
		//{
		//	std::cout << '\t' << extension.extensionName << std::endl;
		//}	   

	}




	// PopulateDebugMessengerCreateInfo ----------------------------------------------------------------------------------------
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
	}




	// SetupDebugMessenger ----------------------------------------------------------------------------------------
	void SetupDebugMessenger()
	{
		if (!enableValidationLayers)
		{
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to setup debug messenger");
		}
	}






	// CreateSurface ----------------------------------------------------------------------------------------
	void CreateSurface()
	{
		if(glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a window surface!");
	    }
	}






	// PickPhysicalDevice ----------------------------------------------------------------------------------------
	void PickPhysicalDevice()
	{
		unsigned int deviceCount = 0;
		vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw std::runtime_error("Failed to find any GPUs with Vulkan!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				_physicalDevice = device;

				break;
			}
		}

		if (_physicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}




	// CreateLogicalDevice ----------------------------------------------------------------------------------------
	void CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(_physicalDevice);


		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<unsigned int> uniqueQueuFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
		 
	/*	VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;*/

		float queuePriority = 1.0f;
		//queueCreateInfo.pQueuePriorities = &queuePriority;

		for (unsigned int queueFamily : uniqueQueuFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};


		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	/*	createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;*/

		createInfo.queueCreateInfoCount = static_cast<unsigned int>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();


		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = 0;

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<unsigned int>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device!");
		}


		vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
		vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);

	}




	// IsDeviceSutable ----------------------------------------------------------------------------------------
	bool IsDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = FindQueueFamilies(device);

		return indices.IsCompleted();
	}




	// FindQueueFamilies ----------------------------------------------------------------------------------------
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;
		unsigned int queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());


		int i = 0;

		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
		


			if (presentSupport)
			{
				indices.presentFamily = i;
			}



			if (indices.IsCompleted())
			{
				break;
			}

			i++;
		}

		return indices;
	}





	// GetRequiredExtensions ----------------------------------------------------------------------------------------
	std::vector<const char*>GetRequiredExtensions()
	{
		// Vars to show amount of extensions / instances
		 unsigned int glfwExtensionCount = 0;
		 const char** glfwExtensions;
		 glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		 std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		 if (enableValidationLayers)
		 {
			 extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		 }


		 return extensions;
		 //// Setting count - extensions / instances
		 //createInfo.enabledExtensionCount = glfwExtensionCount;
		 //createInfo.ppEnabledExtensionNames = glfwExtensions;
		 //createInfo.enabledLayerCount = 0;
	}






	// CheckValidationLayerSupport ----------------------------------------------------------------------------------------
	bool CheckValidationLayerSupport()
	{
		unsigned int layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		
		std::vector<VkLayerProperties> aviableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, aviableLayers.data());

		for (const char *layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : aviableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}





	// DebugCallback  ----------------------------------------------------------------------------------------
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
};


 
// Main ---------------------------------------------------------------------------------------------
int main()
{	 
	TriangleApp app;
	try
	{
		app.Run();
	}
	catch (const std::exception & e)
	{
		std::cerr << e.what() << std::endl;

		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}



//https://www.youtube.com/watch?v=YlVH9Fxh4ys&list=PLRtjMdoYXLf4A8013lsFWHOgM9qdh0kjH&index=10


 
//Borderless resizable window 
//https ://github.com/glfw/glfw/issues/990