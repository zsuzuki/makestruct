//
// Copyright 2018 Yoshinori Suzuki<wave.suzuki.z@gmail.com>
//
#include <cpptoml.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

int
main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cerr << "usage: " << argv[0] << std::endl;
    return -1;
  }

  //
  std::cout << "config: " << argv[1] << std::endl;
  auto config = cpptoml::parse_file(argv[1]);

  //
  auto struct_name = config->get_qualified_as<std::string>("struct.name");
  if (!struct_name)
  {
    std::cerr << "not found {struct.name}" << std::endl;
    return -1;
  }
  std::cout << "struct " << *struct_name << " {" << std::endl;
  auto members = config->get_table_array("member");
  for (const auto& member : *members)
  {
    auto name = member->get_as<std::string>("name");
    auto size = member->get_as<std::string>("size");
    std::cout << "\t" << *size << " " << *name << ";" << std::endl;
  }
  std::cout << "};" << std::endl;

  return 0;
}
