#include "creature.hpp"

Creature::Creature(const std::string& name, const std::string& type, int hitpoints)
    : name_(name), type_(type), hitpoints_(hitpoints) { }

const std::string& Creature::GetName() const { return name_; }
const std::string& Creature::GetType() const { return type_; }
int Creature::GetHitPoints() const { return hitpoints_; }

// Implement the '<<' operator here
/*BEGIN SOLUTION*/
std::ostream &operator<<(std::ostream& out, const Creature& a) {
    out << a.GetType() << " " << a.GetName() << " with " << a.GetHitPoints() << " HP";
    return out;
}
/*END SOLUTION*/
