//
// Copyright 2018 Yoshinori Suzuki<wave.suzuki.z@gmail.com>
//
#include <cpptoml.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <uuid/uuid.h>

namespace
{
//////////
// include guard
//
std::string
begin_include_guard(std::ostream& sout, std::string name)
{
  uuid_t        uv;
  uuid_string_t us;
  uuid_generate(uv);
  uuid_unparse_lower(uv, us);
  std::string ig{us};
  std::replace(ig.begin(), ig.end(), '-', '_');
  std::string label = name + "_" + ig;
  sout << "#ifndef " << label << std::endl;
  sout << "#define " << label << std::endl;
  return label;
}

//
void
end_include_guard(std::ostream& sout, std::string label)
{
  sout << "#endif // " << label << std::endl;
}

//////////
//
//
enum class DefaultType : uint8_t
{
  String,
  Char,
  Float,
  Double,
  Int
};

//
DefaultType
check_default_type(std::string type_name, int array_size)
{
  if (type_name.find("string") != std::string::npos)
  {
    return DefaultType::String;
  }
  if (type_name.find("char") != std::string::npos)
  {
    return array_size > 0 ? DefaultType::String : DefaultType::Char;
  }
  if (type_name.find("float") != std::string::npos)
  {
    return DefaultType::Float;
  }
  if (type_name.find("double") != std::string::npos)
  {
    return DefaultType::Double;
  }
  return DefaultType::Int;
}

//////////
//
//
size_t
calc_size(std::string type_name)
{
  static const std::map<std::string, const size_t> size_map = {
      {"std::string", sizeof(std::string)},
      {"char", sizeof(char)},
      {"unsigned char", sizeof(unsigned char)},
      {"short", sizeof(short)},
      {"unsigned short", sizeof(unsigned short)},
      {"int", sizeof(int)},
      {"unsigned int", sizeof(unsigned int)},
      {"long", sizeof(long)},
      {"unsigned long", sizeof(unsigned long)},
      {"long long", sizeof(long long)},
      {"unsigned long long", sizeof(unsigned long long)},
      {"size_t", sizeof(size_t)},
      {"int8_t", sizeof(int8_t)},
      {"uint8_t", sizeof(uint8_t)},
      {"int16_t", sizeof(int16_t)},
      {"uint16_t", sizeof(uint16_t)},
      {"int32_t", sizeof(int32_t)},
      {"uint32_t", sizeof(uint32_t)},
      {"int64_t", sizeof(int64_t)},
      {"uint64_t", sizeof(uint64_t)},
      {"float", sizeof(float)},
      {"double", sizeof(double)},
  };
  auto r = size_map.find(type_name);
  if (r == size_map.end())
  {
    if (type_name.find("*") != std::string::npos)
    {
      return sizeof(void*);
    }
    return 1;
  }
  return r->second;
}

//
size_t
inquiry_bits(size_t& bit_count, bool clear = false)
{
  constexpr size_t bit_size = sizeof(unsigned) * 8;
  if (clear)
  {
    size_t acnt = bit_size - bit_count;
    bit_count += acnt < bit_size ? acnt : 0;
  }
  if (bit_count >= bit_size)
  {
    bit_count -= bit_size;
    return sizeof(unsigned);
  }
  return 0;
}

//
void
alignment(size_t& byte_count, size_t single_size)
{
  size_t remain_size = byte_count % single_size;
  size_t align_size  = single_size - remain_size;
  byte_count += remain_size > align_size ? align_size : 0;
}

//////////
//
//
bool
file_exist(std::string& fname)
{
  std::ifstream ifs(fname);
  return ifs.is_open();
}

//////////
//
//
void
error_exit(std::string msg)
{
  std::cerr << "error: " << msg << std::endl;
  exit(1);
}

//////////
//
//
int
option_check(int argc, char** argv, std::string& output_filename)
{
  int ipos = 1;
  for (int ai = 1; ai < argc; ++ai)
  {
    if (strncmp(argv[ai], "-o", 3) == 0)
    {
      // output file
      if (++ai < argc)
      {
        output_filename = argv[ai];
      }
      else
      {
        error_exit("argument error: no output filename");
      }
    }
    else
    {
      break;
    }
    ipos = ai + 1;
  }
  if (ipos >= argc)
  {
    error_exit("no set config file");
  }
  return ipos;
}

} // namespace

