#include <iostream>
#include <string>

#include "agent/position.h"
#include "agent/dimension.h"
#include "abmodel.h"

extern ABModel abmodel;

dimension::dimension(int width, const std::string& str)
{
  this->width = width;

  if(str == "all") {
    position_type = type_t::All;
    return;
  }

  auto comma_pos = str.find(",");

  // Is a single absolute position
  if(comma_pos == std::string::npos) {
    std::pair<PositionRelation, int> r = parse_relation(str);
    relation = r.first;
    first_value = r.second;

    if(relation == PositionRelation::None) {
      position_type = type_t::Absolute_Position;
    }
    else {
      // TODO implement
      std::cerr << "Single bound relational dimensions not yet implemented: \'" << str << "\'.\n";
      exit(-1);
    }
  }
  // Contains a relational ordering
  else {
    std::string first_part = str.substr(0,comma_pos);
    std::string second_part = str.substr(comma_pos+1);
    std::pair<PositionRelation, int> r1 = parse_relation(first_part);
    std::pair<PositionRelation, int> r2 = parse_relation(second_part);

    if(r1.first == PositionRelation::None || r2.first == PositionRelation::None) {
      std::cerr << "Invalid dimension specification \'" << str << "\'.";
      std::cerr << "Giving two dimension bounds \'" << str << "\'implies they are both relational and not absolute.\n";
      exit(-1);
    }

    if(r1.first == PositionRelation::LT) {
      if(r2.first == PositionRelation::LT) {
        std::cerr << "Invalid relation specification\n";
        exit(-1);
      }
      else {
        relation = PositionRelation::LTGT;
      }
    }
    else {
      if(r2.first == PositionRelation::LT) {
        relation = PositionRelation::GTLT;
      }
      else {
        std::cerr << "Invalid relation specification\n";
        exit(-1);
      }
    }

    first_value = r1.second;
    second_value = r2.second;
    position_type = type_t::Relational_Position;
  }
}

// Computes if two dimension declarations overlap in space
// TODO fix possible edge cases when first or second value is on a dimension boundary
bool
dimension::overlaps(const dimension& other) const
{
  if(is_binary()) {
    if(other.is_binary()) {
      if(relation == PositionRelation::LTGT || other.relation == PositionRelation::LTGT) {
        return true;
      }
      else {
        return (second_value >= other.first_value) || (first_value <= other.second_value);
      }
    }
    else {
      return other < *this;
    }
  }
  else {
    if(other.is_binary()) {
      if(other.relation == PositionRelation::LTGT) {
        return true;
      }
      else {
        if(relation == PositionRelation::GT) {
          return first_value <= other.second_value;
        }
        else {
          return first_value >= other.first_value;
        }
      }
    }
    else {
      if((relation == PositionRelation::LT && other.relation == PositionRelation::LT)
        || (relation == PositionRelation::GT && other.relation == PositionRelation::GT)) {
        return true;
      }
      else {
        if(relation == PositionRelation::GT) {
          return first_value <= other.first_value;
        }
        else {
          return first_value >= other.first_value;
        }
      }
    }
  }
}

/*******************************************************************************
 * Operators for comparing and operating on agents.
 ******************************************************************************/

// sorts according to initial_agent position relation type.
// So we know which initial agents to process first
bool
dimension::operator<(const struct dimension& other) const
{
  switch(position_type) {
    case type_t::None:
      return false; // error
    case type_t::Absolute_Position:
      if(other.position_type == type_t::Absolute_Position) {
        if(overlaps(other)) {
          std::cerr << "Multiple initial agents declared with the same location." << std::endl;
          exit(-1);
        }
      }
      return true;
    case type_t::Relational_Position:
      switch(other.position_type) {
        case type_t::None:
          return false; // error
        case type_t::Absolute_Position:
          return false;
        case type_t::Relational_Position:
//          if(overlaps(other)) {
//            std::cerr << "Multiple initial agents declared with an overlapping location." << std::endl;
//            exit(-1);
//          }
          return true;
        case type_t::All:
          return true;
      }
    case type_t::All:
      if(other.position_type == type_t::All) {
        std::cerr << "Multiple initial agents declared with location of \'All\'." << std::endl;
        exit(-1);
      }
      return false;
  }

  return false;
}

