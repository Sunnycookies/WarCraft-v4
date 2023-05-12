#include <iostream>
#include <map>
#include <iomanip>

using namespace std;

const int nWeapons = 3;
const int nWarriors = 5;

const int win_award = 8;

int nCities;

int hour, minute, time_limit;

int init_elements;

int loyalty_decrease, arrow_attack;

typedef int Result;

const Result Nothing = 0;
const Result Passively = 1;
const Result Actively = 1 << 1;
const Result Lose = 1 << 2;
const Result Tie = 1 << 3;
const Result Win = 1 << 4;

inline void print_time() { cout << setw(3) << setfill('0') << right << hour << ':' << setw(2) << setfill('0') << right << minute << ' '; }

inline bool time_not_valid() { return 60 * hour + minute > time_limit; }

enum weapon_type
{
    sword,
    bomb,
    arrow
};
enum warrior_type
{
    dragon,
    ninja,
    iceman,
    lion,
    wolf
};
enum city_type
{
    red,
    blue,
    neutral
};

class Weapon;
class Warrior;
class City;
class Headquarter;

class Weapon
{
protected:
    int attack_value;

    weapon_type type;

public:
    Weapon(const int &value, const weapon_type &_type) : attack_value(value), type(_type) {}

    virtual ~Weapon() {}

    virtual void print_self() = 0;

    virtual void utilize() = 0;

    virtual bool is_used_up() = 0;

    friend class Warrior;
};
class Sword : public Weapon
{
public:
    Sword(const int &value) : Weapon(value, sword) {}

    void utilize() override { attack_value *= 0.8; }

    bool is_used_up() override { return !attack_value; }

    void print_self() override { cout << "sword(" << attack_value << ')'; }
};
class Bomb : public Weapon
{
private:
    bool _is_used_up = 0;

public:
    Bomb() : Weapon(INT32_MAX, bomb) {}

    void utilize() override { _is_used_up = 1; }

    bool is_used_up() override { return _is_used_up; }

    void print_self() override { cout << "bomb"; }
};
class Arrow : public Weapon
{
private:
    int left_num = 3;

public:
    Arrow() : Weapon(arrow_attack, arrow) {}

    void utilize() override { --left_num; }

    bool is_used_up() override { return !left_num; }

    void print_self() override { cout << "arrow(" << left_num << ')'; }
};

class Warrior
{
protected:
    static string warrior_name[nWarriors];

    Headquarter *pHeadquarter;

    City *pCity;

    warrior_type type;

    int id, elements, force;

    Weapon *pWeapons[nWeapons];

    bool move = 0;

public:
    static int elements_value[nWarriors], force_value[nWarriors];

    Warrior(Headquarter *headquarter, const int &_id, const warrior_type &_type);

    virtual ~Warrior();

    void get_weapon(const weapon_type &_type)
    {
        switch (_type)
        {
        case sword:
            if (force / 5)
            {
                pWeapons[sword] = new Sword(force / 5);
            }
            break;

        case bomb:
            pWeapons[bomb] = new Bomb;
            break;

        case arrow:
            pWeapons[arrow] = new Arrow;
            break;
        }
    }

    void try_to_destroy_weapon(const weapon_type &weapon)
    {
        if (!pWeapons[weapon] || !pWeapons[weapon]->is_used_up())
        {
            return;
        }
        delete pWeapons[weapon];
        pWeapons[weapon] = nullptr;
    }

    void hurted(const int &value) { elements = max(0, elements - value); }

    void gain_elements(const int &value) { elements += value; }

    int actively_attack_value()
    {
        int attack_value = force;
        if (pWeapons[sword])
        {
            attack_value += pWeapons[sword]->attack_value;
        }
        return attack_value;
    }

    virtual int passively_attack_value()
    {
        int attack_value = force / 2;
        if (pWeapons[sword])
        {
            attack_value += pWeapons[sword]->attack_value;
        }
        return attack_value;
    }

    bool is_dead() { return !elements; }

    Result preview(Warrior *enemy, const Result &mode)
    {
        if (mode == Actively)
        {
            if (actively_attack_value() >= enemy->elements)
            {
                return mode + Win;
            }
            if (enemy->passively_attack_value() >= elements)
            {
                return mode + Lose;
            }
            return mode + Tie;
        }
        if (elements <= enemy->actively_attack_value())
        {
            return mode + Lose;
        }
        if (passively_attack_value() >= enemy->elements)
        {
            return mode + Win;
        }
        return mode + Tie;
    }

