#ifndef DIMENSION_INCLUDED
#define DIMENSION_INCLUDED

#include <string>

#include "agent/position.h"

// Spatial relations for specifying coverage across a dimension
enum class PositionRelation {
    None,
    // Unary relation operators
    LT,
    GT,
    // Binary relational operators
    LTGT,
    GTLT,
};

struct dimension {
    enum class type_t {
        None,
        Absolute_Position,
        Relational_Position,
        All,
    };

    dimension(int width, const std::string& str);
    bool overlaps(const struct dimension& other) const;
    bool operator<(const struct dimension& other) const;\
    bool operator>(const struct dimension& other) const {
        return !((*this < other) || (*this == other));
    }
    bool operator==(const struct dimension& other) const {
        return (first_value == other.first_value) &&
        (second_value == other.second_value) &&
        (relation == other.relation) &&
        (position_type == other.position_type);
    }
    bool operator!=(const struct dimension& other) const { return !(*this == other); }
    bool is_binary() const {
        switch(relation) {
            case PositionRelation::LT:
            case PositionRelation::GT:
            return false;
            default:
            return true;
        }
    }
    dimension& operator=(const dimension& other) {
        first_value = other.first_value;
        second_value = other.second_value;
        relation = other.relation;
        position_type = other.position_type;
        return *this;
    }
    // only defined on Absolute_Position types
    dimension& operator-(int a);
    dimension& operator+(int a);

    dimension begin() const;
    dimension end(int dim_index) const;
    dimension next(const dimension& region_dim) const;

    // 'or equal to' is captured by incrementing or decrementing the value given in the XML model.
    // i.e. we are using exclusive relations. >=5,<10 stores 5 and 9
    int first_value = -1;
    int second_value = -1; // only present if relation is a binary one
    int width = -1;
    PositionRelation relation {PositionRelation::None};
    type_t position_type {dimension::type_t::None};
};

std::pair<PositionRelation, int> parse_relation(const std::string str);

#endif
