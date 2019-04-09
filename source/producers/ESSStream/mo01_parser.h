#pragma once

#include <producers/ESSStream/fb_parser.h>
#include "mo01_nmx_generated.h"

using namespace DAQuiri;

class mo01_nmx : public fb_parser
{
 public:
  mo01_nmx();

  ~mo01_nmx() = default;

  std::string plugin_name() const override
  { return "mo01_nmx"; }

  std::string schema_id() const override;
  std::string get_source_name(void* msg) const override;

  void settings(const Setting&) override;
  Setting settings() const override;

  uint64_t process_payload(SpillQueue spill_queue, void* msg) override;
  uint64_t stop(SpillQueue spill_queue) override;

  StreamManifest stream_manifest() const override;

 private:
  // cached params

  bool filter_source_name_{false};
  std::string source_name_;

  std::string hists_stream_id_{"nmx_hists"};
  std::string x_stream_id_{"nmx_xtrack"};
  std::string y_stream_id_{"nmx_ytrack"};
  std::string hit_stream_id_{"mon_hits"};

  EventModel hists_model_;
  EventModel track_model_;
  EventModel hits_model_;

  uint64_t spoofed_time_{0};
  bool started_{false};

  uint64_t produce_hists(const GEMHist&, SpillQueue queue);
  uint64_t produce_tracks(const GEMTrack&, SpillQueue queue);
  uint64_t produce_hits(const MONHit&, SpillQueue queue);

  static void grab_hist(Event& e, size_t idx, const flatbuffers::Vector<uint32_t>* data);
  SpillPtr grab_track(const flatbuffers::Vector<flatbuffers::Offset<pos>>* data,
                      std::string stream);

  static std::string debug(const GEMHist&);
  static std::string debug(const GEMTrack&);

  static std::string print_hist(const flatbuffers::Vector<uint32_t>* data);
  static std::string print_track(const flatbuffers::Vector<flatbuffers::Offset<pos>>* data);
};