    void report_shot(Warrior *enemy)
    {
        print_time(), print_self();
        cout << " shot";
        if (enemy->is_dead())
        {
            cout << " and killed ";
            enemy->print_self();
        }
        cout << endl;
    }

    void report_weapons()
    {
        print_time(), print_self();
        bool has_weapons = 0, first_weapon = 1;
        cout << " has ";
        for (int i = 0; i < nWeapons; ++i)
        {
            has_weapons |= pWeapons[i] != nullptr;
        }
        if (!has_weapons)
        {
            cout << "no weapon" << endl;
            return;
        }
        for (int i = nWeapons - 1; i >= 0; --i)
        {
            if (!pWeapons[i])
            {
                continue;
            }
            if (first_weapon)
            {
                first_weapon = 0;
            }
            else
            {
                cout << ',';
            }
            pWeapons[i]->print_self();
        }
        cout << endl;
    }

    virtual void pick_weapon();

    void print_self();

    bool try_to_shot(Warrior *enemy);

    bool try_to_use_bomb(Warrior *enemy, const city_type &active);

    void actively_attack(Warrior *enemy);

    virtual void passively_attack(Warrior *enemy);

    virtual void after_attack(Warrior *enemy, const Result &result) {}

    virtual void refresh_record_elements() {}

    void send_elements_to_headquarter(const int &value);

    void report_death();

    bool march();

    virtual void during_march() {}

    void report_arrival();

    bool is_at_target_city();

    bool is_at_home();

    virtual bool should_escape() { return 0; }

    int position();

    friend class City;

    friend class Headquarter;
};
class Dragon : public Warrior
{
private:
    double morale;

public:
    Dragon(Headquarter *pHeadquarter, const int &id, const double &_morale) : Warrior(pHeadquarter, id, dragon), morale(_morale)
    {
        get_weapon(weapon_type(id % nWeapons));
        cout << "Its morale is " << fixed << setprecision(2) << morale << endl;
    }

    void after_attack(Warrior *enemy, const Result &result) override;

    void pick_weapon() override {}
};
class Ninja : public Warrior
{
public:
    Ninja(Headquarter *pHeadquarter, const int &id) : Warrior(pHeadquarter, id, ninja)
    {
        get_weapon(weapon_type(id % nWeapons));
        get_weapon(weapon_type((id + 1) % nWeapons));
    }

    void passively_attack(Warrior *enemy) override {}

    int passively_attack_value() override { return 0; }

    void pick_weapon() override {}
};
class Iceman : public Warrior
{
private:
    int count_steps = 0;

public:
    Iceman(Headquarter *pHeadquarter, const int &id) : Warrior(pHeadquarter, id, iceman)
    {
        get_weapon(weapon_type(id % nWeapons));
    }

    void during_march() override
    {
        ++count_steps;
        if ((count_steps >> 1) & 1)
        {
            count_steps >>= 2;
            elements = max(elements - 9, 1);
            force += 20;
        }
    }

    void pick_weapon() override {}
};
class Lion : public Warrior
{
private:
    int loyalty, record_elements;

public:
    Lion(Headquarter *pHeadquarter, const int &id, const int &_loyalty) : Warrior(pHeadquarter, id, lion), loyalty(_loyalty)
    {
        record_elements = elements;
        cout << "Its loyalty is " << loyalty << endl;
    }

    void refresh_record_elements() override { record_elements = elements; }

    void after_attack(Warrior *enemy, const Result &result) override
    {
        if (enemy->position() != position())
        {
            return;
        }
        if (result < Tie)
        {
            enemy->gain_elements(record_elements);
            return;
        }
        refresh_record_elements();
        if (result < Win)
        {
            loyalty -= loyalty_decrease;
        }
    }

    bool should_escape() override { return loyalty <= 0 && !is_at_target_city(); }

    void pick_weapon() override {}
};
class Wolf : public Warrior
{
public:
    Wolf(Headquarter *pHeadquarter, const int &id) : Warrior(pHeadquarter, id, wolf) {}
};

