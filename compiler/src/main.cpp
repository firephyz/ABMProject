#include <iostream>
#include <cstring>
#include <sstream>

#include "config.h"
#include "agent_form.h"
#include "abmodel.h"
#include "parser.h"

struct program_args_t pargs;

void printUsage()
{
  std::cout << "Usage: gamp [options] <input-model>\n";
  std::cout << "\n<input-model> identifies the input xml file specifying the ABM model.\n\n";
  std::cout << "Available Options:\n";
  std::cout << "\t--help\n\t\tPrint this usage message.\n\n";
  std::cout << "\t--target <(cpu|cluster|fpga)>\n\t\tSelects the output target for the compiler. Defaults to \'cpu\'.\n\n";
}

void parseArgs(int argc, char *argv[])
{
  bool arg_parse_error = false;

  for(int arg_index = 1; arg_index < argc; ++arg_index) {
    if(strcmp(argv[arg_index], "--target") == 0) {
      ++arg_index;

      if(arg_index >= argc) {
        std::cerr << "Ran out of arguments while parsing option \'" << argv[arg_index - 1] << "\'\n";
        arg_parse_error = true;
        break;
      }

      if(strcmp(argv[arg_index], "cpu") == 0) {
        pargs.target = OutputTarget::CPU;
      }
      else if(strcmp(argv[arg_index], "cluster") == 0) {
        pargs.target = OutputTarget::Cluster;
      }
      else if(strcmp(argv[arg_index], "fpga") == 0) {
        pargs.target = OutputTarget::FPGA;
      }
      else {
        std::cerr << "Unrecognized output target \'" << argv[arg_index] << "\'\n";
        arg_parse_error = true;
      }
    }
    else if(strcmp(argv[arg_index], "--help") == 0) {
      pargs.print_help = true;
    }
    else if(arg_index == argc - 1) {
      pargs.xml_model_path = argv[arg_index];
    }
    else {
      std::cerr << "Unrecognized command line argument \'" << argv[arg_index] << "\'\n";
      arg_parse_error = true;
    }
  }

  if(arg_parse_error) {
    std::cerr << "Invalid command line arguments. Exiting...\n";
    exit(-1);
  }

  // Make sure user supplies model path
  if(pargs.xml_model_path == NULL) {
    pargs.print_help = true;
  }

  if(pargs.print_help) {
    printUsage();
    exit(0);
  }
}

int main(int argc, char *argv[]) 
{
  parseArgs(argc, argv);

  // load and parse input model
  ABModel& model = parse_model(pargs.xml_model_path);

  // build model library with g++
  std::stringstream model_source = model.to_c_source();
  
  return 0;
}
