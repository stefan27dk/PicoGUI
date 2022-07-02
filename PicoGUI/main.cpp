#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>


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
	 

	bool IsCompleted()
	{
		return graphicsFamily.has_value();
 
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
		PickPhysicalDevice();
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
		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT( _instance, _debugMessenger, nullptr);
		}

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

			if (indices.IsCompleted())
			{
				break;
			}
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




//https://www.youtube.com/watch?v=9NnekWE_67E&list=PLRtjMdoYXLf4A8013lsFWHOgM9qdh0kjH&index=8

//https://www.youtube.com/watch?v=9NnekWE_67E&list=PLRtjMdoYXLf4A8013lsFWHOgM9qdh0kjH&index=8


//
//Borderless resizable window 
//https ://github.com/glfw/glfw/issues/990