#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <unordered_map>
#include <algorithm>
#include <tuple>

#include "dragon.hpp"
#include "dragon_cave.hpp"
#include "fantasy_dragon.hpp"
#include "magic_dragon.hpp"
#include <gcheck/redirectors.h>
#include <gcheck/tests.h>

std::string to_string(TreasureType t) {
    switch (t) {
    case Wisdom:
        return "Wisdom";
    case Potions:
        return "Potions";
    case Jewellery:
        return "Jewellery";
    default:
        return "ErrorType";
    }
}
std::string to_string(const Treasure& t) {
    return "[" + to_string(t.type) + ", " + t.name + "]";
}
bool operator==(const Treasure& l, const Treasure& r) {
    return l.type == r.type && l.name == r.name;
}

std::string to_string(FoodType t) {
    switch (t) {
    case People:
        return "People";
    case PeopleFood:
        return "PeopleFood";
    case Herbs:
        return "Herbs";
    default:
        return "ErrorType";
    }
}
std::string to_string(const Food& f) {
    return "[" + to_string(f.type) + ", " + f.name + "]";
}
bool operator==(const Food& l, const Food& r) {
    return l.type == r.type && l.name == r.name;
}

std::string to_construct(const Treasure& d) {
    return "(Treasure){" + to_string(d.type) + "," + gcheck::toConstruct(d.name) + "}";
}
std::string to_construct(const Food& d) {
    return "(Food){" + to_string(d.type) + "," + gcheck::toConstruct(d.name) + "}";
}

// BEGIN SERVER INTERNAL
template <typename T>
std::string GetParam(std::string name, size_t age, size_t size) {
    std::string type;
    if constexpr(std::is_same<T, MagicDragon>::value) {
        type = "MagicDragon";
    } else if constexpr(std::is_same<T, FantasyDragon>::value) {
        type = "FantasyDragon";
    }
    return type + "(std::string(\"" + name + "\"), " + std::to_string(age) + ", " + std::to_string(size) + ")";
}
std::string GetParam(Treasure t) {
    return "Treasure{" + to_string(t.type) + ", std::string(\"" + t.name + "\")}";
}
std::string GetParam(Food f) {
    return "Food{" + to_string(f.type) + ", std::string(\"" + f.name + "\")}";
}
std::string GetParam(std::string str) {
    return "std::string(\"" + str + "\")";
}
std::string GetParam(size_t s) {
    return "(size_t)" + std::to_string(s);
}
template<typename T>
std::string GetParam(const std::list<T>& data) {
    std::string out;
    for(auto& d : data) {
        out += GetParam(d) + ",";
    }
    out.pop_back();
    return "std::list({" + out + "})";
}
template<typename T>
std::string GetParam(const std::vector<T>& data) {
    std::string out;
    for(auto& d : data) {
        out += GetParam(d) + ",";
    }
    out.pop_back();
    return "std::vector({" + out + "})";
}
// END SERVER INTERNAL


std::vector<Treasure> CreateRandomTreasures(size_t count) {
    std::vector<Treasure> treasures;

    // Jewellery
    gcheck::Random j_names({"Ruby", "Gold bar", "Agate", "Amber", "Silver coins", "Jade stone", "Bag of pearls", "Topaz", "Sapphire dust"});
    // Wisdom
    gcheck::Random w_names({"Scroll of infinite wisdom", "Sun Tzu's Art of War", "Biographia literaria", "MAOL", "Gabriel's Wing", "Critique of pure reason", "The C++ standard", "The Symposium", "Thus Spoke Zarathustra"});
    // Potions
    gcheck::Random p_names({"Cough syrup", "Liquid luck", "Stoneskin potion", "Blemish blitzer", "Elixir of euphoria", "Garroting gas", "Essence of insanity", "Gin tonic", "Truth serum"});

    gcheck::Random rnd_type(std::vector<TreasureType>({Jewellery, Wisdom, Potions}));
    std::unordered_map<TreasureType, gcheck::Random<const char*>*> names({std::pair(Jewellery, &j_names), std::pair(Wisdom, &w_names), std::pair(Potions, &p_names)});

    for(size_t i = 0; i < count; i++) {
        TreasureType type = rnd_type.Next();
        treasures.push_back({type, std::string(names[type]->Next())});
    }

    return treasures;
}

