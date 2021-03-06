/*********************************************************************************
 * AGASC - Auto-Generated ABM Simulator and Compiler
 * Senior Design Project
 *   Prasidh Arora, Kyle Burge, Logan Johnson, Alex Lee
 *
 * Usage Notes:
 *
 *********************************************************************************
*/

// TODO Compilation steps currently are only for cpu and clusters. Needs tweeking to support hdl as well.
// TODO output-lib-name program argument is only required if the target is cpu or cluster.
// TODO Fix initial agent region enumeration bug where all regions are missing the last one that should be enumerated
// TODO Migrate mlm_data struct produced during code-gen into the runtime. The code getting produced
//      has too much general ABM structure that could be in the runtime.
// TODO Change the generated agent mlm_data structs for the specific agents to have non-static
//      answers and responses structs. This would allow multiple agents to be processed in parallel.
//      Better yet, create them dynamically, one for each thread working on a region of the simulation.
// TODO Many objects (AST, SymbolBindings, etc) may contain pointers to objects created on vectors.
//      This needs to be fixed. Could presize the vectors to the needed length so the reference
//      don't have a chance of getting invalidated.

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
#include "util.h"
#include "debug.h"

struct program_args_t pargs;
ABModel abmodel;
ParserObject parser;

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
    else if(strcmp(argv[arg_index], "--runtime-path") == 0) {
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
    std::cerr << argv[0] << std::endl;
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
    std::cerr << argv[0] << std::endl;
    exit(-1);
  }
}

struct test_struct {
  int a;
};
int main(int argc, char *argv[])
{
  parseArgs(argc, argv);

  // load and parse input model
  ABModel& model = parse_model(pargs.xml_model_path.c_str());

  #if VERBOSE_AST_GEN
  std::cout << model.to_string();
  #endif

  // write out ast as code to file
  std::string unique_id = std::to_string((int)time(NULL));
  std::string source_file_name = "./model-" + unique_id + ".cpp";
  std::string object_file_name = "./model-" + unique_id + ".o";
  std::ofstream file_out(source_file_name);
  std::string model_source = model.to_c_source(std::stol(unique_id));
  file_out << model_source << "\n";
  file_out.close();

  // write out init file
  model.write_init_file(std::stol(unique_id));

  // compile file
  std::cout << "Compiling output file \'" << source_file_name << "\'..." << std::endl;
  std::string object_command = "g++ --std=c++14 -c -g -o " + object_file_name + " -fPIC -I" + pargs.runtime_path + "cpu/include/ " + source_file_name;
  system(object_command.c_str());
  struct stat buf;
  if(stat(object_file_name.c_str(), &buf)) {
    std::cerr << "Failed to compile the generated source file.\n";
  }
  else {
    // link the file
    std::string lib_name = pargs.model_lib_name + "-" + unique_id + ".so ";
    std::string link_command = "g++ -g -shared -o " + lib_name + object_file_name;
    std::cout << link_command << std::endl;
    system(link_command.c_str());
    if(stat(("./" + lib_name).c_str(), &buf)) {
      std::cerr << "Failed to link the generated object file.\n";
      perror(strerror(errno));
    }

    // cleanup
    //remove(object_file_name.c_str());
  }

  // cleanup
  //remove(source_file_name.c_str());

  return 0;
}
