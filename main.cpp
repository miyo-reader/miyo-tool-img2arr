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

#include <boost/program_options.hpp>

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

using namespace boost::program_options;

/**************************************************************************************
 * MAIN
 **************************************************************************************/

int main(int argc, char ** argv) try
{
  std::string param_src_filename, param_target_filename;

  options_description desc("Allowed options");
  desc.add_options()
    ("help", "Show this help message.")
    ("src", value<std::string>(&param_src_filename)->required(), "source image file name, i.e. 'miyo-splash.png'.")
    ("target", value<std::string>(&param_target_filename)->required(), "target header file name, i.e. 'miyo-splash.h'.")
    ;

  variables_map vm;
  store(command_line_parser(argc, argv).options(desc).run(), vm);

  if (vm.count("help"))
  {
    std::cout << "Usage: miyo-img2arr [options]" << std::endl;
    std::cout << "  i.e. miyo-img2arr --src miyo-spash.png --target miyo-splash.h" << std::endl;
    std::cout << desc;
    return EXIT_SUCCESS;
  }

  notify(vm);


  cv::Mat const image_src = cv::imread(param_src_filename, cv::IMREAD_GRAYSCALE);

  if (image_src.empty()) {
    std::cerr << "error, could not open '" << param_src_filename << "' for reading." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << param_src_filename << ":" << std::endl
            << "  rows: " << image_src.rows << std::endl
            << "  cols: " << image_src.cols << std::endl;

  /* Read image matrix into a std::vector. */
  std::vector<uint8_t> image_src_pixel_arr;

  for(int i = 0; i < image_src.rows; i++)
    for(int j = 0; j < image_src.cols; j++)
        image_src_pixel_arr.push_back(image_src.at<uint8_t>(i,j));

  std::cout << image_src_pixel_arr.size() << " pixels read." << std::endl;

  /* Compress colour information down to 4 bits per pixel. */
  std::transform(image_src_pixel_arr.begin(),
                 image_src_pixel_arr.end(),
                 image_src_pixel_arr.begin(),
                 [](uint8_t const pixel) -> uint8_t
                 {
                   return (pixel / 16);
                 });

  /* Compress raw pixel array vector into a
   * four bit per pixel vector.
   */
  std::vector<uint8_t> image_src_pixel_arr_compressed;

  for (size_t i = 0; i < image_src_pixel_arr.size(); i+=2)
  {
    uint8_t const sub_pixel = (image_src_pixel_arr[i] << 4) | image_src_pixel_arr[i+1];
    image_src_pixel_arr_compressed.push_back(sub_pixel);
  }

  std::cout << image_src_pixel_arr_compressed.size() << " after compression to 4 bits per pixel." << std::endl;

  /* Generate C header. */
  std::ofstream header_out(param_target_filename);

  if (!header_out.good()) {
    std::cerr << "error, could not open '" << param_target_filename << "' for writing." << std::endl;
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
catch(std::exception const & e)
{
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
