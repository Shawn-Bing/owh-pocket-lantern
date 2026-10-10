#include "core/telemetry_v2.hpp"
#include <iostream>
#include <stdexcept>
using namespace owh;
void require(bool v,const char *why) { if(!v) throw std::runtime_error(why); }
MatchSnapshot matchFixture() {
  MatchSnapshot s;
  s.mode=WorkflowMode::CustomRoom; s.automation=AutomationState::ACTIVE;
  s.game={101,134000000000000001ULL}; s.running=true;
  s.state=MatchState::InMatch; s.confidence=MatchConfidence::RepeatedVisual;
  s.source=MatchSource::TargetTitle; s.sessionId="agent:1:1:101:134000000000000001";
  s.revision=1; s.lastConfirmedAt=s.lastReliableVisualAt=s.lastObservedAt=s.processObservedAt=10000;
  s.page=CNPage::Room; s.window.foreground=true; s.lastError="";
  s.evidenceTtlMs=s.processTtlMs=15000; return s;
}
int main(int argc,char **argv) {
  auto match=matchFixture();
  HardwareSnapshot hardware;
  hardware.sampledAt="2026-10-10T17:00:00.000Z"; hardware.sampledTick=10000; hardware.sequence=9;
  hardware.cpuTemperature.reason=hardware.cpuPower.reason="cpu_access_denied";
  hardware.gpuPower={40.344,"watts","nvml","GPU-fixture",""};
  if(argc==2 && std::string(argv[1])=="--fixture") {
    std::cout<<"{\"machine\":\"cpp-fixture\",\"state\":\"ACTIVE\",\"detail\":\"v2 fixture\",\"pid\":101,\"uptime\":10,\"restarts\":0,\"score\":0.95,\"dry_run\":false,\"version\":\"0.5.13\",\"v2\":"
      <<telemetryV2Json(55,134000000000000099ULL,1,10000,"2026-10-10T17:00:00.000Z",match,hardware)<<"}\n";
    return 0;
  }
  try {
    TelemetrySchedule schedule;
    require(schedule.due(10000,false,match,hardware),"initial update");
    schedule.sent(10000,match,hardware);
    require(!schedule.due(11000,true,match,hardware),"2s coalescing");
    require(schedule.due(12000,true,match,hardware),"status change update");
    match.lastConfirmedAt=match.lastReliableVisualAt=match.lastObservedAt=19000;
    require(!schedule.due(19000,false,match,hardware),"new frame does not flood");
    require(schedule.due(20000,false,match,hardware),"fresh visual 10s renewal");
    schedule.sent(20000,match,hardware);
    require(!schedule.due(30000,false,match,hardware),"old evidence cannot renew");
    hardware.sequence=10; hardware.sampledTick=30000;
    require(schedule.due(30000,false,match,hardware),"new hardware sample renewal");
    schedule.sent(30000,match,hardware);
    match.state=MatchState::Unverified; match.lastError="visual_evidence_expired";
    require(schedule.due(32000,false,match,hardware),"observer TTL downgrade is prompt");
    schedule.sent(32000,match,hardware);
    hardware.cpuTemperature.value.reset(); hardware.cpuPower.value.reset(); hardware.gpuPower.value.reset();
    hardware.sequence=11;
    require(!schedule.due(42000,false,match,hardware),"failed hardware does not force renewal");
    require(schedule.due(62000,false,match,hardware),"30s base heartbeat");
    TelemetrySchedule nonmatchSchedule;
    match.state=MatchState::OutOfMatch; match.lastError=""; match.lastReliableVisualAt=match.lastObservedAt=70000;
    nonmatchSchedule.sent(70000,match,hardware);
    require(!nonmatchSchedule.due(80000,false,match,hardware),"old out-of-match proof cannot renew");
    match.lastReliableVisualAt=match.lastObservedAt=79000;
    require(nonmatchSchedule.due(80000,false,match,hardware),"fresh menu evidence renews without hardware readings");
    match.state=MatchState::Offline; match.confidence=MatchConfidence::Process; match.processObservedAt=90000;
    nonmatchSchedule.sent(90000,match,hardware);
    require(!nonmatchSchedule.due(100000,false,match,hardware),"old absence proof cannot renew");
    match.processObservedAt=99000;
    require(nonmatchSchedule.due(100000,false,match,hardware),"fresh absence scan renews without hardware readings");
    Json root; std::string error;
    require(parseJson(telemetryV2Json(55,134000000000000099ULL,9007199254740993ULL,20000,
        "2026-10-10T17:00:10.000Z",match,hardware),root,error),"v2 JSON valid");
    require(root.find("sequence")->text=="9007199254740993","large sequence remains exact");
    require(root.find("agent_created")->text=="134000000000000099","FILETIME remains exact");
    require(root.find("observation")->find("evidence_ttl_ms")->number==15000,"TTL comes from snapshot");
    std::cout<<"v2 serialization and scheduling tests passed\n";
  } catch(const std::exception &e) { std::cerr<<e.what()<<'\n'; return 1; }
}