class City
{
private:
    static int count;

    int index, elements = 0;

    Warrior *pWarriors[2];

    city_type flag = neutral, curr_win = neutral, prev_win = neutral;

    Result state = Nothing;

    bool blue_report_shot = 0;

    map<weapon_type, Weapon *> weapon_pool;

public:
    City()
    {
        index = ++count;
        for (int i = 0; i < 2; ++i)
        {
            pWarriors[i] = nullptr;
        }
    }

    ~City()
    {
        --count;
        for (int i = 0; i < 2; ++i)
        {
            if (pWarriors[i])
            {
                delete pWarriors[i];
            }
        }
    }

    city_type active_attacker_type() { return flag == neutral ? city_type((index & 1) ^ 1) : flag; }

    void print_self() { cout << "city " << index; }

    void lion_escape()
    {
        for (int i = 0; i < 2; ++i)
        {
            if (!pWarriors[i] || pWarriors[i]->type != lion)
            {
                continue;
            }
            if (pWarriors[i]->should_escape())
            {
                print_time(), pWarriors[i]->print_self();
                cout << " ran away" << endl;
                delete pWarriors[i];
            }
        }
    }

    void produce_elements() { elements += 10; }

    void warrior_earn_elements()
    {
        if (!elements)
        {
            return;
        }
        if (pWarriors[red] && !pWarriors[blue])
        {
            pWarriors[red]->send_elements_to_headquarter(elements);
            elements = 0;
            return;
        }
        if (pWarriors[blue] && !pWarriors[red])
        {
            pWarriors[blue]->send_elements_to_headquarter(elements);
            elements = 0;
            return;
        }
    }

    void warrior_arrive()
    {
        for (int i = 0; i < 2; ++i)
        {
            if (!pWarriors[i])
            {
                continue;
            }
            if (pWarriors[i]->move)
            {
                pWarriors[i]->report_arrival();
            }
        }
    }

    void warrior_shot()
    {
        City *Next = index == nCities + 1 ? nullptr : this + 1;
        if (pWarriors[red] && Next != nullptr && Next->pWarriors[blue])
        {
            Warrior *Red = pWarriors[red], *Blue = Next->pWarriors[blue];
            if (Red->try_to_shot(Blue))
            {
                Blue->refresh_record_elements();
                Red->report_shot(Blue);
            }
            if (Blue->try_to_shot(Red))
            {
                Red->refresh_record_elements();
                Next->blue_report_shot = 1;
            }
        }
        if (blue_report_shot)
        {
            Warrior *Red = (this - 1)->pWarriors[red];
            pWarriors[blue]->report_shot(Red);
            blue_report_shot = 0;
        }
    }

    void warrior_explode()
    {
        if (!pWarriors[red] || !pWarriors[blue])
        {
            return;
        }
        city_type active = active_attacker_type();
        bool explode = pWarriors[red]->try_to_use_bomb(pWarriors[blue], active) | pWarriors[blue]->try_to_use_bomb(pWarriors[red], active);
        if (explode)
        {
            delete pWarriors[red];
            delete pWarriors[blue];
        }
    }

    void warrior_fight()
    {
        if (!pWarriors[red] || !pWarriors[blue])
        {
            raise_flag();
            return;
        }
        if (pWarriors[red]->is_dead() || pWarriors[blue]->is_dead())
        {
            after_fight();
            return;
        }
        city_type active_type = active_attacker_type();
        Warrior *active = pWarriors[active_type], *passive = pWarriors[active_type ^ 1];
        active->actively_attack(passive);
        if (passive->is_dead())
        {
            after_fight();
            return;
        }
        passive->passively_attack(active);
        after_fight();
    }

    void after_fight()
    {
        city_type active_type = active_attacker_type();
        Warrior *active = pWarriors[active_type], *passive = pWarriors[active_type ^ 1];
        if (active->is_dead() && passive->is_dead())
        {
        }
        else if (active->is_dead())
        {
            state = Actively + Lose;
            curr_win = city_type(active_type ^ 1);
            passive->after_attack(active, Passively + Win);
            active->after_attack(passive, Actively + Lose);
        }
        else if (passive->is_dead())
        {
            state = Actively + Win;
            curr_win = active_type;
            active->after_attack(passive, Actively + Win);
            passive->after_attack(active, Passively + Lose);
        }
        else
        {
            state = Tie;
            active->after_attack(passive, Actively + Tie);
            passive->after_attack(active, Passively + Tie);
        }
        raise_flag();
    }

