#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED

#include <string>

enum class OutputTarget {
  CPU,
  Cluster,
  FPGA,
};

struct program_args_t {
  OutputTarget target = OutputTarget::CPU;
  bool print_help = false;
  std::string xml_model_path;
  std::string runtime_path = std::string("./runtime/");
  std::string model_lib_name = std::string(); // will fill during argument parsing
};

#endif