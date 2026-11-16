idf_component_register(
    SRCS "main.c" "pocket-co-state.c" "pocket-co-content.c" "pocket-co-ui.c"
         "../assets/fonts/pocket-co-font-16.c"
         "../assets/fonts/pocket-co-font-22.c"
         "../assets/images/pocket-co-images.c"
    INCLUDE_DIRS "." "../assets/images"
    REQUIRES bsp esp_timer esp_hw_support esp_system
)