    void raise_flag();

    void reset_record()
    {
        if (state == Nothing)
        {
            return;
        }
        prev_win = curr_win, curr_win = neutral, state = Nothing;
    }

    friend class Warrior;

    friend class Headquarter;
};

class Headquarter
{
private:
    static string headquarter_name[2];

    static warrior_type produce_order[2][nWarriors];

    City *pCity;

    city_type type;

    warrior_type *order;

    int elements = init_elements, warriors = 0, index = 0, elements_buffer = 0;

    map<int, Warrior *> pWarriors;

public:
    Headquarter(City *city, const city_type &_type) : pCity(city), type(_type)
    {
        pCity->flag = type;
        order = produce_order[type];
    }

    void produce()
    {
        warrior_type _type = order[index];
        int cost = Warrior::elements_value[_type];
        if (elements < cost)
        {
            return;
        }
        elements -= cost, ++warriors;
        index = ++index % nWarriors;
        switch (_type)
        {
        case dragon:
            pWarriors.emplace(warriors, new Dragon(this, warriors, 1.0 * elements / cost));
            break;

        case ninja:
            pWarriors.emplace(warriors, new Ninja(this, warriors));
            break;
        case iceman:
            pWarriors.emplace(warriors, new Iceman(this, warriors));
            break;
        case lion:
            pWarriors.emplace(warriors, new Lion(this, warriors, elements));
            break;
        case wolf:
            pWarriors.emplace(warriors, new Wolf(this, warriors));
        }
    }

    void report_elements()
    {
        print_time();
        cout << elements << " elements in " << headquarter_name[type] << " headquarter" << endl;
    }

    void report_weapons()
    {
        if (type == blue)
        {
            for (auto p = pWarriors.begin(); p != pWarriors.end(); ++p)
            {
                p->second->report_weapons();
            }
            return;
        }
        for (auto p = pWarriors.rbegin(); p != pWarriors.rend(); ++p)
        {
            p->second->report_weapons();
        }
    }

    bool march_and_if_conquer()
    {
        bool conquer = 0;
        for (auto p = pWarriors.begin(); p != pWarriors.end(); ++p)
        {
            conquer |= p->second->march();
        }
        return conquer;
    }

    void award_elements()
    {
        for (auto p = pWarriors.begin(); p != pWarriors.end(); ++p)
        {
            if (elements < win_award)
            {
                break;
            }
            if (p->second->pCity->curr_win == type)
            {
                p->second->gain_elements(win_award), elements -= win_award;
                p->second->refresh_record_elements();
            }
        }
        elements += elements_buffer, elements_buffer = 0;
    }

    void report_conquer()
    {
        print_time();
        cout << headquarter_name[type ^ 1] << " headquarter was taken" << endl;
    }

    friend class Warrior;

    friend class City;
};

string Warrior::warrior_name[nWarriors] = {"dragon", "ninja", "iceman", "lion", "wolf"};

int Warrior::elements_value[nWarriors], Warrior::force_value[nWarriors];

int City::count = -1;

string Headquarter::headquarter_name[2] = {"red", "blue"};

warrior_type Headquarter::produce_order[2][nWarriors] = {{iceman, lion, wolf, ninja, dragon}, {lion, dragon, ninja, iceman, wolf}};

Warrior::Warrior(Headquarter *headquarter, const int &_id, const warrior_type &_type) : pHeadquarter(headquarter), type(_type), id(_id)
{
    elements = elements_value[type], force = force_value[type];
    pCity = pHeadquarter->pCity;
    pCity->pWarriors[pHeadquarter->type] = this;
    for (int i = 0; i < nWeapons; ++i)
    {
        pWeapons[i] = nullptr;
    }
    print_time(), print_self();
    cout << " born" << endl;
}

