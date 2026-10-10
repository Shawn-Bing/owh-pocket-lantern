package main

import (
	"reflect"
	"strconv"
	"testing"
	"time"
)

func alertCounts(s *Server) map[string]int {
	counts := map[string]int{}
	for _, e := range s.events {
		if e.AlertEvent != "" {
			counts[e.AlertEvent]++
		}
	}
	return counts
}
func nextAlertPacket(h *Heartbeat, now time.Time) {
	h.V2.Sequence = strconv.FormatUint(number(h.V2.Sequence)+1, 10)
	tick := number(h.V2.SentAtMS) + 1000
	h.V2.SentAtMS = strconv.FormatUint(tick, 10)
	h.V2.SentAt = now.Format(time.RFC3339Nano)
	o := &h.V2.Observation
	o.LastObservedAtMS = h.V2.SentAtMS
	o.LastReliableVisualAtMS = h.V2.SentAtMS
	o.ProcessObservedAtMS = h.V2.SentAtMS
	if o.State == "IN_MATCH" {
		o.LastConfirmedAtMS = h.V2.SentAtMS
	}
	h.V2.Hardware = nil
}
func TestAlertNoReaderExpiryAndDedup(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	postV2(t, s, h, 200)
	if a := getAgent(t, s).Alert; !a.Enabled || a.Kind != "NONE" || a.Reached {
		t.Fatal(a)
	}
	// No reads or heartbeats after this. Known proof deadline defines the start,
	// rather than the first viewer's arrival or the stale ACTIVE state.
	*now = now.Add(315 * time.Second)
	a := getAgent(t, s).Alert
	if !a.Reached || a.DurationMS != 300000 || a.Kind != "OFFLINE" || a.EpisodeID == "" {
		t.Fatalf("%+v", a)
	}
	for i := 0; i < 10; i++ {
		getAgent(t, s)
		sseSnapshot(t, s)
	}
	counts := alertCounts(s)
	if counts["started"] != 1 || counts["threshold_reached"] != 1 {
		t.Fatal(counts)
	}
	if !reflect.DeepEqual(getAgent(t, s).Alert, sseSnapshot(t, s).Agents[0].Alert) {
		t.Fatal("GET/SSE alert differs")
	}
	nextAlertPacket(&h, *now)
	postV2(t, s, h, 200)
	if a = getAgent(t, s).Alert; a.Kind != "NONE" || a.Reached || a.EpisodeID != "" {
		t.Fatal(a)
	}
	if alertCounts(s)["resolved"] != 1 {
		t.Fatal(alertCounts(s))
	}
	// A second independent abnormal interval can notify again.
	h.V2.Observation.State = "UNVERIFIED"
	nextAlertPacket(&h, *now)
	postV2(t, s, h, 200)
	*now = now.Add(300 * time.Second)
	a = getAgent(t, s).Alert
	if !a.Reached || alertCounts(s)["threshold_reached"] != 2 {
		t.Fatal(a, alertCounts(s))
	}
}
func TestAlertMixedKindsAndRecoverySessions(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	h.V2.Observation.State = "OUT_OF_MATCH"
	h.V2.Observation.Confidence = "VISUAL"
	h.V2.Observation.Page = "Home"
	postV2(t, s, h, 200)
	id := getAgent(t, s).Alert.EpisodeID
	*now = now.Add(10 * time.Second)
	h.V2.Observation.State = "UNVERIFIED"
	nextAlertPacket(&h, *now)
	postV2(t, s, h, 200)
	*now = now.Add(280 * time.Second)
	// Process/session recovery within one observer revision is not a manual start.
	h.V2.Observation.SessionID = "recovery-session"
	h.V2.Observation.GamePID = 202
	h.V2.Observation.GameCreated = "134000000000000002"
	nextAlertPacket(&h, *now)
	postV2(t, s, h, 200)
	*now = now.Add(10 * time.Second)
	h.V2.Observation.State = "OUT_OF_MATCH"
	nextAlertPacket(&h, *now)
	postV2(t, s, h, 200)
	a := getAgent(t, s).Alert
	if a.EpisodeID != id || !a.Reached || a.DurationMS != 300000 || a.TypeDurationMS != 0 || a.Kind != "OUT_OF_MATCH" {
		t.Fatalf("%+v", a)
	}
	if alertCounts(s)["started"] != 1 || alertCounts(s)["threshold_reached"] != 1 {
		t.Fatal(alertCounts(s))
	}
}
func TestAlertStopRestartAndNewInstance(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	h.V2.Observation.State = "UNVERIFIED"
	postV2(t, s, h, 200)
	*now = now.Add(299 * time.Second)
	h.State = "STOPPED"
	h.V2.Observation.AutomationState = "STOPPED"
	h.V2.Observation.Running = false
	h.V2.Observation.Revision = "2"
	h.V2.Observation.SessionID = "stopped"
	nextAlertPacket(&h, *now)
	postV2(t, s, h, 200)
	*now = now.Add(time.Hour)
	if a := getAgent(t, s).Alert; a.Enabled || a.Reached || a.Reason != "assistant_stopped" {
		t.Fatal(a)
	}
	if alertCounts(s)["threshold_reached"] != 0 {
		t.Fatal(alertCounts(s))
	}
	h.State = "ACTIVE"
	h.V2.Observation.AutomationState = "ACTIVE"
	h.V2.Observation.Running = true
	h.V2.Observation.Revision = "3"
	h.V2.Observation.SessionID = "new-running"
	nextAlertPacket(&h, *now)
	postV2(t, s, h, 200)
	id := getAgent(t, s).Alert.EpisodeID
	*now = now.Add(100 * time.Second)
	h.V2.AgentCreated = "134000000000000100"
	h.V2.Sequence = "1"
	h.V2.Observation.Revision = "1"
	h.V2.Observation.SessionID = "new-agent"
	h.V2.SentAt = now.Format(time.RFC3339Nano)
	postV2(t, s, h, 200)
	a := getAgent(t, s).Alert
	if a.EpisodeID == id || a.DurationMS != 0 || a.Reached {
		t.Fatal(a)
	}
}
func TestAlertRestartRequiresFreshHeartbeat(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	h.V2.Observation.State = "UNVERIFIED"
	postV2(t, s, h, 200)
	*now = now.Add(300 * time.Second)
	getAgent(t, s)
	if err := s.save(); err != nil {
		t.Fatal(err)
	}
	restored := NewServer("a", "v", s.dataDir)
	restored.now = func() time.Time { return *now }
	if err := restored.load(); err != nil {
		t.Fatal(err)
	}
	a := getAgent(t, restored).Alert
	if a.Enabled || a.Reached || a.Reason != "awaiting_fresh_running_heartbeat" {
		t.Fatal(a)
	}
	previousEvents := alertCounts(restored)["threshold_reached"]
	nextAlertPacket(&h, *now)
	postV2(t, restored, h, 200)
	a = getAgent(t, restored).Alert
	if !a.Enabled || a.Reached || a.DurationMS != 0 || previousEvents != 1 {
		t.Fatal(a, previousEvents)
	}
}
func TestAlertLegacyDryRunAndClock(t *testing.T) {
	for _, variant := range []string{"legacy", "dry_run", "clock"} {
		t.Run(variant, func(t *testing.T) {
			s, now := newProtocolServer(t)
			h := freshHeartbeat()
			h.V2.Observation.State = "UNVERIFIED"
			switch variant {
			case "legacy":
				h.V2 = nil
			case "dry_run":
				h.DryRun = true
			case "clock":
				h.V2.SentAt = now.Add(time.Hour).Format(time.RFC3339Nano)
			}
			postV2(t, s, h, 200)
			*now = now.Add(time.Hour)
			if a := getAgent(t, s).Alert; a.Enabled || a.Reached || len(alertCounts(s)) != 0 {
				t.Fatal(a, alertCounts(s))
			}
		})
	}
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	h.V2.Observation.State = "UNVERIFIED"
	postV2(t, s, h, 200)
	*now = now.Add(-time.Second)
	if a := getAgent(t, s).Alert; a.Enabled || a.Reached {
		t.Fatal("clock rollback retained timer", a)
	}
}
func TestAlertGameAbsenceIsNotConnectionOffline(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	o := &h.V2.Observation
	o.State = "OFFLINE"
	o.Confidence = "PROCESS"
	o.GamePID = 0
	o.GameCreated = "0"
	postV2(t, s, h, 200)
	a := getAgent(t, s).Alert
	if a.Kind != "OUT_OF_MATCH" || a.Reason != "game_process_absent" {
		t.Fatal(a)
	}
	*now = now.Add(91 * time.Second)
	if a = getAgent(t, s).Alert; a.Kind != "OFFLINE" || a.Reason != "heartbeat_missing" {
		t.Fatal(a)
	}
}
func TestAlertRuntimeTickNotifiesWithoutViewers(t *testing.T) {
	s, now := newProtocolServer(t)
	s.alertAfter = 2 * time.Second
	h := freshHeartbeat()
	h.V2.Observation.State = "UNVERIFIED"
	postV2(t, s, h, 200)
	changes := make(chan struct{}, 1)
	s.subscribers[changes] = struct{}{}
	*now = now.Add(2 * time.Second)
	s.mu.Lock()
	s.refreshAlertsLocked(*now)
	s.mu.Unlock()
	select {
	case <-changes:
	default:
		t.Fatal("runtime threshold did not signal SSE")
	}
	if alertCounts(s)["threshold_reached"] != 1 {
		t.Fatal(alertCounts(s))
	}
}

