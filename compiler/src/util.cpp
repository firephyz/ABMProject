#include "util.h"

#include <libxml2/libxml/parser.h>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>

xmlAttrPtr xmlGetAttribute(xmlNodePtr node, const char * attr_name) {
  xmlAttrPtr result = node->properties;
  while(result != NULL) {
    if(xmlStrcmp(result->name, (const xmlChar *)attr_name) == 0) {
      return result;
    }
    result = result->next;
  }
  return (xmlAttrPtr)NULL;
}

bool stobool(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    std::istringstream is(str);
    bool b;
    is >> std::boolalpha >> b;
    return b;
}

std::string xtos(const xmlChar* toString) { 
  return std::string((const char*)toString);
}

std::ostream&
util::error(xmlNodePtr node)
{
  std::cerr << "<" << xmlGetLineNo(node) << "> ";
  return std::cerr;
}

// removes all whitespace surrounding code (including end new-line if present)
std::string
util::indent(std::string code)
{
  // // trim input string
  // int start_index = 0;
  // std::find_if(code.begin(), code.end(),
  //   [&](const char& c){
  //     if ((c != ' ') && (c != '\t') && (c != '\n')) {
  //       return true;
  //     }
  //     else {
  //       ++start_index;
  //       return false;
  //     }
  // });
  // code = code.substr(start_index);
  // int end_index = code.length() - 1;
  // std::find_if(code.rbegin(), code.rend(),
  //   [&](const char& c){
  //     if ((c != ' ') && (c != '\t') && (c != '\n')) {
  //       return true;
  //     }
  //     else {
  //       --end_index;
  //       return false;
  //     }
  // });
  // code = code.substr(0, end_index + 1);

  // indent
  std::stringstream result;
  result << "\t";
  for(size_t index = 0; index < code.length(); ++index) {
    if(code[index] == '\n') {
      result << "\n\t";
    }
    else {
      result << code[index];
    }
  }

  return result.str();
}

std::string
util::indent(std::string str, int num)
{
  while(num != 0) {
    str = util::indent(str);
    --num;
  }
  return str;
}