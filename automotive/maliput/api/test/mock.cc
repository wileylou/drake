#include "drake/automotive/maliput/api/test/mock.h"

#include <unordered_map>
#include <vector>

#include "drake/automotive/maliput/api/branch_point.h"
#include "drake/automotive/maliput/api/junction.h"
#include "drake/automotive/maliput/api/lane.h"
#include "drake/automotive/maliput/api/rules/phase_ring.h"
#include "drake/automotive/maliput/api/rules/regions.h"
#include "drake/automotive/maliput/api/rules/traffic_lights.h"
#include "drake/automotive/maliput/api/segment.h"
#include "drake/common/drake_optional.h"

namespace drake {
namespace maliput {
namespace api {
namespace test {
namespace {

using rules::DirectionUsageRule;
using rules::RightOfWayRule;
using rules::SpeedLimitRule;
using rules::TrafficLight;

class MockIdIndex final : public RoadGeometry::IdIndex {
 public:
  DRAKE_NO_COPY_NO_MOVE_NO_ASSIGN(MockIdIndex);
  MockIdIndex() {}

 private:
  const Lane* DoGetLane(const LaneId&) const override { return nullptr; }

  const std::unordered_map<LaneId, const Lane*>& DoGetLanes() const override {
    return lane_map_;
  }

  const Segment* DoGetSegment(const SegmentId&) const override {
    return nullptr;
  };
  const Junction* DoGetJunction(const JunctionId&) const override {
    return nullptr;
  };
  const BranchPoint* DoGetBranchPoint(const BranchPointId&) const override {
    return nullptr;
  }

  const std::unordered_map<LaneId, const Lane*> lane_map_;
};

class MockRoadGeometry final : public RoadGeometry {
 public:
  DRAKE_NO_COPY_NO_MOVE_NO_ASSIGN(MockRoadGeometry)
  MockRoadGeometry() {}

 private:
  RoadGeometryId do_id() const override { return RoadGeometryId("mock"); }
  int do_num_junctions() const override { return 1; }
  const Junction* do_junction(int) const override { return nullptr; };
  int do_num_branch_points() const override { return 1; }
  const BranchPoint* do_branch_point(int) const override { return nullptr; }
  const IdIndex& DoById() const override { return mock_id_index_; }
  RoadPosition DoToRoadPosition(const GeoPosition&, const RoadPosition*,
                                GeoPosition*, double*) const override {
    return RoadPosition();
  }
  double do_linear_tolerance() const override { return 0; }
  double do_angular_tolerance() const override { return 0; }
  double do_scale_length() const override { return 0; }
  MockIdIndex mock_id_index_;
};

class MockRoadRulebook final : public rules::RoadRulebook {
 public:
  DRAKE_NO_COPY_NO_MOVE_NO_ASSIGN(MockRoadRulebook)
  MockRoadRulebook() {}

 private:
  QueryResults DoFindRules(const std::vector<rules::LaneSRange>&,
                           double) const override {
    return {{}, {}};
  }
  RightOfWayRule DoGetRule(
      const RightOfWayRule::Id&) const override {
    return Rule();
  }
  SpeedLimitRule DoGetRule(const SpeedLimitRule::Id&) const override {
    return SpeedLimitRule(rules::SpeedLimitRule::Id("some_id"),
                          CreateLaneSRange(),
                          rules::SpeedLimitRule::Severity::kStrict, 33.,
                          77.);
  }

  DirectionUsageRule DoGetRule(const DirectionUsageRule::Id&) const override {
    return CreateDirectionUsageRule();
  }
};

class MockTrafficLightBook final : public rules::TrafficLightBook {
 public:
  DRAKE_NO_COPY_NO_MOVE_NO_ASSIGN(MockTrafficLightBook)
  MockTrafficLightBook() {}

 private:
  optional<TrafficLight> DoGetTrafficLight(const TrafficLight::Id&) const
      override {
    return nullopt;
  }
};

class MockPhaseRingBook final : public rules::PhaseRingBook {
 public:
  DRAKE_NO_COPY_NO_MOVE_NO_ASSIGN(MockPhaseRingBook)
  MockPhaseRingBook() {}

 private:
  optional<rules::PhaseRing> DoGetPhaseRing(const rules::PhaseRing::Id&) const
      override {
    return nullopt;
  }