//////////
//
//
int
main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cerr << "usage: " << argv[0] << std::endl;
    return -1;
  }

  // option & filenames
  std::string output_filename;
  int         ipos = option_check(argc, argv, output_filename);
  std::string input_filename{argv[ipos]};
  if (file_exist(input_filename) == false)
  {
    error_exit(std::string{"file not found: "} + input_filename);
  }

  // output file
  std::unique_ptr<std::ofstream> outfile{};
  if (output_filename.empty() == false)
  {
    auto newfile = std::make_unique<std::ofstream>(output_filename);
    outfile      = std::move(newfile);
    if (outfile->is_open() == false)
    {
      error_exit(std::string{"file open failed: "} + output_filename);
    }
  }
  std::ostream& sout = outfile ? *outfile : std::cout;

  //
  sout << "// input config file: " << input_filename << std::endl;
  auto config = cpptoml::parse_file(input_filename);

  //
  auto struct_name = config->get_qualified_as<std::string>("struct.name");
  if (!struct_name)
  {
    error_exit("not found {struct.name}");
  }

  // include guard
  auto ig_label = begin_include_guard(sout, *struct_name);

  auto include_list = config->get_qualified_array_of<std::string>("struct.include");
  if (include_list)
  {
    for (const auto& i : *include_list)
    {
      sout << "#include <" << i << ">" << std::endl;
    }
  }

  // define list
  auto define_list = config->get_table_qualified("define");

  // namespace
  auto namespace_label = config->get_qualified_as<std::string>("struct.namespace");
  if (namespace_label)
  {
    sout << "namespace " << *namespace_label << " {" << std::endl;
  }

  // struct body
  size_t byte_count = 0;
  size_t bit_count  = 0;
  sout << "struct " << *struct_name << " {" << std::endl;
  auto members = config->get_table_array("member");
  for (const auto& member : *members)
  {
    auto name = member->get_as<std::string>("name");
    auto type = member->get_as<std::string>("type");
    auto bits = member->get_as<int>("bits");

    if (bits)
    {
      // bit field
      sout << "\t" << *type << " " << *name << " : " << *bits;
      bit_count += *bits;
      byte_count += inquiry_bits(bit_count);
    }
    else
    {
      sout << "\t" << *type << " " << *name;

      // array
      int  array_size = 0;
      auto array      = member->get_as<int>("array");
      if (array)
      {
        sout << "[" << *array << "]";
        array_size = *array;
      }
      else
      {
        auto type_array = member->get_as<std::string>("array");
        if (type_array)
        {
          sout << "[" << *type_array << "]";
          if (define_list)
          {
            auto sym   = define_list->get_as<int>(*type_array);
            array_size = sym ? *sym : 1;
          }
          else
          {
            array_size = 1;
          }
        }
      }

      // default value
      auto put_v = [&](auto v, std::string f = "", std::string b = "") {
        if (v)
        {
          sout << "{ " << f << *v << b << " }";
        }
      };
      switch (check_default_type(*type, array_size))
      {
      case DefaultType::String:
      {
        auto v = member->get_as<std::string>("default");
        put_v(v, "\"", "\"");
      }
      break;
      case DefaultType::Char:
      {
        auto v = member->get_as<std::string>("default");
        put_v(v, "'", "'");
      }
      break;
      case DefaultType::Float:
      {
        auto v = member->get_as<double>("default");
        put_v(v, "", "f");
      }
      break;
      case DefaultType::Double:
      {
        auto v = member->get_as<double>("default");
        put_v(v);
      }
      break;
      default:
      {
        auto v = member->get_as<int>("default");
        put_v(v);
      }
      break;
      }

      size_t single_size = calc_size(*type);
      byte_count += inquiry_bits(bit_count, true);
      alignment(byte_count, single_size);
      byte_count += single_size * (array_size > 0 ? array_size : 1);
    }

    // finish
    sout << ";" << std::endl;
  }

  byte_count += inquiry_bits(bit_count, true);
  alignment(byte_count, sizeof(unsigned));
  sout << "}; // struct size = " << byte_count << " bytes." << std::endl;

  // namespace
  if (namespace_label)
  {
    sout << "} // namespace " << *namespace_label << std::endl;
  }

  // include guard
  end_include_guard(sout, ig_label);

  return 0;
}