std::vector<Food> CreateRandomFood(size_t count) {
    std::vector<Food> food;

    // PeopleFood
    gcheck::Random pf_names({"Tenderloin steak", "Carnivore pizza", "Beef chuck", "Pork ribs", "Cornered beef", "Flank steak", "Pork rack", "Half of a pork's ass", "Horse (salami)"});
    // People
    gcheck::Random p_names({"Raimo", "Petteri", "Joonas", "Konsta", "Jack Sparrow", "John Smith", "Random crusader", "Barack Obama", "The Pope"});
    // Herbs
    gcheck::Random h_names({"Arrowroot", "Bay leaves", "Hawthorn", "Hypericum perforatum", "Poppy seeds", "Magic mushrooms", "Salvia divinorum", "Salvia officinalis", "Peppermint root"});

    gcheck::Random rnd_type(std::vector<FoodType>({People, PeopleFood, Herbs}));
    std::unordered_map<FoodType, gcheck::Random<const char*>*> names({std::pair(People, &p_names), std::pair(PeopleFood, &pf_names), std::pair(Herbs, &h_names)});

    for(size_t i = 0; i < count; i++) {
        FoodType type = rnd_type.Next();
        food.push_back({type, std::string(names[type]->Next())});
    }

    return food;
}

auto rnd_treasures = gcheck::RandomSizeContainer<std::list>(10, 30, CreateRandomTreasures(50));
auto rnd_foods = gcheck::RandomSizeContainer<std::list>(10, 30, CreateRandomFood(50));
gcheck::Random<std::string> rnd_name(std::vector<std::string>{"Puff", "Drogon", "Bahamut", "Haku"});
gcheck::Random<size_t> rnd_age(0, 10000);
gcheck::Random<size_t> rnd_size(5, 30);
auto rnd_names = gcheck::RandomSizeContainer(3, 3, std::vector<std::string>{"Puff", "Drogon", "Bahamut", "Haku"});
auto rnd_ages = gcheck::RandomSizeContainer(3, 3, (size_t)0, (size_t)10000);
auto rnd_sizes = gcheck::RandomSizeContainer(3, 3, (size_t)5, (size_t)30);


TEST(Dragons, constructors, 1) {

    size_t puff_age = rnd_age.Next();
    size_t bahamut_age = rnd_age.Next();
    size_t puff_size = rnd_size.Next();
    size_t bahamut_size = rnd_size.Next();
    MagicDragon* mdragon = new MagicDragon("Puff", puff_age, puff_size);
    FantasyDragon* fdragon = new FantasyDragon("Bahamut", bahamut_age, bahamut_size);

    EXPECT_EQ("Puff", mdragon->GetName());
    EXPECT_EQ(puff_age, mdragon->GetAge());
    EXPECT_EQ(puff_size, mdragon->GetSize());

    EXPECT_EQ("Bahamut", fdragon->GetName());
    EXPECT_EQ(bahamut_age, fdragon->GetAge());
    EXPECT_EQ(bahamut_size, fdragon->GetSize());

    delete fdragon;
    delete mdragon;
}

// BEGIN SERVER INTERNAL
template<typename T>
std::string DoHoard(std::list<Treasure>& treasures, std::vector<Treasure>* treas = nullptr) {
    std::stringstream out;
    std::string prefix;
    TreasureType type;
    if constexpr(std::is_same<T, MagicDragon>::value) {
        prefix = "Magic dragon received: ";
        type = Potions;
    } else if constexpr(std::is_same<T, FantasyDragon>::value) {
        prefix = "Fantasy dragon received: ";
        type = Jewellery;
    }
    for (auto it = treasures.begin(); it != treasures.end();) {
        if (it->type == type) {
            out << prefix << it->name << std::endl;
            if(treas)
                treas->push_back(*it);
            it = treasures.erase(it);
        } else
            it++;
    }

    return out.str();
}
template<typename T>
gcheck::Result<std::string> CHoardOutput(std::list<Treasure> treasures) {
    gcheck::Result<std::string> res;
    //res.SetInputParams(GetParam(treasures));
    res.output = DoHoard<T>(treasures);
    return res;
}
template<typename T>
gcheck::Result<std::vector<Treasure>> CHoardList(std::list<Treasure> treasures) {
    gcheck::Result<std::vector<Treasure>> res;
    //res.SetInputParams(GetParam(treasures));
    DoHoard<T>(treasures);
    res.output = std::vector<Treasure>(treasures.begin(), treasures.end());
    //res.SetOutputParams(GetParam(res.output));
    return res;
}
template<typename T>
gcheck::Result<std::vector<Treasure>> CHoardDragons(std::list<Treasure> treasures) {
    gcheck::Result<std::vector<Treasure>> res;
    //res.SetInputParams(GetParam(treasures));
    DoHoard<T>(treasures, &res.output);
    //res.SetOutputParams(GetParam(res.output));
    return res;
}
// END SERVER INTERNAL
template<typename T>
std::string THoardOutput(std::list<Treasure> treasures) {
    gcheck::StdoutCapturer tout;
    T("", 0, 0).Hoard(treasures);
    return tout.str();
}
template<typename T>
std::vector<Treasure> THoardList(std::list<Treasure> treasures) {
    T("", 0, 0).Hoard(treasures);
    return std::vector<Treasure>(treasures.begin(), treasures.end());
}
template<typename T>
std::vector<Treasure> THoardDragons(std::list<Treasure> treasures) {
    T dragon("", 0, 0);
    dragon.Hoard(treasures);
    auto& treas = dragon.GetTreasures();
    return std::vector<Treasure>(treas.begin(), treas.end());
}

