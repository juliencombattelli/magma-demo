#include <magma/Instance.hpp>
#include <magma/glfw/GlfwStack.hpp>

#include <spdlog/spdlog.h>

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <set>

static constexpr int default_window_width = 800;
static constexpr int default_window_height = 600;

static constexpr const char* applicationName = "Vulkan test";
static constexpr int applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);

uint32_t findGraphicsQueueFamilyIndex(
    std::vector<vk::QueueFamilyProperties> const& queueFamilyProperties) {
    // get the first index into queueFamilyProperties which supports graphics
    std::vector<vk::QueueFamilyProperties>::const_iterator graphicsQueueFamilyProperty
        = std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(),
            [](vk::QueueFamilyProperties const& qfp) {
                return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
            });
    assert(graphicsQueueFamilyProperty != queueFamilyProperties.end());
    return static_cast<uint32_t>(
        std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
}

magma::ContextDebugConfig loadFromFile(const std::string& filename) {
    YAML::Node configFile = YAML::LoadFile(filename);
    magma::ContextDebugConfig debugConfig{
        .validationLayer = configFile["validationLayer"].as<bool>(),
        .debugUtilsExtension = configFile["debugUtilsExtension"].as<bool>(),
        .verbose = configFile["verbose"].as<bool>(),
    };
    return debugConfig;
}

class PhysicalDeviceCompatibilityChecker {
public:
    using CompatibilityTest = std::function<bool(const vk::raii::PhysicalDevice&)>;

    bool isCompatible(const vk::raii::PhysicalDevice& device);

    template<typename TCompatibilityTest>
    void addTest(TCompatibilityTest&& test) {
        tests.push_back(std::forward<TCompatibilityTest>(test));
    }

    static auto areExtensionsAvailable(const std::vector<std::string_view>& extensions) {
        return [extensions](const vk::raii::PhysicalDevice& device) {
            const auto availableExtensions = device.enumerateDeviceExtensionProperties();
            bool extensionsSupported = true;
            // Check availability of all extensions
            for (auto extension : extensions) {
                if (!utils::contains(
                        availableExtensions, extension, &vk::ExtensionProperties::extensionName)) {
                    spdlog::debug("Device does not support extension {}", extension);
                    extensionsSupported = false;
                }
            }
            return extensionsSupported;
        };
    }

private:
    std::vector<CompatibilityTest> tests;
};

static const std::vector<std::string_view> g_requiredExtensions
    = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

struct PhysicalDeviceScorekeeper {
    void score();
    vk::raii::PhysicalDevice best();
    vk::raii::PhysicalDevice worst();
};

struct PhysicalDevicePicker {
    vk::raii::PhysicalDevice pick(const vk::raii::PhysicalDevices& device);
};

int main(int argc, char* argv[]) try {
    magma::ContextCreateInfo createInfo{
        .debugConfig = loadFromFile("config/MagmaDebugConfig.yaml"),
        .applicationName = applicationName,
        .applicationVersion = applicationVersion,
    };
    magma::Instance instance(createInfo);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Do not create an OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    magma::GlfwWindow::Ptr window = magma::GlfwWindow::create(
        default_window_width, default_window_height, applicationName, nullptr, nullptr);

    vk::raii::SurfaceKHR surface = instance.makeSurface(window.get());
    vk::raii::PhysicalDevice physical_device = instance.pickPhysicalDevice(surface);

    uint32_t graphicsQueueFamilyIndex
        = findGraphicsQueueFamilyIndex(physical_device.getQueueFamilyProperties());

    float queue_priority = 1.0f;
    vk::DeviceQueueCreateInfo device_queue_create_info{
        .queueFamilyIndex = graphicsQueueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    vk::PhysicalDeviceFeatures device_features{};

    const std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    vk::DeviceCreateInfo device_create_info{
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &device_queue_create_info,
        //.enabledLayerCount = static_cast<uint32_t>(validation_layers.size()),
        //.ppEnabledLayerNames = validation_layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data(),
        .pEnabledFeatures = &device_features,
    };

    vk::raii::Device device(physical_device, device_create_info);

    while (!glfwWindowShouldClose(window.get())) {
        glfwPollEvents();
    }

    return EXIT_SUCCESS;

} catch (const vk::ExtensionNotPresentError& extensionNotPresent) {
    std::cerr << "Error: " << extensionNotPresent.what() << "\n";
    return EXIT_FAILURE;
} catch (const std::exception& except) {
    std::cerr << "Error: " << except.what() << "\n";
    return EXIT_FAILURE;
}