Warrior::~Warrior()
{
    for (int i = 0; i < nWeapons; ++i)
    {
        if (pWeapons[i])
        {
            pCity->weapon_pool.emplace(weapon_type(i), pWeapons[i]);
            pWeapons[i] = nullptr;
        }
    }
    pCity->pWarriors[pHeadquarter->type] = nullptr;
    pHeadquarter->pWarriors.erase(id);
}

void Warrior::print_self() { cout << Headquarter::headquarter_name[pHeadquarter->type] << ' ' << warrior_name[type] << ' ' << id; }

void Warrior::pick_weapon()
{
    map<weapon_type, Weapon *> &pool = pCity->weapon_pool;
    for (int i = 0; i < nWeapons; ++i)
    {
        if (pWeapons[i])
        {
            continue;
        }
        auto weapon = pool.find(weapon_type(i));
        if (weapon != pool.end())
        {
            pWeapons[i] = weapon->second;
            pool.erase(weapon_type(i));
        }
    }
}

bool Warrior::try_to_shot(Warrior *enemy)
{
    if (!pWeapons[arrow] || enemy->is_dead() || enemy->is_at_home())
    {
        return 0;
    }
    enemy->hurted(pWeapons[arrow]->attack_value);
    pWeapons[arrow]->utilize();
    try_to_destroy_weapon(arrow);
    return 1;
}

bool Warrior::try_to_use_bomb(Warrior *enemy, const city_type &active)
{
    if (!pWeapons[bomb] || is_dead() || enemy->is_dead())
    {
        return 0;
    }
    Result mode = pHeadquarter->type == active ? Actively : Passively;
    if (preview(enemy, mode) < Tie)
    {
        enemy->elements = elements = 0;
        pWeapons[bomb]->utilize();
        try_to_destroy_weapon(bomb);
        print_time(), print_self();
        cout << " used a bomb and killed ";
        enemy->print_self();
        cout << endl;
        return 1;
    }
    return 0;
}

void Warrior::actively_attack(Warrior *enemy)
{
    int attack_value = force;
    if (pWeapons[sword])
    {
        attack_value += pWeapons[sword]->attack_value;
        pWeapons[sword]->utilize();
        try_to_destroy_weapon(sword);
    }
    enemy->hurted(attack_value);
    print_time(), print_self();
    cout << " attacked ";
    enemy->print_self();
    cout << " in ";
    pCity->print_self();
    cout << " with " << elements << " elements and force " << force << endl;
    if (enemy->is_dead())
    {
        enemy->report_death();
    }
}

void Warrior::passively_attack(Warrior *enemy)
{
    int attack_value = force / 2;
    if (pWeapons[sword])
    {
        attack_value += pWeapons[sword]->attack_value;
        pWeapons[sword]->utilize();
        try_to_destroy_weapon(sword);
    }
    enemy->hurted(attack_value);
    print_time(), print_self();
    cout << " fought back against ";
    enemy->print_self();
    cout << " in ";
    pCity->print_self();
    cout << endl;
    if (enemy->is_dead())
    {
        enemy->report_death();
    }
}

void Warrior::send_elements_to_headquarter(const int &value)
{
    pHeadquarter->elements_buffer += value;
    print_time(), print_self();
    cout << " earned " << value << " elements for his headquarter" << endl;
}

void Warrior::report_death()
{
    print_time(), print_self();
    cout << " was killed in ";
    pCity->print_self();
    cout << endl;
}

bool Warrior::march()
{
    if (is_at_target_city())
    {
        return 0;
    }
    move = 1;
    during_march();
    city_type _type = pHeadquarter->type;
    pCity->pWarriors[_type] = nullptr;
    pCity += (_type == red ? 1 : -1);
    bool conquer = pCity->pWarriors[_type];
    pCity->pWarriors[_type] = this;
    return conquer;
}

void Warrior::report_arrival()
{
    move = 0, print_time(), print_self();
    if (is_at_target_city())
    {
        cout << " reached " << Headquarter::headquarter_name[pHeadquarter->type ^ 1] << " headquarter";
    }
    else
    {
        cout << " marched to ";
        pCity->print_self();
    }
    cout << " with " << elements << " elements and force " << force << endl;
}

bool Warrior::is_at_target_city() { return pCity->index == nCities + 1 - pHeadquarter->pCity->index; }

bool Warrior::is_at_home() { return pCity == pHeadquarter->pCity; }