TEST(MagicDragon, Hoard_printed_output, 1, "Dragons.constructors") {
    CompareWithCallable(3, CHoardOutput<MagicDragon>, THoardOutput<MagicDragon>, rnd_treasures);
}
TEST(FantasyDragon, Hoard_printed_output, 1, "Dragons.constructors") {
    CompareWithCallable(3, CHoardOutput<FantasyDragon>, THoardOutput<FantasyDragon>, rnd_treasures);
}

TEST(MagicDragon, Hoard_treasures_left_in_list, 1, "Dragons.constructors") {
    CompareWithCallable(3, CHoardList<MagicDragon>, THoardList<MagicDragon>, rnd_treasures);
}
TEST(FantasyDragon, Hoard_treasures_left_in_list, 1, "Dragons.constructors") {
    CompareWithCallable(3, CHoardList<FantasyDragon>, THoardList<FantasyDragon>, rnd_treasures);
}

TEST(MagicDragon, Hoard_dragons_treasures, 1, "Dragons.constructors") {
    CompareWithCallable(3, CHoardDragons<MagicDragon>, THoardDragons<MagicDragon>, rnd_treasures);
}
TEST(FantasyDragon, Hoard_dragons_treasures, 1, "Dragons.constructors") {
    CompareWithCallable(3, CHoardDragons<FantasyDragon>, THoardDragons<FantasyDragon>, rnd_treasures);
}

// BEGIN SERVER INTERNAL
template<typename T>
std::string DoEat(std::list<Food>& foods, size_t* p_size = nullptr) {
    size_t size = 0;
    std::stringstream out;
    std::string prefix;
    FoodType type, type2;
    if constexpr(std::is_same<T, MagicDragon>::value) {
        prefix = "Magic dragon ate: ";
        type = Herbs;
        type2 = Herbs;
    } else if constexpr(std::is_same<T, FantasyDragon>::value) {
        prefix = "Fantasy dragon ate: ";
        type = People;
        type2 = PeopleFood;
    }
    for (auto it = foods.begin(); it != foods.end();) {
        if (it->type == type || it->type == type2) {
            out << prefix << it->name << std::endl;
            it = foods.erase(it);
            size++;
        } else
            it++;
    }
    if(p_size)
        *p_size = size;
    return out.str();
}
template<typename T>
gcheck::Result<std::string> CEatOutput(std::list<Food> foods) {
    gcheck::Result<std::string> res;
    //res.SetInputParams(GetParam(foods));
    res.output = DoEat<T>(foods);
    return res;
}
template<typename T>
gcheck::Result<std::vector<Food>> CEatList(std::list<Food> foods) {
    gcheck::Result<std::vector<Food>> res;
    //res.SetInputParams(GetParam(foods));
    DoEat<T>(foods);
    res.output = std::vector<Food>(foods.begin(), foods.end());
    //res.SetOutputParams(GetParam(res.output));
    return res;
}
template<typename T>
gcheck::Result<size_t> CEatSize(std::list<Food> foods) {
    gcheck::Result<size_t> res(0);
    //res.SetInputParams(GetParam(foods));
    DoEat<T>(foods, &res.output);
    //res.SetOutputParams(GetParam(res.output));
    return res;
}
// END SERVER INTERNAL
template<typename T>
std::string TEatOutput(std::list<Food> foods) {
    gcheck::StdoutCapturer tout;
    T("", 0, 0).Eat(foods);
    return tout.str();
}
template<typename T>
std::vector<Food> TEatList(std::list<Food> foods) {
    T("", 0, 0).Eat(foods);
    return std::vector<Food>(foods.begin(), foods.end());
}
template<typename T>
size_t TEatSize(std::list<Food> foods) {
    T dragon("", 0, 0);
    dragon.Eat(foods);
    return dragon.GetSize();
}

