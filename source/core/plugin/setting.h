#pragma once

#include "setting_metadata.h"
#include <boost/date_time.hpp>
#include "pattern.h"
#include "container.h"

namespace DAQuiri {

enum Match
{
  id = 1 << 0,
  name = 1 << 1, //deprecate?
  address = 1 << 2, //deprecate?
  indices = 1 << 3,
  stype = 1 << 4,
  value = 1 << 5
};

inline Match operator|(Match a, Match b)
{
  return static_cast<Match>(static_cast<int>(a) | static_cast<int>(b));
}

inline Match operator&(Match a, Match b)
{
  return static_cast<Match>(static_cast<int>(a) & static_cast<int>(b));
}

class Setting;

class Setting
{
  public:
    Container<Setting> branches;

    Setting() {}
    Setting(std::string id);
    Setting(SettingMeta meta);
    Setting(std::string sid, boost::posix_time::ptime v);
    Setting(std::string sid, boost::posix_time::time_duration v);
    Setting(std::string sid, Pattern v);
    static Setting floating(std::string sid, floating_t val);
    static Setting precise(std::string sid, precise_t val);
    static Setting boolean(std::string sid, bool val);
    static Setting integer(std::string sid, integer_t val);
    static Setting indicator(std::string sid, integer_t val);
    static Setting text(std::string sid, std::string val);
    static Setting stem(std::string sid);

    // id, type, metadata
    explicit operator bool() const;
    std::string id() const;
    bool is(SettingType type) const;
    SettingMeta metadata() const;
    void strip_metadata();
    void enrich(const std::map<std::string, SettingMeta> &,
                bool impose_limits = false);

    // equality
    bool shallow_equals(const Setting &other) const;
    bool operator==(const Setting &other) const;
    bool operator!=(const Setting &other) const;
    bool compare(const Setting &other, Match m) const;

    //time
    void set_time(boost::posix_time::ptime v);
    boost::posix_time::ptime time() const;

    //duration
    void set_duration(boost::posix_time::time_duration v);
    boost::posix_time::time_duration duration() const;

    //pattern
    void set_pattern(Pattern v);
    Pattern pattern() const;

    //numerics
    bool numeric() const;
    double get_number() const;
    void set_number(double);
    // prefix
    Setting &operator++();
    Setting &operator--();
    // postfix
    Setting operator++(int);
    Setting operator--(int);

    // menu/indicator/integer
    void set_int(integer_t v);
    integer_t get_int() const;
    void select(integer_t v);
    integer_t selection() const;

    //command/boolean
    bool get_bool() const;
    void set_bool(bool);
    bool triggered() const;
    void trigger();
    void reset();

    //text
    void set_text(std::string val);
    std::string get_text() const;

    // indices
    void clear_indices();
    void set_indices(std::set<int32_t> l);
    void add_indices(std::set<int32_t> l);
    bool has_index(int32_t i) const;
    std::set<int32_t> indices() const;

    // assign value
    void set_val(const Setting &other);

    // recursive access
    void replace(const Setting &s, Match m = Match::id, bool greedy = false);
    void set(const Setting &s, Match m = Match::id, bool greedy = false);
    //template for all containers?
    void set(const std::list<Setting> &s, Match m = Match::id, bool greedy = false);
    bool has(Setting address, Match m = Match::id) const;
    void erase(Setting address, Match m = Match::id);
    Setting find(Setting address, Match m = Match::id) const;
    std::list<Setting> find_all(const Setting &setting, Match m = Match::id) const;

    // metadata-related convenience functions
    void enable_if_flag(bool enable, const std::string &flag);
    void hide(bool h = true);
    void condense();
    void cull_hidden();
    void cull_readonly();
    void enforce_limits();

    // to string
    std::string debug(std::string prepend = "", bool verbose = true) const;
    std::string val_to_string() const;
    std::string indices_to_string(bool showblanks = false) const;

    // serialization
    friend void to_json(json &j, const Setting &s);
    friend void from_json(const json &j, Setting &s);

  private:
    bool compare_indices(const Setting &other) const;

    bool find_dfs(Setting &result, const Setting &root, Match m) const;
    void erase_matches(const Setting &, Setting &, Match m);
    bool set_first(const Setting &setting, Match m);
    void set_all(const Setting &setting, Match m);

    bool replace_first(const Setting &setting, Match m);
    void replace_all(const Setting &setting, Match m);

    json val_to_json() const;
    void val_from_json(const json &j);

    SettingMeta metadata_;
    std::set<int32_t> indices_;

    integer_t value_int{0};
    floating_t value_dbl{0};
    precise_t value_precise{0};
    std::string value_text;
    boost::posix_time::ptime value_time {boost::posix_time::not_a_date_time};
    boost::posix_time::time_duration value_duration {boost::posix_time::not_a_date_time};
    Pattern value_pattern;
};

}
