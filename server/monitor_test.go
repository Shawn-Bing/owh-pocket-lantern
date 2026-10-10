package main

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"net/http/httptest"
	"os"
	"os/exec"
	"path/filepath"
	"reflect"
	"strconv"
	"strings"
	"sync"
	"testing"
	"time"
)

var protocolNow = time.Date(2026, 10, 10, 17, 0, 0, 0, time.UTC)

func freshHeartbeat() Heartbeat {
	power := 40.344
	return Heartbeat{Machine: "pc-v2", State: "ACTIVE", Version: "0.5.13", V2: &TelemetryV2{
		AgentPID: 55, AgentCreated: "134000000000000099", Sequence: "1", SentAt: protocolNow.Format(time.RFC3339Nano), SentAtMS: "100000",
		Observation: Observation{SessionID: "session-1", Revision: "1", Mode: "CUSTOM_ROOM", AutomationState: "ACTIVE", Running: true, GamePID: 101, GameCreated: "134000000000000001", State: "IN_MATCH", Confidence: "REPEATED_VISUAL", Source: "TARGET_JOIN", LastConfirmedAtMS: "100000", LastObservedAtMS: "100000", LastReliableVisualAtMS: "100000", ProcessObservedAtMS: "100000", Page: "Room", Window: MatchWindow{Foreground: true}, EvidenceTTLMS: 15000, ProcessTTLMS: 15000},
		Hardware: &Hardware{SampledAt: protocolNow.Format(time.RFC3339Nano), SampledTickMS: "100000", Sequence: "9", IntervalMS: 5000, StaleAfterMS: 15000,
			CPUPackageTemperature: Reading{Unit: "celsius", Source: "lhm", Reason: "cpu_access_denied"}, CPUPackagePower: Reading{Unit: "watts", Source: "lhm", Reason: "cpu_access_denied"}, GPUPower: Reading{Value: &power, Unit: "watts", Source: "nvml", Sensor: "GPU-fixture"}},
	}}
}
func postV2(t *testing.T, s *Server, h Heartbeat, want int) {
	t.Helper()
	b, e := json.Marshal(h)
	if e != nil {
		t.Fatal(e)
	}
	w := request(s, "POST", "/api/heartbeat", "a", b)
	if w.Code != want {
		t.Fatalf("POST want%d got%d: %s", want, w.Code, w.Body.String())
	}
}
func newProtocolServer(t *testing.T) (*Server, *time.Time) {
	t.Helper()
	now := protocolNow
	s := NewServer("a", "v", t.TempDir())
	s.now = func() time.Time { return now }
	return s, &now
}
func getAgent(t *testing.T, s *Server) Agent {
	t.Helper()
	w := request(s, "GET", "/api/status", "v", nil)
	var x Snapshot
	if w.Code != 200 || json.Unmarshal(w.Body.Bytes(), &x) != nil || len(x.Agents) != 1 {
		t.Fatalf("bad status: %s", w.Body.String())
	}
	return x.Agents[0]
}
func sseSnapshot(t *testing.T, s *Server) Snapshot {
	t.Helper()
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	r := httptest.NewRequest("GET", "/api/events", nil).WithContext(ctx)
	r.Header.Set("Authorization", "Bearer v")
	w := &cancelOnFlushRecorder{ResponseRecorder: httptest.NewRecorder(), cancel: cancel}
	s.handler().ServeHTTP(w, r)
	var x Snapshot
	data := strings.TrimSuffix(strings.TrimPrefix(w.Body.String(), "event: snapshot\ndata: "), "\n\n")
	if w.Code != 200 || json.Unmarshal([]byte(data), &x) != nil {
		t.Fatalf("bad SSE: %s", w.Body.String())
	}
	return x
}
func TestV2FreshnessAndReadIsolation(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	postV2(t, s, h, 200)
	a := getAgent(t, s)
	if a.Monitor.State != "IN_MATCH" || a.Monitor.EvidenceRemainingMS != 15000 || a.V2.Hardware.GPUPower.Value == nil {
		t.Fatalf("not fresh: %+v", a.Monitor)
	}
	if !reflect.DeepEqual(a, sseSnapshot(t, s).Agents[0]) {
		t.Fatal("GET/SSE disagree")
	}
	*now = now.Add(15 * time.Second)
	a = getAgent(t, s)
	if a.Monitor.State != "UNVERIFIED" || a.V2.Observation.State != "UNVERIFIED" || a.V2.Hardware.GPUPower.Value != nil || a.V2.Hardware.GPUPower.Reason != "sample_expired" {
		t.Fatal("expired cached green/hardware survived")
	}
	if s.agents[h.Machine].V2.Observation.State != "IN_MATCH" || s.agents[h.Machine].V2.Hardware.GPUPower.Value == nil {
		t.Fatal("read mutated cached original")
	}
	// Snapshot pointers are isolated from storage and each other.
	a = getAgent(t, s)
	a.V2.Observation.LastError = "mutation"
	a.V2.Hardware.GPUPower.Source = "mutation"
	if getAgent(t, s).V2.Hardware.GPUPower.Source == "mutation" {
		t.Fatal("read alias leaked")
	}
	*now = now.Add(76 * time.Second)
	if getAgent(t, s).Monitor.HostOnline {
		t.Fatal("host remained online")
	}
}
func TestV2DelayedHeartbeatsCannotRenewEvidence(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	postV2(t, s, h, 200)
	*now = now.Add(10 * time.Second)
	h.V2.Sequence = "2"
	h.V2.SentAtMS = "100001"
	h.V2.SentAtMS = "110000"
	h.V2.SentAt = now.Format(time.RFC3339Nano)
	postV2(t, s, h, 200)
	if getAgent(t, s).Monitor.EvidenceRemainingMS != 5000 {
		t.Fatal("ordinary heartbeat renewed old evidence")
	}
	*now = now.Add(5 * time.Second)
	if getAgent(t, s).Monitor.State != "UNVERIFIED" {
		t.Fatal("same evidence kept green")
	}
	s2, _ := newProtocolServer(t)
	h = freshHeartbeat()
	h.V2.SentAt = protocolNow.Add(-16 * time.Second).Format(time.RFC3339Nano)
	postV2(t, s2, h, 200)
	if getAgent(t, s2).Monitor.State != "UNVERIFIED" {
		t.Fatal("late first packet accepted as fresh")
	}
}
func TestV2ClockProblems(t *testing.T) {
	for _, offset := range []time.Duration{2 * time.Second, -time.Hour} {
		t.Run(offset.String(), func(t *testing.T) {
			s, _ := newProtocolServer(t)
			h := freshHeartbeat()
			h.V2.SentAt = protocolNow.Add(offset).Format(time.RFC3339Nano)
			postV2(t, s, h, 200)
			if getAgent(t, s).Monitor.State != "UNVERIFIED" {
				t.Fatal("clock problem left green")
			}
		})
	}
	s, now := newProtocolServer(t)
	postV2(t, s, freshHeartbeat(), 200)
	*now = now.Add(-time.Second)
	if getAgent(t, s).Monitor.State != "UNVERIFIED" {
		t.Fatal("server clock rollback left green")
	}
}
func TestV2Validation(t *testing.T) {
	cases := []struct {
		name string
		edit func(*TelemetryV2)
	}{
		{"instance_zero", func(v *TelemetryV2) { v.AgentCreated = "0" }},
		{"sequence_float", func(v *TelemetryV2) { v.Sequence = "1.0" }},
		{"overflow", func(v *TelemetryV2) { v.Sequence = "18446744073709551616" }},
		{"timestamp", func(v *TelemetryV2) { v.SentAt = "2026-02-30T00:00:00Z" }},
		{"future_tick", func(v *TelemetryV2) { v.Observation.LastObservedAtMS = "100001" }},
		{"inverted_ticks", func(v *TelemetryV2) { v.Observation.LastReliableVisualAtMS = "99999" }},
		{"unknown_state", func(v *TelemetryV2) { v.Observation.State = "GREEN" }},
		{"wrong_automation", func(v *TelemetryV2) { v.Observation.AutomationState = "BAD" }},
		{"ttl_raise", func(v *TelemetryV2) { v.Observation.EvidenceTTLMS = 60000 }},
		{"null_without_reason", func(v *TelemetryV2) { v.Hardware.CPUPackagePower.Reason = "" }},
		{"wrong_unit", func(v *TelemetryV2) { v.Hardware.GPUPower.Unit = "W" }},
		{"negative_power", func(v *TelemetryV2) { p := -1.0; v.Hardware.GPUPower.Value = &p }},
		{"huge_power", func(v *TelemetryV2) { p := 10001.0; v.Hardware.GPUPower.Value = &p }},
		{"bad_sample_date", func(v *TelemetryV2) { v.Hardware.SampledAt = "bad" }},
		{"future_sample_tick", func(v *TelemetryV2) { v.Hardware.SampledTickMS = "100001" }},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			s, _ := newProtocolServer(t)
			h := freshHeartbeat()
			tc.edit(h.V2)
			postV2(t, s, h, 400)
			if len(s.snapshot().Agents) != 0 {
				t.Fatal("invalid payload mutated storage")
			}
		})
	}
	s, _ := newProtocolServer(t)
	h := freshHeartbeat()
	h.V2.Hardware = nil
	postV2(t, s, h, 200)
	b, _ := json.Marshal(freshHeartbeat())
	b = bytes.Replace(b, []byte(`"sequence":"1"`), []byte(`"sequence":1`), 1)
	if request(s, "POST", "/api/heartbeat", "a", b).Code != 400 {
		t.Fatal("numeric sequence accepted")
	}
	b = bytes.Replace(b, []byte(`"value":40.344`), []byte(`"value":1e999`), 1)
	if request(s, "POST", "/api/heartbeat", "a", b).Code != 400 {
		t.Fatal("nonfinite number accepted")
	}
}
func TestV2UnreliableClaimsAreDowngraded(t *testing.T) {
	edits := []func(*Observation){func(o *Observation) { o.Running = false }, func(o *Observation) { o.GameCreated = "0" }, func(o *Observation) { o.Confidence = "VISUAL" }, func(o *Observation) { o.Source = "NONE" }, func(o *Observation) { o.Source = "DEATHMATCH_QUEUE" }, func(o *Observation) { o.Page = "Menu" }, func(o *Observation) { o.Window.Minimized = true }, func(o *Observation) { o.Window.Occluded = true }, func(o *Observation) { o.Window.Foreground = false }, func(o *Observation) { o.Window.BackgroundInput = true }, func(o *Observation) { o.LastConfirmedAtMS = "99999" }, func(o *Observation) { o.ProcessObservedAtMS = "80000" }}
	for i, edit := range edits {
		t.Run(string(rune('a'+i)), func(t *testing.T) {
			s, _ := newProtocolServer(t)
			h := freshHeartbeat()
			edit(&h.V2.Observation)
			postV2(t, s, h, 200)
			if getAgent(t, s).Monitor.State != "UNVERIFIED" {
				t.Fatal("unreliable claim left green")
			}
		})
	}
}
func TestV2OutOfMatchAndOfflineExpire(t *testing.T) {
	for _, state := range []string{"OUT_OF_MATCH", "OFFLINE"} {
		t.Run(state, func(t *testing.T) {
			s, now := newProtocolServer(t)
			h := freshHeartbeat()
			o := &h.V2.Observation
			o.State = state
			o.Source = "NONE"
			o.LastConfirmedAtMS = "0"
			if state == "OFFLINE" {
				o.GamePID = 0
				o.GameCreated = "0"
				o.Confidence = "PROCESS"
				o.LastReliableVisualAtMS = "0"
				o.Page = "None"
			} else {
				o.Page = "Menu"
				o.Confidence = "VISUAL"
			}
			postV2(t, s, h, 200)
			if getAgent(t, s).Monitor.State != state {
				t.Fatal("fresh negative fact lost")
			}
			*now = now.Add(15 * time.Second)
			if getAgent(t, s).Monitor.State != "UNVERIFIED" {
				t.Fatal("negative fact never expired")
			}
		})
	}
}
func TestV2OrderingInstanceSessionAndStop(t *testing.T) {
	s, _ := newProtocolServer(t)
	h := freshHeartbeat()
	postV2(t, s, h, 200)
	postV2(t, s, h, 409)
	h.V2.Sequence = "2"
	h.V2.SentAtMS = "100001"
	h.V2.Observation.Running = false
	h.V2.Observation.State = "UNVERIFIED"
	h.V2.Observation.Confidence = "NONE"
	h.V2.Observation.SessionID = "session-stop"
	h.V2.Observation.Revision = "2"
	postV2(t, s, h, 200)
	late := freshHeartbeat()
	late.V2.Sequence = "3"
	late.V2.SentAtMS = "100002"
	postV2(t, s, late, 409)
	late.V2.Observation.Revision = "2"
	late.V2.Observation.SessionID = "session-stop"
	postV2(t, s, late, 409)
	h = freshHeartbeat()
	h.V2.AgentPID = 56
	h.V2.AgentCreated = "134000000000000100"
	postV2(t, s, h, 200)
	late = freshHeartbeat()
	late.V2.Sequence = "99"
	postV2(t, s, late, 409)
	h.V2.Sequence = "2"
	h.V2.SentAtMS = "100001"
	h.V2.Observation.SessionID = "replacement-process"
	h.V2.Observation.GamePID = 102
	h.V2.Observation.GameCreated = "134000000000000002"
	postV2(t, s, h, 200)
	h.V2.Sequence = "3"
	h.V2.SentAtMS = "100002"
	h.V2.Observation.SessionID = "session-1"
	postV2(t, s, h, 409)
}
func TestV2V1ReplacementAndPersistedReplayGuard(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	postV2(t, s, h, 200)
	if err := s.save(); err != nil {
		t.Fatal(err)
	}
	restarted := NewServer("a", "v", s.dataDir)
	restarted.now = func() time.Time { return *now }
	if err := restarted.load(); err != nil {
		t.Fatal(err)
	}
	a := getAgent(t, restarted)
	if a.Monitor.State != "UNVERIFIED" || a.Monitor.Reason != "server_restarted" || a.V2.Hardware.GPUPower.Value != nil {
		t.Fatal("restart resurrected persisted evidence")
	}
	postV2(t, restarted, h, 409)
	legacy := h
	legacy.V2 = nil
	postV2(t, restarted, legacy, 200)
	if a = getAgent(t, restarted); a.Monitor.State != "UNVERIFIED" || a.V2 != nil {
		t.Fatal("v1 inherited v2 green")
	}
	if err := restarted.save(); err != nil {
		t.Fatal(err)
	}
	again := NewServer("a", "v", s.dataDir)
	again.now = func() time.Time { return *now }
	if err := again.load(); err != nil {
		t.Fatal(err)
	}
	postV2(t, again, h, 409)
}
func TestV2ReadOnlyHardwareAndEvents(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	postV2(t, s, h, 200)
	before := len(s.snapshot().Events)
	for i := 0; i < 30; i++ {
		_ = getAgent(t, s)
		_ = sseSnapshot(t, s)
	}
	if s.agents[h.Machine].V2.Hardware.Sequence != "9" || len(s.snapshot().Events) != before {
		t.Fatal("read triggered sampling/events")
	}
	*now = now.Add(time.Second)
	h.V2.Sequence = "2"
	h.V2.SentAtMS = "100001"
	h.V2.SentAtMS = "101000"
	h.V2.SentAt = now.Format(time.RFC3339Nano)
	h.V2.Hardware.Sequence = "1"
	h.V2.Hardware.SampledTickMS = "101000"
	h.V2.Hardware.SampledAt = h.V2.SentAt
	postV2(t, s, h, 200)
	if len(s.snapshot().Events) != before || getAgent(t, s).V2.Hardware.GPUPower.Value == nil {
		t.Fatal("bridge restart polluted events/rejected sample")
	}
}
func TestV2ConcurrentSnapshotsAreIndependent(t *testing.T) {
	s, _ := newProtocolServer(t)
	postV2(t, s, freshHeartbeat(), 200)
	var wg sync.WaitGroup
	for i := 0; i < 16; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for j := 0; j < 25; j++ {
				x := s.snapshot()
				x.Agents[0].V2.Observation.LastError = "local"
				*x.Agents[0].V2.Hardware.GPUPower.Value = 999
			}
		}()
	}
	wg.Wait()
	if *getAgent(t, s).V2.Hardware.GPUPower.Value != 40.344 {
		t.Fatal("parallel snapshot modified storage")
	}
}
func TestCPPV2WireContract(t *testing.T) {
	fixture := os.Getenv("OWH_V2_PROTOCOL_FIXTURE")
	if fixture == "" {
		fixture = filepath.Join("..", "build", "windows", "telemetry-v2-tests.exe")
		if _, err := os.Stat(fixture); err != nil {
			t.Skip("compiled C++ v2 fixture unavailable")
		}
	}
	output, err := exec.Command(fixture, "--fixture").Output()
	if err != nil {
		t.Fatal(err)
	}
	s, _ := newProtocolServer(t)
	w := request(s, "POST", "/api/heartbeat", "a", output)
	if w.Code != http.StatusOK {
		t.Fatalf("real C++ v2 rejected: %s", w.Body.String())
	}
	a := getAgent(t, s)
	if a.Monitor.State != "IN_MATCH" || a.V2.Hardware.GPUPower.Value == nil || a.V2.Hardware.CPUPackagePower.Value != nil || a.V2.Hardware.CPUPackagePower.Reason != "cpu_access_denied" {
		t.Fatal("C++ observation/null/source contract mismatch")
	}
	if !reflect.DeepEqual(a, sseSnapshot(t, s).Agents[0]) {
		t.Fatal("C++ GET/SSE mismatch")
	}
	if err := s.save(); err != nil {
		t.Fatal(err)
	}
	restarted := NewServer("a", "v", s.dataDir)
	restarted.now = s.now
	if err := restarted.load(); err != nil {
		t.Fatal(err)
	}
	if getAgent(t, restarted).Monitor.State != "UNVERIFIED" {
		t.Fatal("C++ persisted observation resurrected")
	}
}