TEST(MagicDragon, Eat_printed_output, 1, "Dragons.constructors") {
    CompareWithCallable(3, CEatOutput<MagicDragon>, TEatOutput<MagicDragon>, rnd_foods);
}
TEST(FantasyDragon, Eat_printed_output, 1, "Dragons.constructors") {
    CompareWithCallable(3, CEatOutput<FantasyDragon>, TEatOutput<FantasyDragon>, rnd_foods);
}

TEST(MagicDragon, Eat_foods_left_in_list, 1, "Dragons.constructors") {
    CompareWithCallable(3, CEatList<MagicDragon>, TEatList<MagicDragon>, rnd_foods);
}
TEST(FantasyDragon, Eat_foods_left_in_list, 1, "Dragons.constructors") {
    CompareWithCallable(3, CEatList<FantasyDragon>, TEatList<FantasyDragon>, rnd_foods);
}

TEST(MagicDragon, Eat_dragon_size, 1, "Dragons.constructors") {
    CompareWithCallable(3, CEatSize<MagicDragon>, TEatSize<MagicDragon>, rnd_foods);
}
TEST(FantasyDragon, Eat_dragon_size, 1, "Dragons.constructors") {
    CompareWithCallable(3, CEatSize<FantasyDragon>, TEatSize<FantasyDragon>, rnd_foods);
}


// BEGIN SERVER INTERNAL
template<typename T>
gcheck::Result<std::string> CPrint(std::list<Treasure> treasures, std::string name, size_t age, size_t size) {
    gcheck::Result<std::string> res;
    //res.SetInputParams("std::make_tuple<std::list<Treasure>, std::string, size_t, size_t>(" + GetParam(treasures) + "," + "\"" + name + "\"," + std::to_string(age) + "," + std::to_string(size) + ")");

    std::vector<Treasure> treas;
    DoHoard<T>(treasures, &treas);

    std::stringstream os;
    os << "Dragon named: " << name << ", age: " << age << ", size: " << size << std::endl;
    os << "Treasures:" << std::endl;
    for (auto it = treas.begin(); it != treas.end(); it++)
        os << it->name << std::endl;

    res.output = os.str();
    return res;
}
// END SERVER INTERNAL
template<typename T>
std::string TPrint(std::list<Treasure> treasures, std::string name, size_t age, size_t size) {
    T dragon(name, age, size);
    dragon.Hoard(treasures);
    std::stringstream os;
    os << dragon;
    return os.str();
}

TEST(Dragon, ostream_write_operator, 1, "Dragons.constructors") {
    CompareWithCallable(1, CPrint<MagicDragon>, TPrint<MagicDragon>, rnd_treasures, rnd_name, rnd_age, rnd_size);
    CompareWithCallable(1, CPrint<FantasyDragon>, TPrint<FantasyDragon>, rnd_treasures, rnd_name, rnd_age, rnd_size);
}

TEST(DragonCave, Accommodate, 2, "Dragons.constructors") {

    // Random treasures
    std::list<Treasure> treasures = rnd_treasures.Next();
    // Random food
    std::list<Food> food = rnd_foods.Next();

    MagicDragon* mdragon = new MagicDragon("Puff", rnd_age.Next(), rnd_size.Next());
    FantasyDragon* fdragon = new FantasyDragon("Bahamut", rnd_age.Next(), rnd_size.Next());

    DragonCave cave;

    cave.Accommodate(mdragon);
    cave.Accommodate(fdragon);

    auto it = cave.GetDragons().begin();

    EXPECT_EQ(mdragon, *it) << "First item in GetDragons";
    it++;

    EXPECT_EQ(fdragon, *it) << "Second item in GetDragons";
}

