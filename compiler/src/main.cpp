// TODO Compilation steps currently are only for cpu and clusters. Needs tweeking to support hdl as well.
// TODO output-lib-name program argument is only required if the target is cpu or cluster.

#include <iostream>
#include <cstring>
#include <string>
#include <sys/stat.h> // for stat
#include <time.h>
#include <fstream>

#include "config.h"
#include "agent_form.h"
#include "abmodel.h"
#include "parser.h"

struct program_args_t pargs;

void printUsage()
{
  std::cout << "Usage: gamp [options] <input-model>\n";
  std::cout << "\n<input-model> Specifies the input xml file specifying the ABM model.\n";
  std::cout << "\nAvailable Options:\n";
  std::cout << "\t--help\n\t\tPrint this usage message.\n\n";
  std::cout << "\t-target <(cpu|cluster|fpga)>\n\t\tSelects the output target for the compiler. Defaults to \'cpu\'.\n\n";
  std::cout << "\t-runtime-path <path>\n\t\tPoints GAMP to the runtime directory that contains the various target runtime sources.\n\t\tDefaults to \'./runtime/\'.\n";
  std::cout << "\t-o <output-lib-name>\n\t\tSpecifies the output library file name. This can be used with the runtime to simulate the model.\n\t\t";
    std::cout << "Defaults to the name of the xml input file but with a file extension of \'.so\'\n";
  std::cout << std::endl;
}

bool check_runtime_sources()
{
  std::string cpu_path = pargs.runtime_path + std::string("cpu/include");
  struct stat buf;
  if(stat(cpu_path.c_str(), &buf)) {
    return false;
  }

  return true;
}

void parseArgs(int argc, char *argv[])
{
  bool arg_parse_error = false;

  for(int arg_index = 1; arg_index < argc; ++arg_index) {
    if(strcmp(argv[arg_index], "-target") == 0) {
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
    else if(strcmp(argv[arg_index], "-runtime-path") == 0) {
      ++arg_index;

      if(arg_index >= argc) {
        std::cerr << "Ran out of arguments while parsing option \'" << argv[arg_index - 1] << "\'\n";
        arg_parse_error = true;
        break;
      }

      pargs.runtime_path = std::string(argv[arg_index]);
      if(pargs.runtime_path[pargs.runtime_path.length() - 1] != '/') {
        pargs.runtime_path += "/";
      }
    }
    else if(strcmp(argv[arg_index], "-o") == 0) {
      ++arg_index;

      if(arg_index >= argc) {
        std::cerr << "Ran out of arguments while parsing option \'" << argv[arg_index - 1] << "\'\n";
        arg_parse_error = true;
        break;
      }

      pargs.model_lib_name = std::string(argv[arg_index]);
    }
    else if(strcmp(argv[arg_index], "--help") == 0) {
      pargs.print_help = true;
    }
    else if(arg_index == argc - 1) {
      pargs.xml_model_path = std::string(argv[arg_index]);
      auto extension_pos = pargs.xml_model_path.rfind(".xml");
      if(extension_pos == std::string::npos) {
        std::cerr << "Input model file needs to be an XML file.\n" << std::endl;
        arg_parse_error = true;
      }
    }
    else {
      std::cerr << "Unrecognized command line argument \'" << argv[arg_index] << "\'\n";
      arg_parse_error = true;
    }
  }

  // Make sure user supplies model path
  if(pargs.xml_model_path.length() == 0) {
    std::cerr << "Must supply the input xml file path as the last argument.\n";
    exit(-1);
  }

  // set default model lib name
  if(pargs.model_lib_name.length() == 0) {
    pargs.model_lib_name = std::string(pargs.xml_model_path);
    auto extension_pos = pargs.model_lib_name.rfind(".xml");
    pargs.model_lib_name = pargs.model_lib_name.substr(0, extension_pos);
    auto last_slash_pos = pargs.model_lib_name.rfind("/");
    if(last_slash_pos != std::string::npos) {
      pargs.model_lib_name = pargs.model_lib_name.substr(last_slash_pos + 1);
    }
    pargs.model_lib_name += ".so";
  }

  if(arg_parse_error) {
    std::cerr << "Invalid command line arguments. Exiting...\n";
    exit(-1);
  }

  if(pargs.print_help) {
    printUsage();
    exit(0);
  }

  if(!check_runtime_sources()) {
    std::cerr << "Runtime path \'" << pargs.runtime_path << "\' does not contain runtime sources needed for model compilation." << std::endl;
    exit(-1);
  }
}

int main(int argc, char *argv[])
{
  parseArgs(argc, argv);

  // load and parse input model
  ABModel& model = parse_model(pargs.xml_model_path.c_str());

//  std::cout << model.to_string();

  // write out ast as code to file
  std::string unique_id = std::to_string((int)time(NULL));
  std::string source_file_name = "./model-" + unique_id + ".cpp";
  std::string object_file_name = "./model-" + unique_id + ".o";
  std::ofstream file_out(source_file_name);
  std::string model_source = model.to_c_source();
  file_out << model_source << "\n";
  file_out.close();

  // compile file
  std::string object_command = "g++ --std=c++14 -c -g -o " + object_file_name + " -fPIC -I" + pargs.runtime_path + "cpu/include/ " + source_file_name;
  system(object_command.c_str());
  struct stat buf;
  if(stat(object_file_name.c_str(), &buf)) {
    std::cerr << "Failed to compile the generated source file.\n";
  }
  else {
    // link the file
    std::string link_command = "g++ -g -shared -o " + pargs.model_lib_name + " " + object_file_name;
    system(link_command.c_str());
    if(stat(pargs.model_lib_name.c_str(), &buf)) {
      std::cerr << "Failed to link the generated object file.\n";
    }

    // cleanup
    remove(object_file_name.c_str());
  }

  // cleanup
  remove(source_file_name.c_str());

  return 0;
}