int Warrior::position() { return pCity->index; }

void Dragon::after_attack(Warrior *enemy, const Result &result)
{
    if (result < Tie)
    {
        return;
    }
    morale += result >= Win ? 0.2 : -0.2;
    if (result != Actively + Tie && result != Actively + Win)
    {
        return;
    }
    if (morale > 0.8)
    {
        print_time(), print_self();
        cout << " yelled in ";
        pCity->print_self();
        cout << endl;
    }
}

void City::raise_flag()
{
    for (int i = 0; i < 2; ++i)
    {
        if (pWarriors[i] && pWarriors[i]->is_dead())
        {
            delete pWarriors[i];
        }
    }
    for (int i = 0; i < 2; ++i)
    {
        if (pWarriors[i])
        {
            pWarriors[i]->pick_weapon();
        }
    }
    weapon_pool.clear(), warrior_earn_elements();
    if (state == Nothing || curr_win != prev_win || flag == curr_win || curr_win == neutral)
    {
        return;
    }
    flag = prev_win;
    print_time();
    cout << Headquarter::headquarter_name[flag] << " flag raised in ";
    print_self();
    cout << endl;
}

bool for_all_cities(City *start, City *finish, void (*f)(City *), const int &time_increment = 0)
{
    minute += time_increment;
    if (time_not_valid())
    {
        return 1;
    }
    for (; start != finish; ++start)
    {
        f(start);
    }
    return 0;
}

inline void lion_escape_func(City *city) { city->lion_escape(); }

inline void produce_elements_func(City *city) { city->produce_elements(); }

inline void earn_elements_func(City *city) { city->warrior_earn_elements(); }

inline void shot_func(City *city) { city->warrior_shot(); }

inline void explode_func(City *city) { city->warrior_explode(); }

inline void fight_func(City *city) { city->warrior_fight(); }

inline void reset_record_func(City *city) { city->reset_record(); }

int main()
{
    freopen("data.in", "r", stdin);
    freopen("WarCraft.out", "w", stdout);
    int cases;
    cin >> cases;
    for (int k = 1; k <= cases; ++k)
    {
        hour = minute = 0;
        cin >> init_elements >> nCities >> arrow_attack >> loyalty_decrease >> time_limit;
        for (int i = 0; i < nWarriors; ++i)
        {
            cin >> Warrior::elements_value[i];
        }
        for (int i = 0; i < nWarriors; ++i)
        {
            cin >> Warrior::force_value[i];
        }
        cout << "Case " << k << ':' << endl;

        City *city = new City[nCities + 2], *start = city, *finish = city + nCities + 2;
        Headquarter Red(start, red), Blue(finish - 1, blue);

        while (!time_not_valid())
        {
            Red.produce(), Blue.produce();
            if (for_all_cities(start, finish, lion_escape_func, 5))
            {
                break;
            }
            minute += 5;
            if (time_not_valid())
            {
                break;
            }
            bool red_victory = Red.march_and_if_conquer(), blue_victory = Blue.march_and_if_conquer();
            for (City *i = start; i < finish; ++i)
            {
                i->warrior_arrive();
                if (i == start && blue_victory)
                {
                    Blue.report_conquer();
                }
                if (i == finish - 1 && red_victory)
                {
                    Red.report_conquer();
                }
            }
            if (red_victory || blue_victory)
            {
                break;
            }
            if (for_all_cities(start + 1, finish - 1, produce_elements_func, 10))
            {
                break;
            }
            if (for_all_cities(start + 1, finish - 1, earn_elements_func, 10))
            {
                break;
            }
            if (for_all_cities(start, finish, shot_func, 5))
            {
                break;
            }
            if (for_all_cities(start, finish, explode_func, 3))
            {
                break;
            }
            if (for_all_cities(start, finish, fight_func, 2))
            {
                break;
            }
            Red.award_elements(), Blue.award_elements();
            for_all_cities(start, finish, reset_record_func);
            minute += 10;
            if (time_not_valid())
            {
                break;
            }
            Red.report_elements(), Blue.report_elements();
            minute += 5;
            if (time_not_valid())
            {
                break;
            }
            Red.report_weapons(), Blue.report_weapons();
            ++hour, minute = 0;
        }

        delete[] city;
    }
    return 0;
}