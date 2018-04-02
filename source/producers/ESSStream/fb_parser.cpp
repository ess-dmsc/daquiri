#include "fb_parser.h"
#include "custom_logger.h"

KafkaStreamConfig::KafkaStreamConfig()
{
  std::string r {plugin_name()};

  SettingMeta topic(r + "/KafkaTopic", SettingType::text, "Kafka topic");
  topic.set_flag("preset");
  add_definition(topic);

  SettingMeta drop {r + "/KafkaFF", SettingType::boolean, "Fast-forward to recent packets"};
  add_definition(drop);

  SettingMeta mb(r + "/KafkaMaxBacklog", SettingType::integer, "Kafka maximum backlog");
  mb.set_val("min", 1);
  mb.set_val("units", "buffers");
  add_definition(mb);

  int32_t i {0};
  SettingMeta root(r, SettingType::stem, "Kafka topic configuration");
  root.set_enum(i++, r + "/KafkaTopic");
  root.set_enum(i++, r + "/KafkaFF");
  root.set_enum(i++, r + "/KafkaMaxBacklog");
  add_definition(root);
}

Setting KafkaStreamConfig::settings() const
{
  std::string r{plugin_name()};
  auto set = get_rich_setting(r);
  set.set(Setting::text(r + "/KafkaTopic", kafka_topic_name_));
  set.set(Setting::boolean(r + "/KafkaFF", kafka_ff_));
  set.set(Setting::integer(r + "/KafkaMaxBacklog", kafka_max_backlog_));
  return set;
}

void KafkaStreamConfig::settings(const Setting& set)
{
  std::string r{plugin_name()};
  kafka_topic_name_ = set.find({r + "/KafkaTopic"}).get_text();
  kafka_ff_ = set.find({r + "/KafkaFF"}).get_bool();
  kafka_max_backlog_ = set.find({r + "/KafkaMaxBacklog"}).get_int();
}

fb_parser::fb_parser()
    : Producer()
{
}

Setting fb_parser::settings() const
{
  std::string r {this->plugin_name()};
  auto set = this->get_rich_setting(r);
  set.branches.add_a(kafka_config.settings());
  return set;
}

void fb_parser::settings(const Setting& set)
{
  std::string r {this->plugin_name()};
  kafka_config.settings(set.find({kafka_config.plugin_name()}));
}


void fb_parser::boot()
{
  if (!(status_ & ProducerStatus::can_boot))
    return;
  status_ = ProducerStatus::loaded | ProducerStatus::booted;
}

void fb_parser::die()
{
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}
