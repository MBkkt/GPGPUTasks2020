#include <CL/cl.h>
#include <libclew/ocl_init.h>

#include <vector>
#include <sstream>
#include <iostream>
#include <stdexcept>


template<typename T>
std::string to_string(T value) {
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

void reportError(cl_int err, const std::string &filename, int line) {
    if (CL_SUCCESS == err)
        return;

    // Таблица с кодами ошибок:
    // libs/clew/CL/cl.h:103
    // P.S. Быстрый переход к файлу в CLion: Ctrl+Shift+N -> cl.h (или даже с номером строки: cl.h:103) -> Enter
    std::string message = "OpenCL error code " + to_string(err) + " encountered at " + filename + ":" + to_string(line);
    throw std::runtime_error(message);
}

#define OCL_SAFE_CALL(expr) reportError(expr, __FILE__, __LINE__)


int main() {
    // Пытаемся слинковаться с символами OpenCL API в runtime (через библиотеку libs/clew)
    if (!ocl_init())
        throw std::runtime_error("Can't init OpenCL driver!");

    // Откройте 
    // https://www.khronos.org/registry/OpenCL/sdk/1.2/docs/man/xhtml/
    // Нажмите слева: "OpenCL Runtime" -> "Query Platform Info" -> "clGetPlatformIDs"
    // Прочитайте документацию clGetPlatformIDs и убедитесь что этот способ узнать сколько есть платформ соответствует документации:
    cl_uint platformsCount = 0;
    OCL_SAFE_CALL(clGetPlatformIDs(0, nullptr, &platformsCount));
    std::cout << "Number of OpenCL platforms: " << platformsCount << std::endl;

    // Тот же метод используется для того чтобы получить идентификаторы всех платформ - сверьтесь с документацией, что это сделано верно:
    std::vector<cl_platform_id> platforms(platformsCount);
    OCL_SAFE_CALL(clGetPlatformIDs(platformsCount, platforms.data(), nullptr));

    for (int platformIndex = 0; platformIndex < platformsCount; ++platformIndex) {
        std::cout << "Platform #" << (platformIndex + 1) << "/" << platformsCount << std::endl;
        cl_platform_id platform = platforms[platformIndex];

        // Откройте документацию по "OpenCL Runtime" -> "Query Platform Info" -> "clGetPlatformInfo"
        // Не забывайте проверять коды ошибок с помощью макроса OCL_SAFE_CALL
        {
            size_t platformNameSize = 0;
            OCL_SAFE_CALL(clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, nullptr, &platformNameSize));
            std::vector<unsigned char> platformName(platformNameSize, 0);
            OCL_SAFE_CALL(
                    clGetPlatformInfo(platform, CL_PLATFORM_NAME, platformNameSize, platformName.data(), nullptr));
            std::cout << "    Platform name: " << platformName.data() << std::endl;
        }
        // CL_​INVALID_​VALUE if param_name is not one of the supported values or if size in bytes specified by param_value_size is < size of return type as specified in the OpenCL Platform Queries table, and param_value is not a NULL value.
        // Попробуйте вместо CL_PLATFORM_NAME передать какое-нибудь случайное число - например 239
        // Т.к. это некорректный идентификатор параметра платформы - то метод вернет код ошибки
        // Макрос OCL_SAFE_CALL заметит это, и кинет ошибку с кодом
        // Откройте таблицу с кодами ошибок:
        // libs/clew/CL/cl.h:103
        // P.S. Быстрый переход к файлу в CLion: Ctrl+Shift+N -> cl.h (или даже с номером строки: cl.h:103) -> Enter
        // Найдите там нужный код ошибки и ее название
        // Затем откройте документацию по clGetPlatformInfo и в секции Errors найдите ошибку, с которой столкнулись
        // в документации подробно объясняется, какой ситуации соответствует данная ошибка, и это позволит проверив код понять чем же вызвана данная ошибка (не корректным аргументом param_name)
        // Обратите внимание что в этом же libs/clew/CL/cl.h файле указаны всевоможные defines такие как CL_DEVICE_TYPE_GPU и т.п.
        // Аналогично тому как был запрошен список идентификаторов всех платформ - так и с названием платформы, теперь, когда известна длина названия - его можно запросить:


        // Запросите и напечатайте так же в консоль вендора данной платформы
        {
            size_t platformVendorSize = 0;
            OCL_SAFE_CALL(clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, 0, nullptr, &platformVendorSize));
            std::vector<unsigned char> platformVendor(platformVendorSize, 0);
            OCL_SAFE_CALL(
                    clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, platformVendorSize, platformVendor.data(),
                                      nullptr));
            std::cout << "    Platform vendor: " << platformVendor.data() << std::endl;
        }
        // Запросите число доступных устройств данной платформы (аналогично тому как это было сделано для запроса числа доступных платформ - см. секцию "OpenCL Runtime" -> "Query Devices")
        cl_uint devicesCount = 0;
        OCL_SAFE_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &devicesCount));
        std::cout << "    Number of OpenCL devices: " << devicesCount << std::endl;
        std::vector<cl_device_id> devices(devicesCount);
        OCL_SAFE_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, devicesCount, devices.data(), nullptr));

        for (int deviceIndex = 0; deviceIndex < devicesCount; ++deviceIndex) {
            cl_device_id device = devices[deviceIndex];
            // Запросите и напечатайте в консоль:
            {
                // - Название устройства
                size_t deviceNameSize = 0;
                OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_NAME, 0, nullptr, &deviceNameSize));
                std::vector<unsigned char> deviceName(deviceNameSize, 0);
                OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_NAME, deviceNameSize, deviceName.data(), nullptr));
                std::cout << "        Platform device name: " << deviceName.data() << std::endl;
            }
            {
                // - Тип устройства (видеокарта/процессор/что-то странное)
                size_t deviceTypeSize = 0;
                OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_TYPE, 0, nullptr, &deviceTypeSize));
                cl_device_type deviceType = CL_DEVICE_TYPE_ALL;
                OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_TYPE, deviceTypeSize, &deviceType, nullptr));
                std::cout << "        Platform device type: ";
                switch (deviceType) {
                    case CL_DEVICE_TYPE_DEFAULT:
                        std::cout << "DEFAULT";
                        break;
                    case CL_DEVICE_TYPE_CPU:
                        std::cout << "CPU";
                        break;
                    case CL_DEVICE_TYPE_GPU:
                        std::cout << "GPU";
                        break;
                    case CL_DEVICE_TYPE_ACCELERATOR:
                        std::cout << "ACCELERATOR";
                        break;
                    case CL_DEVICE_TYPE_ALL:
                        std::cout << "ALL";
                        break;
                    default:
                        std::cout << "WTF";
                        break;
                }
                std::cout << std::endl;
            }
            {
                // - Размер памяти устройства в мегабайтах
                size_t deviceMemorySizeSize = 0;
                OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, 0, nullptr, &deviceMemorySizeSize));
                cl_uint deviceMemorySize = 0;
                OCL_SAFE_CALL(
                        clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, deviceMemorySizeSize, &deviceMemorySize,
                                        nullptr));
                std::cout << "        Platform device global memory size: " << deviceMemorySize / 1024 / 1024 << "MB"
                          << std::endl;
            }
            // - Еще пару или более свойств устройства, которые вам покажутся наиболее интересными
            {
                size_t deviceMaxMemAllocSize = 0;
                OCL_SAFE_CALL(
                        clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, 0, nullptr, &deviceMaxMemAllocSize));
                cl_uint deviceMaxMemAlloc = 0;
                OCL_SAFE_CALL(
                        clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, deviceMaxMemAllocSize,
                                        &deviceMaxMemAlloc,
                                        nullptr));
                std::cout << "        Platform device max memory alloc size: " << deviceMaxMemAlloc / 1024 / 1024
                          << "MB" << std::endl;
            }
            {
                size_t deviceMaxWorkGroupSizeSize = 0;
                OCL_SAFE_CALL(
                        clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, 0, nullptr,
                                        &deviceMaxWorkGroupSizeSize));
                cl_uint deviceMaxWorkGroupSize = 0;
                OCL_SAFE_CALL(
                        clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, deviceMaxWorkGroupSizeSize,
                                        &deviceMaxWorkGroupSize,
                                        nullptr));
                std::cout << "        Platform device max work group size: " << deviceMaxWorkGroupSize << std::endl;
            }
        }
    }

    return 0;
}