func TestV2ProofFirstDeadlineCannotBeRenewed(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	postV2(t, s, h, 200)
	*now = now.Add(10 * time.Second)
	h.V2.Sequence = "2"
	h.V2.SentAt = now.Format(time.RFC3339Nano)
	postV2(t, s, h, 409) // sequence alone cannot renew a frozen send tick
	h.V2.SentAtMS = "100001"
	postV2(t, s, h, 200)
	if getAgent(t, s).Monitor.EvidenceRemainingMS != 5000 || getAgent(t, s).Monitor.HardwareRemainingMS != 5000 {
		t.Fatal("tiny send-tick increment renewed first proof deadline")
	}
	legacy := h
	legacy.V2 = nil
	postV2(t, s, legacy, 200)
	*now = now.Add(6 * time.Second)
	h.V2.Sequence = "3"
	h.V2.SentAtMS = "100002"
	h.V2.SentAt = now.Format(time.RFC3339Nano)
	postV2(t, s, h, 200)
	a := getAgent(t, s)
	if a.Monitor.State != "UNVERIFIED" || a.V2.Hardware.GPUPower.Value != nil {
		t.Fatal("v1 replacement discarded first-proof expiry")
	}
}
func TestV2RestartRequiresNewProof(t *testing.T) {
	s, now := newProtocolServer(t)
	h := freshHeartbeat()
	postV2(t, s, h, 200)
	if err := s.save(); err != nil {
		t.Fatal(err)
	}
	restarted := NewServer("a", "v", s.dataDir)
	restarted.now = func() time.Time { return *now }
	if err := restarted.load(); err != nil {
		t.Fatal(err)
	}
	h.V2.Sequence = "2"
	h.V2.SentAtMS = "100001"
	postV2(t, restarted, h, 200)
	if getAgent(t, restarted).Monitor.State != "UNVERIFIED" {
		t.Fatal("same persisted proof regained green")
	}
	h.V2.Sequence = "3"
	h.V2.SentAtMS = "100002"
	o := &h.V2.Observation
	o.LastConfirmedAtMS = "100002"
	o.LastReliableVisualAtMS = "100002"
	o.LastObservedAtMS = "100002"
	o.ProcessObservedAtMS = "100002"
	postV2(t, restarted, h, 200)
	if getAgent(t, restarted).Monitor.State != "IN_MATCH" {
		t.Fatal("new post-restart proof failed to recover")
	}
}
func TestV2RecoverySessionCountIsNotALifetimeLimit(t *testing.T) {
	s, _ := newProtocolServer(t)
	first := freshHeartbeat()
	postV2(t, s, first, 200)
	for i := 2; i <= 300; i++ {
		h := freshHeartbeat()
		h.V2.Sequence = strconv.Itoa(i)
		h.V2.SentAtMS = strconv.Itoa(100000 + i)
		o := &h.V2.Observation
		o.SessionID = fmt.Sprintf("recovery-%d", i)
		o.ProcessObservedAtMS = h.V2.SentAtMS
		postV2(t, s, h, 200)
	}
	late := first
	late.V2.Sequence = "301"
	late.V2.SentAtMS = "100301"
	postV2(t, s, late, 409)
	postV2(t, s, first, 409)
	if len(s.guards[first.Machine].Retired) > maxSessionsPerRevision {
		t.Fatal("retired session memory not bounded")
	}
}
func TestV2AutomationSnapshotsMayDiffer(t *testing.T) {
	s, _ := newProtocolServer(t)
	h := freshHeartbeat()
	h.V2.Observation.AutomationState = "STANDBY"
	postV2(t, s, h, 200)
	if getAgent(t, s).Monitor.State != "IN_MATCH" {
		t.Fatal("independent valid automation snapshots rejected observer evidence")
	}
}
