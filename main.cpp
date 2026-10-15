#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <ostream>
#include <string>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-util.h>
#include "layer-shell.h"
#include "xdg-shell.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

int screen_width = 0;
int screen_height = 0;

wl_surface* surface = nullptr;
zwlr_layer_surface_v1* layer_surface = nullptr;
wl_compositor* compositor = nullptr;
wl_shm* shm = nullptr;
zwlr_layer_shell_v1* layer_shell = nullptr;


void registry_handle_global(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t versions){
    if(strcmp(interface, "wl_compositor") == 0) {
        compositor = (wl_compositor*)wl_registry_bind(registry, id, &wl_compositor_interface, 4);
    }
    if(strcmp(interface, "wl_shm") == 0) {
        shm = (wl_shm*)wl_registry_bind(registry, id, &wl_shm_interface, 1);
    }
    if(strcmp(interface, "zwlr_layer_shell_v1") == 0){
        layer_shell = (zwlr_layer_shell_v1*)wl_registry_bind(registry, id, &zwlr_layer_shell_v1_interface, 1);
    }
}
void registry_handle_global_remove(void* data, wl_registry* registry, uint32_t id){}

void layer_surface_handle_configure(void* data, zwlr_layer_surface_v1* layer_surface, uint32_t serial, uint32_t width, uint32_t height) {
    screen_width = width;
    screen_height = height;
    zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
}

void layer_surface_handle_closed(void* data, zwlr_layer_surface_v1* layer_surface) {
    
}

int create_shm_file(size_t size) {
    int fd = shm_open("/aqc_wallpaper_buffer", O_RDWR | O_CREAT | O_EXCL, 0600);
    shm_unlink("/aqc_wallpaper_buffer");
    ftruncate(fd, size);
    return fd;
}

int main(int argc, char* argv[]) {
    std::string home_dir = getenv("HOME");
    std::string wall_dir = home_dir + "/AQC_Wall";

    if (!std::filesystem::exists(wall_dir)) {
        std::filesystem::create_directories(wall_dir);
        std::cout<<"Creating wallpaper directory at: "<<wall_dir<<std::endl;
    }

    std::cout<<"Using wallpaper directory: "<<wall_dir<<std::endl;

    std::string selected_wallpaper = "";

    for (const auto& entry : std::filesystem::directory_iterator(wall_dir)) {
        if (entry.is_regular_file()) {
            selected_wallpaper = entry.path().string();
            break;
        }
    }
    
    if (selected_wallpaper.empty()) {
        std::cout<<"Warning. Wallpaper directory is empty. Please drop a PNG/JPG into: "<<wall_dir<<std::endl;
    }

    std::cout<<"Selected file: "<<selected_wallpaper<<std::endl;

    wl_display* display = wl_display_connect(nullptr);

    if (display==nullptr) {
        std::cout<<"Failed to connect Wayland display"<<std::endl;
        return 1;
    }

    std::cout<<"Connected to Wayland display"<<std::endl;

    wl_registry* registry = wl_display_get_registry(display);

    wl_registry_listener registry_listener = {registry_handle_global, registry_handle_global_remove };
    wl_registry_add_listener(registry, &registry_listener, nullptr);
    wl_display_roundtrip(display);

    if (layer_shell == nullptr) {
        std::cout<<"Error. Wayland compositor does not support layer-shell!"<<std::endl;
        return 1;
    }
    std::cout<<"ALL tools ready!"<<std::endl;

    surface = wl_compositor_create_surface(compositor);
    layer_surface = zwlr_layer_shell_v1_get_layer_surface(layer_shell, surface, nullptr, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "wallpaper");
    
    zwlr_layer_surface_v1_listener layer_listner = { layer_surface_handle_configure, layer_surface_handle_closed };
    zwlr_layer_surface_v1_add_listener(layer_surface, &layer_listner, nullptr);

    zwlr_layer_surface_v1_set_anchor(layer_surface, 15);



    wl_surface_commit(surface);
    wl_display_roundtrip(display);

    int buffer_size = screen_width * screen_height * 4;
    int fd = create_shm_file(buffer_size);

    uint32_t* pool_data = (uint32_t*)mmap(nullptr, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (pool_data == MAP_FAILED) {
        std::cout<<"Error. Failed to mmap shared memory buffer"<<std::endl;
        close(fd);
        wl_display_disconnect(display);
        return 1;
    }

    int img_width, img_hight, img_channels;
    unsigned char* image_pixels = stbi_load(selected_wallpaper.c_str(), &img_width, &img_hight, &img_channels, 4);

    if (image_pixels == nullptr) {
        std::cout<<"Error. Failed to load image file in: "<<selected_wallpaper<<std::endl;
        munmap(pool_data, buffer_size);
        close(fd);
        return 1;
    }

    for (int i = 0; i < screen_width * screen_height; i++) {
        uint8_t r = image_pixels[i * 4];
        uint8_t g = image_pixels[i * 4 + 1];
        uint8_t b = image_pixels[i * 4 + 2];
        uint8_t a = image_pixels[i * 4 + 3];
        pool_data[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    stbi_image_free(image_pixels);
    wl_shm_pool* pool = wl_shm_create_pool(shm, fd, buffer_size);
    wl_buffer* buffer = wl_shm_pool_create_buffer(pool, 0, screen_width, screen_height, screen_width * 4, WL_SHM_FORMAT_ARGB8888);
    
    
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage_buffer(surface, 0, 0, screen_width, screen_height);
    wl_surface_commit(surface);
    wl_display_roundtrip(display);

    wl_shm_pool_destroy(pool);
    close(fd);

    while(wl_display_dispatch(display) != -1) {

    }

    wl_display_disconnect(display);
    return 0;
}

extern "C" {
    #include "layer-shell.code"
    #include "xdg-shell.code"
}