#include <cstdint>
#include <iostream>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

void registry_handle_global(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t versions){
    std::cout<<"Interface: "<<interface<<std::endl;
}
void registry_handle_global_remove(void* data, wl_registry* registry, uint32_t id){}

int main(int argc, char* argv[]) {
    if(argc < 2){
        std::cout<<"Error"<<std::endl;
        return 1;
    }
   
    std::cout<<"Wallpaper: "<<argv[1]<<std::endl;
    
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

    wl_display_disconnect(display);
    return 0;
}