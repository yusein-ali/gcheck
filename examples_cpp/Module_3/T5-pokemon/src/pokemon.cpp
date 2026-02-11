#include "pokemon.hpp"

/*BEGIN SOLUTION*/
#include <algorithm>

PokemonCollection::PokemonCollection(PokemonCollection first, PokemonCollection second) {
    std::copy(first.pokemons_.begin(), first.pokemons_.end(), std::back_inserter(pokemons_));
    std::copy(second.pokemons_.begin(), second.pokemons_.end(), std::back_inserter(pokemons_));
    
    SortByName();
    pokemons_.unique();
}

void PokemonCollection::Add(const std::string& name, size_t id) {
    pokemons_.push_back({name, id});
}

bool PokemonCollection::Remove(const std::string& name, size_t id) {
    std::pair<std::string, size_t> card(name, id);
    auto ret_it = std::find(pokemons_.begin(), pokemons_.end(), card);
  
    if (ret_it != pokemons_.end()) {
        pokemons_.erase(ret_it);
        return true;
    }
    return false;
}

void PokemonCollection::Print() const {
    for (auto it = pokemons_.begin(); it != pokemons_.end(); it++)
        std::cout << "id: " << it->second << ", name: " << it->first << std::endl;
}

bool CompareById(const std::pair<std::string, size_t>& c1, const std::pair<std::string, size_t>& c2) {
    return (c1.second != c2.second) ? (c1.second < c2.second) : (c1.first < c2.first);
}

void PokemonCollection::SortByName() {
    pokemons_.sort();
}
void PokemonCollection::SortById() {
    pokemons_.sort(CompareById);
}
/*END SOLUTION*/
