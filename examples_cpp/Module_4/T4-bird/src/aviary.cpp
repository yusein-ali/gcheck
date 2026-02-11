#include "aviary.hpp"

/*BEGIN SOLUTION*/
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include "bird.hpp"

void Aviary::Add(Bird* b) {
    birds_.push_back(b);
}

Aviary::~Aviary() {
    for (auto i : birds_)
        delete i;
}

void Aviary::SpeakAll(std::ostream& os) const {
    for(auto b : birds_)
        b->Speak(os);
}

size_t Aviary::Size() const {
    return birds_.size();
}
/*END SOLUTION*/