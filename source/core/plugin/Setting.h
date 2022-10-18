/* Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file Setting.h
///
/// \brief key primitive - holds strings, ints , floats, ...
///
/// \todo seems like it is intended to hold only a single type at a time. Perhaps
/// the use of union type {} could clarify this?
///
//===----------------------------------------------------------------------===//
#pragma once

#include <core/plugin/SettingMeta.h>
#include <core/plugin/Pattern.h>
#include <core/plugin/Container.h>
#include <core/util/time_extensions.h>

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

    Setting() = default;
    explicit Setting(const std::string& id);
    explicit Setting(const SettingMeta& meta);
    Setting(const std::string& sid, hr_time_t v);
    Setting(const std::string& sid, hr_duration_t v);
    Setting(const std::string& sid, Pattern v);
    static Setting floating(const std::string& sid, floating_t val);
    static Setting precise(const std::string& sid, precise_t val);
    static Setting boolean(const std::string& sid, bool val);
    static Setting integer(const std::string& sid, integer_t val);
    static Setting indicator(const std::string& sid, integer_t val);
    static Setting text(const std::string& sid, const std::string& val);
    static Setting stem(const std::string& sid);

    // id, type, metadata
    explicit operator bool() const;
    [[nodiscard]] std::string id() const;
    [[nodiscard]] bool is(SettingType type) const;
    [[nodiscard]] SettingMeta metadata() const;
    void strip_metadata();
    void enrich(const std::map<std::string, SettingMeta> &,
                bool impose_limits = false);

    // equality
    [[nodiscard]] bool shallow_equals(const Setting &other) const;
    bool operator==(const Setting &other) const;
    bool operator!=(const Setting &other) const;
    [[nodiscard]] bool compare(const Setting &other, Match m) const;

    //time
    void set_time(hr_time_t v);
    [[nodiscard]] hr_time_t time() const;

    //duration
    void set_duration(hr_duration_t v);
    [[nodiscard]] hr_duration_t duration() const;

    //pattern
    void set_pattern(Pattern v);
    [[nodiscard]] Pattern pattern() const;

    //numerics
    [[nodiscard]] bool numeric() const;
    [[nodiscard]] double get_number() const;
    void set_number(double);
    // prefix
    Setting &operator++();
    Setting &operator--();
    // postfix
    const Setting operator++(int);
    const Setting operator--(int);

    // Precise float
    void set_precise(PreciseFloat pf);
    [[nodiscard]] PreciseFloat precise() const;

    // menu/indicator/integer
    void set_int(integer_t v);
    [[nodiscard]] integer_t get_int() const;
    void select(integer_t v);
    [[nodiscard]] integer_t selection() const;

    //command/boolean
    [[nodiscard]] bool get_bool() const;
    void set_bool(bool);
    [[nodiscard]] bool triggered() const;
    void trigger();
    void reset();

    //text
    void set_text(const std::string& val);
    [[nodiscard]] std::string get_text() const;

    // indices
    void clear_indices();
    void set_indices(const std::set<int32_t>& l);
    void add_indices(const std::set<int32_t>& l);
    [[nodiscard]] bool has_index(int32_t i) const;
    [[nodiscard]] std::set<int32_t> indices() const;

    // assign value
    void set_val(const Setting &other);

    // recursive access
    void replace(const Setting &s, Match m = Match::id, bool greedy = false);
    void set(const Setting &s, Match m = Match::id, bool greedy = false);
    //template for all containers?
    void set(const std::list<Setting> &s, Match m = Match::id, bool greedy = false);
    [[nodiscard]] bool has(Setting address, Match m = Match::id) const;
    void erase(Setting address, Match m = Match::id);
    [[nodiscard]] Setting find(Setting address, Match m = Match::id) const;
    [[nodiscard]] std::list<Setting> find_all(const Setting &setting, Match m = Match::id) const;

    // metadata-related convenience functions
    void enable_if_flag(bool enable, const std::string &flag);
    void hide(bool h = true);
    void condense();
    void cull_hidden();
    void cull_readonly();
    void enforce_limits();

    // to string
    [[nodiscard]] std::string debug(const std::string& prepend = "", bool verbose = true) const;
    [[nodiscard]] std::string val_to_string() const;
    [[nodiscard]] std::string indices_to_string(bool showblanks = false) const;

    // serialization
    friend void to_json(json &j, const Setting &s);
    friend void from_json(const json &j, Setting &s);

  private:
    [[nodiscard]] bool compare_indices(const Setting &other) const;

    bool find_dfs(Setting &result, const Setting &root, Match m) const;
    void erase_matches(const Setting &, Setting &, Match m);
    bool set_first(const Setting &setting, Match m);
    void set_all(const Setting &setting, Match m);

    bool replace_first(const Setting &setting, Match m);
    void replace_all(const Setting &setting, Match m);

    [[nodiscard]] json val_to_json() const;
    void val_from_json(const json &j);

    SettingMeta metadata_;
    std::set<int32_t> indices_;

    integer_t value_int{0};
    floating_t value_dbl{0};
    precise_t value_precise{0};
    std::string value_text;
    hr_time_t value_time {};
    hr_duration_t value_duration {};
    Pattern value_pattern;
};

}
