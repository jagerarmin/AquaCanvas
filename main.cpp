#include <cstdint>
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <cstring>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

// Функция которая срабатывает каждый раз, когда wayland сообщает о новом интерфейсе
static void registry_handle_global(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version) {
    // пока выводим в терминал все что предлогает Hyprland
    std::cout << "[Registry] Find global API: " << interface << "(ID: " << id << ", Version: " << version << ")" << std::endl;
}

//Функция которая срабатываутб если  интерфейс удаляется
static void registry_handle_global_remove(void* data, wl_registry* registry, uint32_t id) {
    std::cout << "[Registry] Global API & ID" << id << " disconected." << std::endl;
}

// Создаем структуру-"слушатель" и привязываем к ней наши функции
static const wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove
};

int main() {
    const char* home_env = std::getenv("HOME");
// ошибка не найдена Home директория
    if (!home_env) {
        std::cerr << "[Error] Home directory not found" << std::endl;
        return 1;
    }
// директория найдена
    std::cout << "Home directory path successfully found" << home_env << std::endl;
// склеиваем пути в памяти
    std::filesystem::path home_path(home_env);
    std::filesystem::path conf_dir = home_path / ".config" / "Aqua" / "AQCanvas";
    std::filesystem::path wall_dir = home_path / "Aqua" / "AQWallpaperd";
// создание директорий
    std::filesystem::create_directories(conf_dir);
    std::filesystem::create_directories(wall_dir);
// открываем дефолтный дисплей Wayland
    wl_display* display = wl_display_connect(nullptr);
    // предупреждения
    if (!display) {
        std::cerr << "[Error] Failed to connect to Wayland display" << std::endl;
        return 1;
    }
    // подтверждение
    std::cout << "Connected to Wayland display successfully!" << std::endl;

    // получаем объект регистратуры от дисплея 
    wl_registry* registry = wl_display_get_registry(display);

    // Привязываем "слушатуля" к регистратуре
    wl_registry_add_listener(registry, &registry_listener, nullptr);

    // Заставляем Wayland обменяться сообщениями прямо сейчас что бы запустить коллбэки
    wl_display_roundtrip(display);

    // Освобождаем память от объекта регистратуры перед выходом
    wl_registry_destroy(registry);
// вывод подтверждения
    std::cout << "[Success created] Configuration directory:" << conf_dir << std::endl;
    std::cout << "[Success created] Wallpapers directory   :" << wall_dir << std::endl;

    // коректно закрываем подключение
    wl_display_disconnect(display);
    
    return 0;
}