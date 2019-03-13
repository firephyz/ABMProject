#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED

enum class OutputTarget {
  CPU,
  Cluster,
  FPGA,
};

struct program_args_t {
  OutputTarget target = OutputTarget::CPU;
  bool print_help = false;
};

#endif