  optional<rules::PhaseRing> DoFindPhaseRing(const rules::RightOfWayRule::Id&)
      const override {
    return nullopt;
  }
};

class MockRuleStateProvider final : public rules::RuleStateProvider {
 public:
  DRAKE_NO_COPY_NO_MOVE_NO_ASSIGN(MockRuleStateProvider)
  MockRuleStateProvider() {}

 private:
  drake::optional<RightOfWayResult> DoGetState(const RightOfWayRule::Id&) const
      override {
    return nullopt;
  }
};

class MockPhaseProvider final : public rules::PhaseProvider {
 public:
  DRAKE_NO_COPY_NO_MOVE_NO_ASSIGN(MockPhaseProvider)
  MockPhaseProvider() {}

 private:
  optional<Result> DoGetPhase(const rules::PhaseRing::Id&) const override {
    return nullopt;
  }
};

class MockIntersection final : public Intersection {
 public:
  DRAKE_NO_COPY_NO_MOVE_NO_ASSIGN(MockIntersection)
  explicit MockIntersection(const Intersection::Id& id,
                            const rules::PhaseRing::Id& ring_id)
      : Intersection(id, {}, ring_id) {}

 private:
  optional<rules::PhaseProvider::Result> Phase() const override {
    return nullopt;
  }

  void SetPhase(const api::rules::Phase::Id&) override {}
};

}  // namespace

rules::LaneSRoute CreateLaneSRoute() {
  return rules::LaneSRoute(
      {rules::LaneSRange(LaneId("a"), {0., 9.}),
       rules::LaneSRange(LaneId("b"), {17., 12.})});
}

rules::LaneSRange CreateLaneSRange() {
  return rules::LaneSRange(LaneId("a"), {0., 9.});
}

RightOfWayRule::State::YieldGroup YieldGroup2() {
  return {RightOfWayRule::Id("other_rule_a"),
          RightOfWayRule::Id("other_rule_b")};
}

RightOfWayRule::State NoYieldState() {
  return RightOfWayRule::State(RightOfWayRule::State::Id("s1"),
                               RightOfWayRule::State::Type::kStop, {});
}

RightOfWayRule::State YieldState() {
  return RightOfWayRule::State(RightOfWayRule::State::Id("s2"),
                               RightOfWayRule::State::Type::kGo, YieldGroup2());
}

RightOfWayRule Rule() {
  return RightOfWayRule(RightOfWayRule::Id("mock_id"), CreateLaneSRoute(),
                        RightOfWayRule::ZoneType::kStopExcluded,
                        {NoYieldState(), YieldState()});
}

DirectionUsageRule::State CreateDirectionUsageRuleState() {
  return DirectionUsageRule::State(
    DirectionUsageRule::State::Id("dur_state"),
    DirectionUsageRule::State::Type::kWithS,
    DirectionUsageRule::State::Severity::kStrict);
}

DirectionUsageRule CreateDirectionUsageRule() {
  return DirectionUsageRule(DirectionUsageRule::Id("dur_id"),
                            CreateLaneSRange(),
                            {CreateDirectionUsageRuleState()});
}

std::unique_ptr<RoadGeometry> CreateRoadGeometry() {
  return std::make_unique<MockRoadGeometry>();
}

std::unique_ptr<rules::RoadRulebook> CreateRoadRulebook() {
  return std::make_unique<MockRoadRulebook>();
}

std::unique_ptr<rules::TrafficLightBook> CreateTrafficLightBook() {
  return std::make_unique<MockTrafficLightBook>();
}

std::unique_ptr<rules::PhaseRingBook> CreatePhaseRingBook() {
  return std::make_unique<MockPhaseRingBook>();
}

std::unique_ptr<rules::RuleStateProvider> CreateRuleStateProvider() {
  return std::make_unique<MockRuleStateProvider>();
}

std::unique_ptr<rules::PhaseProvider> CreatePhaseProvider() {
  return std::make_unique<MockPhaseProvider>();
}

std::unique_ptr<Intersection> CreateIntersection(
    const Intersection::Id& id, const rules::PhaseRing::Id& ring_id) {
  return std::make_unique<MockIntersection>(id, ring_id);
}

}  // namespace test
}  // namespace api
}  // namespace maliput
}  // namespace drake
