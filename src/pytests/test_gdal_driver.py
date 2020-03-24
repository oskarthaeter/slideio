from testlib import get_test_image_path
import slideio
import os

def test_open_slide():
    image_path = get_test_image_path("gdal","img_2448x2448_3x8bit_SRC_RGB_ducks.png")
    slide = slideio.open_slide(image_path, "GDAL")
    numb_scenes = slide.num_scenes
    assert numb_scenes == 1