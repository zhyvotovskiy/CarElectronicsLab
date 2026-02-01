#include "SplashScreen.h"

// Внешнее объявление изображения логотипа Opel из opel_logo_320_172.c
extern "C" {
    extern const lv_img_dsc_t opel_logo_320_172;
}

void SplashScreen_Show(uint32_t duration_ms)
{
    // Устанавливаем черный фон
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);

    // Создаем объект изображения
    lv_obj_t * img = lv_img_create(lv_scr_act());

    // Устанавливаем картинку
    lv_img_set_src(img, &opel_logo_320_172);

    // Центрируем изображение
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    // Принудительно обновляем экран
    lv_refr_now(NULL);

    // Задержка для показа заставки
    delay(duration_ms);

    // Удаляем объект изображения
    lv_obj_del(img);
}
