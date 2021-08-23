#include <magma/Instance.hpp>
#include <magma/glfw/GlfwStack.hpp>

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

    const std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    vk::raii::PhysicalDevices physical_devices(instance);
    vk::raii::PhysicalDevice physical_device(instance, nullptr);
    for (auto&& phydevice : physical_devices) {
        const auto& device_properties = phydevice.getProperties();
        std::cout << "Physical device:\n"
                  << "\tname: " << device_properties.deviceName << "\n"
                  << "\ttype: " << vk::to_string(device_properties.deviceType) << "\n";

        // Get all available device extensions
        auto available_device_extensions = phydevice.enumerateDeviceExtensionProperties();
        std::set<std::string> available_device_extension_set;
        for (const auto& extensions : available_device_extensions) {
            available_device_extension_set.insert(extensions.extensionName);
        }

        // Get required device extensions
        std::set<std::string> required_device_extension_set(
            device_extensions.begin(), device_extensions.end());

        // Assert no required device extension is missing
        std::set<std::string> missing_device_extension_set;
        std::ranges::set_difference(required_device_extension_set, available_device_extension_set,
            std::inserter(missing_device_extension_set, missing_device_extension_set.end()));
        if (!missing_device_extension_set.empty()) {
            for (const auto& missing : missing_device_extension_set) {
                std::cerr << "Missing device extension: " << missing << "\n";
            }
            continue;
        }

        // Assert at least on surface format is provided by the device
        auto surface_formats = phydevice.getSurfaceFormatsKHR(*surface);
        if (surface_formats.empty()) {
            std::cerr << "Device does not provide any surface format\n";
            continue;
        }

        // Assert at least on surface present mode is provided by the device
        auto surface_present_modes = phydevice.getSurfacePresentModesKHR(*surface);
        if (surface_present_modes.empty()) {
            std::cerr << "Device does not provide any surface present mode\n";
            continue;
        }

        // Assert device has graphics and presentation capabilities
        auto queue_family_properties = phydevice.getQueueFamilyProperties();
        std::optional<uint32_t> queue_graphics_family_index;
        for (auto queue_family_it = queue_family_properties.begin();
             queue_family_it != queue_family_properties.end(); ++queue_family_it) {
            if (queue_family_it->queueFlags & vk::QueueFlagBits::eGraphics) {
                queue_graphics_family_index
                    = std::distance(queue_family_properties.begin(), queue_family_it);
            }
            if (!queue_graphics_family_index) {
                std::cerr << "Device does not support graphics\n";
                continue;
            }
            auto present_support
                = phydevice.getSurfaceSupportKHR(*queue_graphics_family_index, *surface);
            if (!present_support) {
                std::cerr << "Device does not support presentation\n";
                continue;
            }
        }

        // TODO compute score based on device performance (type and memory available)
        //      and choose the highest ranked device
        physical_device = std::move(phydevice);
        break;
    }

    if (!*physical_device) {
        throw std::runtime_error("No suitable GPU found");
    }

    uint32_t graphicsQueueFamilyIndex
        = findGraphicsQueueFamilyIndex(physical_device.getQueueFamilyProperties());

    float queue_priority = 1.0f;
    vk::DeviceQueueCreateInfo device_queue_create_info{
        .queueFamilyIndex = graphicsQueueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    vk::PhysicalDeviceFeatures device_features{};

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
