/**
 * Miyo - a open software/open hardware ebook - Reader (img2arr tool)
 * Copyright (C) 2022 Alexander Entinger, LXRobotics
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <cstdlib>
#include <cstdint>

#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

#include <opencv2/opencv.hpp>

/**************************************************************************************
 * CONSTANTS
 **************************************************************************************/

static size_t constexpr BIT_PER_PIXEL  = 4;
static size_t constexpr PIXEL_PER_BYTE = 8 / BIT_PER_PIXEL;

/**************************************************************************************
 * MAIN
 **************************************************************************************/

int main(int /* argc */, char ** /* argv */)
{
  std::string const image_src_filename = "miyo-splash.png";

  cv::Mat const image_src = cv::imread(image_src_filename, cv::IMREAD_GRAYSCALE);

  if (image_src.empty()) {
    std::cerr << "error, could not open '" << image_src_filename << "' for reading." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << image_src_filename << ":" << std::endl
            << "  rows: " << image_src.rows << std::endl
            << "  cols: " << image_src.cols << std::endl;

  /* Read image matrix into a std::vector. */
  std::vector<uint8_t> image_src_pixel_arr;

  for(int i = 0; i < image_src.rows; i++)
    for(int j = 0; j < image_src.cols; j++)
        image_src_pixel_arr.push_back(image_src.at<uint8_t>(i,j) / 16);

  std::cout << image_src_pixel_arr.size() << " pixels read." << std::endl;

  /* Compress raw pixel array vector into a
   * four bit per pixel vector.
   */
  std::vector<uint8_t> image_src_pixel_arr_compressed;
  uint8_t sub_pixel = 0;
  size_t sub_pixel_cnt = (PIXEL_PER_BYTE - 1);
  std::for_each(image_src_pixel_arr.cbegin(),
                image_src_pixel_arr.cend(),
                [&image_src_pixel_arr_compressed, &sub_pixel, &sub_pixel_cnt](uint8_t const pixel)
                {
                  size_t const sub_pixel_shift_amount = sub_pixel_cnt * BIT_PER_PIXEL;
                  sub_pixel |= (pixel << sub_pixel_shift_amount);

                  if (sub_pixel_cnt == 0) {
                    image_src_pixel_arr_compressed.push_back(sub_pixel);
                    sub_pixel = 0;
                  }

                  sub_pixel_cnt = (sub_pixel_cnt > 0) ? (sub_pixel_cnt - 1) : (PIXEL_PER_BYTE - 1);
                });

  std::cout << image_src_pixel_arr_compressed.size() << " after compression to 4 bits per pixel." << std::endl;

  /* Generate C header. */
  std::string const header_filename = "miyo-splash.h";
  std::ofstream header_out(header_filename);

  if (!header_out.good()) {
    std::cerr << "error, could not open '" << header_filename << "' for writing." << std::endl;
    return EXIT_FAILURE;
  }

  header_out << "#ifndef HEADER_H_" << std::endl
             << "#define HEADER_H_" << std::endl
             << std::endl
             << "static uint8_t constexpr MIYO_SPLASH["
             << image_src_pixel_arr_compressed.size()
             << "] = " << std::endl
             << "{" << std::endl
             << "  ";

  std::for_each(image_src_pixel_arr_compressed.cbegin(),
                image_src_pixel_arr_compressed.cend(),
                [&header_out](uint8_t const val)
                {
                  header_out << "0x" << std::hex << static_cast<size_t>(val) << std::dec << ", ";
                });

  header_out << std::endl
             << "};" << std::endl
             << std::endl
             << "#endif" << std::endl;

  header_out.close();

  return EXIT_SUCCESS;
}