func TestAlertLongGapHeartbeatClosesThresholdEpisode(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	postV2(t, s, h, 200)
	*now = now.Add(315 * time.Second)
	// A newly received green packet must not erase an unseen expired interval.
	nextAlertPacket(&h, *now)
	postV2(t, s, h, 200)
	counts := alertCounts(s)
	if counts["started"] != 1 || counts["threshold_reached"] != 1 || counts["resolved"] != 1 {
		t.Fatal(counts)
	}
	if a := getAgent(t, s).Alert; a.Kind != "NONE" || a.Reached {
		t.Fatal(a)
	}
}
func TestAlertOfflineTypeDurationHasIndependentBoundary(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	postV2(t, s, h, 200)
	*now = now.Add(315 * time.Second)
	a := getAgent(t, s).Alert
	if a.DurationMS != 300000 || a.TypeDurationMS != 225000 || a.Kind != "OFFLINE" {
		t.Fatal(a)
	}
}
func TestAlertHostAgePrefersRuntimeReceipt(t *testing.T) {
	s, _ := newProtocolServer(t)
	h := freshHeartbeat()
	postV2(t, s, h, 200)
	cached := s.agents[h.Machine]
	cached.Seen = cached.Seen.Add(-time.Hour)
	s.agents[h.Machine] = cached
	if !getAgent(t, s).Monitor.HostOnline {
		t.Fatal("wall-clock Seen replaced live receipt age")
	}
}

func TestAlertClockFailureDoesNotMeanManualStop(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	postV2(t, s, h, 200)
	*now = now.Add(time.Second)
	nextAlertPacket(&h, *now)
	h.V2.SentAt = now.Add(time.Hour).Format(time.RFC3339Nano)
	postV2(t, s, h, 200)
	a := getAgent(t, s).Alert
	if !a.Enabled || a.Kind != "UNVERIFIED" {
		t.Fatal(a)
	}
	*now = now.Add(300 * time.Second)
	a = getAgent(t, s).Alert
	if !a.Enabled || !a.Reached || a.DurationMS != 300000 {
		t.Fatal(a)
	}
}