TEST(DragonCave, Evict, 3, "Accommodate") {
    // Random treasures
    std::list<Treasure> treasures = rnd_treasures.Next();
    // Random food
    std::list<Food> food = rnd_foods.Next();

    MagicDragon* mdragon = new MagicDragon("Puff", rnd_age.Next(), rnd_size.Next());
    FantasyDragon* fdragon = new FantasyDragon("Bahamut", rnd_age.Next(), rnd_size.Next());
    MagicDragon* mdragon2 = new MagicDragon("Drogon", rnd_age.Next(), rnd_size.Next());

    DragonCave cave;

    cave.Accommodate(mdragon);
    cave.Accommodate(fdragon);
    cave.Accommodate(mdragon2);

    cave.Evict("Puff");

    auto it = cave.GetDragons().begin();

    EXPECT_EQ(fdragon, *it) << "First item in GetDragons";
    it++;

    EXPECT_EQ(mdragon2, *it) << "Second item in GetDragons";

    delete mdragon;
}

// BEGIN SERVER INTERNAL
gcheck::Result<std::string> CCavePrint(const std::list<Treasure>& treasures, std::vector<std::string> names, std::vector<size_t> ages, std::vector<size_t> sizes) {
    gcheck::Result<std::string> res;
    //res.SetInputParams("std::make_tuple<std::list<Treasure>, std::vector<std::string>, std::vector<size_t>, std::vector<size_t>>(" + GetParam(treasures) + "," + GetParam(names) + "," + GetParam(ages) + "," + GetParam(sizes)+")");

    std::vector<Treasure> treas, treas2, treas3;
    std::stringstream os;
    os << "DragonCave dwellers:\n\n";
    os << CPrint<MagicDragon>(treasures, names[0], ages[0], sizes[0]).output << std::endl;
    os << CPrint<FantasyDragon>(treasures, names[1], ages[1], sizes[1]).output;

    res.output = os.str();
    return res;
}
// END SERVER INTERNAL
std::string TCavePrint(std::list<Treasure> treasures, std::vector<std::string> names, std::vector<size_t> ages, std::vector<size_t> sizes) {
    DragonCave cave;
    MagicDragon* mdragon = new MagicDragon(names[0], ages[0], sizes[0]);
    FantasyDragon* fdragon = new FantasyDragon(names[1], ages[1], sizes[1]);
    mdragon->Hoard(treasures);
    fdragon->Hoard(treasures);
    cave.Accommodate(mdragon);
    cave.Accommodate(fdragon);
    std::stringstream os;
    os << cave;
    return os.str();
}

TEST(DragonCave, stream_write_operator, 3, "Accommodate Dragon.ostream_write_operator") {
    CompareWithCallable(1, CCavePrint, TCavePrint, rnd_treasures, rnd_names, rnd_ages, rnd_sizes);
}
namespace {
    template<class>
    struct sfinae_bool {};
    template<class T>
    struct sfinae_true : sfinae_bool<T>, std::true_type{};
    template<class T>
    struct sfinae_false : sfinae_bool<T>, std::false_type{};
}

namespace detail {
    template<class T>
    static auto has_copy_constructor(int) -> sfinae_true<decltype(T(std::declval<T>()))>;
    template<class T>
    static auto has_copy_constructor(long) -> sfinae_false<T>;

    template<class T>
    static auto has_copy_assignment(int) -> sfinae_true<decltype(std::declval<T>() = std::declval<T>())>;
    template<class T>
    static auto has_copy_assignment(long) -> sfinae_false<T>;
} // detail

template<class T>
struct has_copy_constructor_t : decltype(detail::has_copy_constructor<T>(0)){};
template<class T>
struct has_copy_assignment_t : decltype(detail::has_copy_assignment<T>(0)){};

TEST(DragonCave, copy_assignment_constructor, 2) {
    bool has_copy_constructor = has_copy_constructor_t<DragonCave>();
    bool has_copy_assignment = has_copy_assignment_t<DragonCave>();
    EXPECT_FALSE(has_copy_constructor) << "You might have to run 'make clean' (ctrl + shift + b -> Clean test Dragon)\nfor any changes to this to take effect when running locally";
    EXPECT_FALSE(has_copy_assignment) << "You might have to run 'make clean' (ctrl + shift + b -> Clean test Dragon)\nfor any changes to this to take effect when running locally";
}