dimension&
dimension::operator-(int value) {
  first_value -= value;
  return *this;
}

dimension&
dimension::operator+(int value) {
  first_value += value;
  return *this;
}

/*******************************************************************************
 * Dimension iterators for enumerating agents in a space.
 ******************************************************************************/
dimension
dimension::begin() const
{
  dimension result(*this);
  result.second_value = -1;
  result.position_type = dimension::type_t::Absolute_Position;
  result.relation = PositionRelation::None;
  switch(position_type) {
    case type_t::None:
      break;
    case type_t::Absolute_Position:
      break;
    case type_t::Relational_Position:
      switch(relation) {
        case PositionRelation::None:
          break;
        case PositionRelation::LT:
          result.first_value = 0;
          break;
        case PositionRelation::GT:
          result.first_value = first_value;
          break;
        case PositionRelation::LTGT:
          result.first_value = 0;
          break;
        case PositionRelation::GTLT:
          result.first_value = first_value;
          break;
      }
      break;
    case type_t::All:
      result.first_value = 0;
      break;
  }
  return result;
}

dimension
dimension::end(int dim_index) const
{
  int max_value = abmodel.dimension_sizes[dim_index];
  dimension result(*this);
  result.second_value = -1;
  result.position_type = dimension::type_t::Absolute_Position;
  result.relation = PositionRelation::None;
  switch(position_type) {
    case type_t::None:
      break;
    case type_t::Absolute_Position:
      result.first_value += 1;
      break;
    case type_t::Relational_Position:
      switch(relation) {
        case PositionRelation::None:
          break;
        case PositionRelation::LT:
          result.first_value = first_value + 1;
          break;
        case PositionRelation::GT:
          result.first_value = max_value;
          break;
        case PositionRelation::LTGT:
          result.first_value = max_value;
          break;
        case PositionRelation::GTLT:
          result.first_value = second_value + 1;
          break;
      }
      break;
    case type_t::All:
      result.first_value = max_value;
      break;
  }
  return result;
}

dimension
dimension::next(const dimension& region_dim) const
{
  dimension result(*this);
  result.position_type = dimension::type_t::Absolute_Position;
  result.relation = PositionRelation::None;
  switch(region_dim.position_type) {
    case type_t::None:
      break;
    case type_t::Absolute_Position:
      result.first_value += 1;
      break;
    case type_t::Relational_Position:
      switch(region_dim.relation) {
        case PositionRelation::None:
          break;
        case PositionRelation::LT:
          result.first_value = first_value + 1;
          break;
        case PositionRelation::GT:
          result.first_value = first_value + 1;
          break;
        case PositionRelation::LTGT:
          result.first_value = first_value + 1;
          if(result.first_value == region_dim.first_value + 1) {
            result.first_value = region_dim.second_value;
          }
          break;
        case PositionRelation::GTLT:
          result.first_value = first_value + 1;
          break;
      }
      break;
    case type_t::All:
      result.first_value = first_value + 1;
      break;
  }
  return result;
}

std::pair<PositionRelation, int>
parse_relation(const std::string str)
{
  if(str[0] == '<') {
    int value = -1;
    auto relation = PositionRelation::LT;
    if(str[1] == '=') {
      value = std::stoi(str.substr(2));
    }
    else {
      value = std::stoi(str.substr(1)) - 1;
    }
    return {relation, value};
  }
  else if(str[0] == '>') {
    int value = -1;
    auto relation = PositionRelation::GT;
    if(str[1] == '=') {
      value = std::stoi(str.substr(2));
    }
    else {
      value = std::stoi(str.substr(1)) + 1;
    }
    return {relation, value};
  }
  else {
    return {PositionRelation::None, std::stoi(str)};
  }
}
