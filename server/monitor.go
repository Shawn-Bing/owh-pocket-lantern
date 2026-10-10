package main

import (
	"fmt"
	"math"
	"strconv"
	"strings"
	"time"
)

const (
	maxEvidenceTTL         = uint64(15000)
	maxProcessTTL          = uint64(15000)
	maxHardwareTTL         = uint64(300000)
	maxClockLead           = time.Second
	maxSessionsPerRevision = 256
)

type TelemetryV2 struct {
	AgentPID     uint32      `json:"agent_pid"`
	AgentCreated string      `json:"agent_created"`
	Sequence     string      `json:"sequence"`
	SentAt       string      `json:"sent_at"`
	SentAtMS     string      `json:"sent_at_ms"`
	Observation  Observation `json:"observation"`
	Hardware     *Hardware   `json:"hardware,omitempty"`
}
type Observation struct {
	SessionID              string      `json:"session_id"`
	Revision               string      `json:"revision"`
	Mode                   string      `json:"mode"`
	AutomationState        string      `json:"automation_state"`
	Running                bool        `json:"running"`
	GamePID                uint32      `json:"game_pid"`
	GameCreated            string      `json:"game_created"`
	State                  string      `json:"state"`
	Confidence             string      `json:"confidence"`
	Source                 string      `json:"source"`
	LastConfirmedAtMS      string      `json:"last_confirmed_at_ms"`
	LastObservedAtMS       string      `json:"last_observed_at_ms"`
	LastReliableVisualAtMS string      `json:"last_reliable_visual_at_ms"`
	ProcessObservedAtMS    string      `json:"process_observed_at_ms"`
	Page                   string      `json:"page"`
	Window                 MatchWindow `json:"window"`
	LastError              string      `json:"last_error"`
	EvidenceTTLMS          uint64      `json:"evidence_ttl_ms"`
	ProcessTTLMS           uint64      `json:"process_ttl_ms"`
}
type MatchWindow struct {
	Minimized       bool `json:"minimized"`
	Occluded        bool `json:"occluded"`
	Foreground      bool `json:"foreground"`
	BackgroundInput bool `json:"background_input"`
}
type Reading struct {
	Value  *float64 `json:"value"`
	Unit   string   `json:"unit"`
	Source string   `json:"source"`
	Sensor string   `json:"sensor"`
	Reason string   `json:"reason"`
}
type Hardware struct {
	SampledAt             string  `json:"sampled_at"`
	SampledTickMS         string  `json:"sampled_tick_ms"`
	Sequence              string  `json:"sequence"`
	IntervalMS            uint64  `json:"interval_ms"`
	StaleAfterMS          uint64  `json:"stale_after_ms"`
	CPUPackageTemperature Reading `json:"cpu_package_temperature"`
	CPUPackagePower       Reading `json:"cpu_package_power"`
	GPUPower              Reading `json:"gpu_power"`
}

// Derived only on read. No input, sampler, or mutable cached pointers are exposed.
type MonitorView struct {
	State               string `json:"state"`
	Reason              string `json:"reason"`
	EvidenceRemainingMS uint64 `json:"evidence_remaining_ms"`
	ProcessRemainingMS  uint64 `json:"process_remaining_ms"`
	HardwareRemainingMS uint64 `json:"hardware_remaining_ms"`
	ClockTrusted        bool   `json:"clock_trusted"`
	HostOnline          bool   `json:"host_online"`
}
type receipt struct {
	at                                                                   time.Time
	trusted                                                              bool
	transportMS                                                          uint64
	visualDeadline, confirmedDeadline, processDeadline, hardwareDeadline time.Time
}
type protocolGuard struct {
	AgentPID          uint32          `json:"agent_pid"`
	AgentCreated      string          `json:"agent_created"`
	Sequence          string          `json:"sequence"`
	SentTick          string          `json:"sent_tick"`
	Revision          string          `json:"revision"`
	SessionID         string          `json:"session_id"`
	Running           bool            `json:"running"`
	Observed          string          `json:"observed"`
	ProcessObserved   string          `json:"process_observed"`
	Retired           map[string]bool `json:"retired,omitempty"`
	VisualProof       string          `json:"visual_proof,omitempty"`
	ConfirmedProof    string          `json:"confirmed_proof,omitempty"`
	HardwareProof     string          `json:"hardware_proof,omitempty"`
	visualDeadline    time.Time       `json:"-"`
	confirmedDeadline time.Time       `json:"-"`
	processDeadline   time.Time       `json:"-"`
	hardwareDeadline  time.Time       `json:"-"`
}

func decimal(v string) (uint64, bool) {
	if v == "" || len(v) > 20 || (len(v) > 1 && v[0] == '0') {
		return 0, false
	}
	for _, r := range v {
		if r < '0' || r > '9' {
			return 0, false
		}
	}
	n, e := strconv.ParseUint(v, 10, 64)
	return n, e == nil
}
func number(v string) uint64 {
	n, ok := decimal(v)
	if !ok {
		return 0
	}
	return n
}
func utc(v string) (time.Time, bool) {
	t, e := time.Parse(time.RFC3339Nano, v)
	_, offset := t.Zone()
	return t, e == nil && offset == 0 && !t.IsZero()
}
func oneOf(v string, names string) bool {
	for _, n := range strings.Fields(names) {
		if v == n {
			return true
		}
	}
	return false
}
func validReading(r Reading, unit string) bool {
	if r.Unit != unit || len(r.Source) > 256 || len(r.Sensor) > 256 || len(r.Reason) > 512 {
		return false
	}
	if r.Value == nil {
		return r.Reason != ""
	}
	n := *r.Value
	if math.IsNaN(n) || math.IsInf(n, 0) || r.Source == "" || r.Sensor == "" || r.Reason != "" {
		return false
	}
	if unit == "celsius" {
		return n >= -100 && n <= 200
	}
	return n >= 0 && n <= 10000
}
func validV2(v *TelemetryV2) bool {
	if v == nil {
		return true
	}
	if v.AgentPID == 0 || number(v.AgentCreated) == 0 || number(v.Sequence) == 0 {
		return false
	}
	if _, ok := utc(v.SentAt); !ok {
		return false
	}
	sent, ok := decimal(v.SentAtMS)
	if !ok {
		return false
	}
	o := v.Observation
	revision, ok := decimal(o.Revision)
	if !ok {
		return false
	}
	if len(o.SessionID) > 256 || (o.Running && (o.SessionID == "" || revision == 0)) || len(o.LastError) > 2000 {
		return false
	}
	if !oneOf(o.Mode, "CUSTOM_ROOM DEATHMATCH") || !states[o.AutomationState] {
		return false
	}
	if !oneOf(o.State, "IN_MATCH OUT_OF_MATCH UNVERIFIED OFFLINE") || !oneOf(o.Confidence, "NONE PROCESS VISUAL REPEATED_VISUAL") || !oneOf(o.Source, "NONE TARGET_TITLE TARGET_JOIN EXPLICIT_START DEATHMATCH_QUEUE DEATHMATCH_SCOREBOARD") {
		return false
	}
	if !oneOf(o.Page, "None LoginError Home Play More Browser Join Launch Room Menu Login LauncherSelect BrowserSearch Queue Hero Combat Results Leave ConfirmLeave Respawn DeathmatchStart Endorse RoomList Joining") {
		return false
	}
	created, ok := decimal(o.GameCreated)
	if !ok || (o.GamePID == 0 && created != 0) {
		return false
	}
	for _, tick := range []string{o.LastConfirmedAtMS, o.LastObservedAtMS, o.LastReliableVisualAtMS, o.ProcessObservedAtMS} {
		n, ok := decimal(tick)
		if !ok || n > sent {
			return false
		}
	}
	if number(o.LastConfirmedAtMS) > number(o.LastReliableVisualAtMS) || number(o.LastReliableVisualAtMS) > number(o.LastObservedAtMS) {
		return false
	}
	if o.EvidenceTTLMS == 0 || o.ProcessTTLMS == 0 || o.EvidenceTTLMS > maxEvidenceTTL || o.ProcessTTLMS > maxProcessTTL {
		return false
	}
	if h := v.Hardware; h != nil {
		if _, ok := utc(h.SampledAt); !ok {
			return false
		}
		sample, ok := decimal(h.SampledTickMS)
		if !ok || sample == 0 || sample > sent || number(h.Sequence) == 0 {
			return false
		}
		if h.IntervalMS < 1000 || h.IntervalMS > 100000 || h.StaleAfterMS == 0 || h.StaleAfterMS > maxHardwareTTL || h.StaleAfterMS > h.IntervalMS*3 {
			return false
		}
		if !validReading(h.CPUPackageTemperature, "celsius") || !validReading(h.CPUPackagePower, "watts") || !validReading(h.GPUPower, "watts") {
			return false
		}
	}
	return true
}
func newReceipt(v *TelemetryV2, now time.Time) *receipt {
	r := &receipt{at: now, trusted: true}
	if v == nil {
		return r
	}
	sent, _ := utc(v.SentAt)
	if sent.After(now.Add(maxClockLead)) {
		r.trusted = false
		return r
	}
	if sent.After(now) {
		r.transportMS = uint64(maxClockLead / time.Millisecond)
	} else {
		r.transportMS = uint64(now.Sub(sent) / time.Millisecond)
	}
	return r
}

// Preserve anti-replay watermarks across v1 replacement and server restarts.
func nextGuard(old protocolGuard, v *TelemetryV2) (protocolGuard, error) {
	if v == nil {
		return old, nil
	}
	created := number(v.AgentCreated)
	previous := number(old.AgentCreated)
	if created < previous || (created == previous && old.AgentPID != 0 && old.AgentPID != v.AgentPID) {
		return old, fmt.Errorf("old_agent_instance")
	}
	o := v.Observation
	if created == previous && old.AgentPID != 0 {
		if number(v.Sequence) <= number(old.Sequence) || number(v.SentAtMS) <= number(old.SentTick) {
			return old, fmt.Errorf("out_of_order")
		}
		if number(o.Revision) < number(old.Revision) {
			return old, fmt.Errorf("old_session")
		}
		if number(o.Revision) == number(old.Revision) {
			if number(o.ProcessObservedAtMS) < number(old.ProcessObserved) {
				return old, fmt.Errorf("old_process_observation")
			}
			if number(o.LastReliableVisualAtMS) < number(old.VisualProof) && o.SessionID == old.SessionID {
				return old, fmt.Errorf("old_visual_observation")
			}
			if number(o.LastConfirmedAtMS) < number(old.ConfirmedProof) && o.SessionID == old.SessionID {
				return old, fmt.Errorf("old_confirmation")
			}
			if !old.Running && o.Running {
				return old, fmt.Errorf("stopped_session")
			}
			if o.SessionID == old.SessionID {
				if number(o.LastObservedAtMS) < number(old.Observed) || number(o.ProcessObservedAtMS) < number(old.ProcessObserved) {
					return old, fmt.Errorf("old_observation")
				}
			} else {
				if old.Retired[o.SessionID] {
					return old, fmt.Errorf("retired_session")
				}
				retired := make(map[string]bool, len(old.Retired)+1)
				for k, b := range old.Retired {
					retired[k] = b
				}
				if len(retired) >= maxSessionsPerRevision {
					for id := range retired {
						delete(retired, id)
						break
					}
				}
				retired[old.SessionID] = true
				old.Retired = retired
			}
		} else {
			old.Retired = nil
		}
	} else {
		old.Retired = nil
	}
	next := old
	next.AgentPID = v.AgentPID
	next.AgentCreated = v.AgentCreated
	next.Sequence = v.Sequence
	next.SentTick = v.SentAtMS
	next.Revision = o.Revision
	next.SessionID = o.SessionID
	next.Running = o.Running
	next.Observed = o.LastObservedAtMS
	next.ProcessObserved = o.ProcessObservedAtMS
	return next, nil
}
func cloneReading(r Reading) Reading {
	if r.Value != nil {
		v := *r.Value
		r.Value = &v
	}
	return r
}
func cloneAgent(a Agent) Agent {
	if a.V2 != nil {
		v := *a.V2
		a.V2 = &v
		if v.Hardware != nil {
			h := *v.Hardware
			h.CPUPackageTemperature = cloneReading(h.CPUPackageTemperature)
			h.CPUPackagePower = cloneReading(h.CPUPackagePower)
			h.GPUPower = cloneReading(h.GPUPower)
			a.V2.Hardware = &h
		}
	}
	a.Monitor = nil
	if a.receipt != nil {
		copied := *a.receipt
		a.receipt = &copied
	}
	return a
}
func remaining(sent, tick, ttl, transport, elapsed uint64) uint64 {
	if tick == 0 || tick > sent {
		return 0
	}
	age := sent - tick
	if age >= ttl || transport >= ttl-age {
		return 0
	}
	age += transport
	if elapsed >= ttl-age {
		return 0
	}
	return ttl - age - elapsed
}
func deriveAgent(cached Agent, now time.Time) Agent {
	a := cloneAgent(cached)
	seenAt := a.Seen
	if a.receipt != nil {
		seenAt = a.receipt.at
	}
	age := now.Sub(seenAt)
	m := &MonitorView{State: "UNVERIFIED", Reason: "legacy_protocol", HostOnline: age >= 0 && age <= 90*time.Second}
	a.Monitor = m
	if a.V2 == nil {
		return a
	}
	v := a.V2
	o := &v.Observation
	r := cached.receipt
	fail := func(reason string) {
		m.State = "UNVERIFIED"
		m.Reason = reason
		o.State = "UNVERIFIED"
		o.Confidence = "NONE"
		o.LastError = reason
	}
	if r == nil {
		fail("server_restarted")
	} else if !r.trusted || now.Before(r.at) {
		fail("clock_untrusted")
	} else {
		m.ClockTrusted = true
		elapsed := uint64(now.Sub(r.at) / time.Millisecond)
		sent := number(v.SentAtMS)
		m.EvidenceRemainingMS = remaining(sent, number(o.LastReliableVisualAtMS), o.EvidenceTTLMS, r.transportMS, elapsed)
		m.ProcessRemainingMS = remaining(sent, number(o.ProcessObservedAtMS), o.ProcessTTLMS, r.transportMS, elapsed)
		m.EvidenceRemainingMS = capDeadline(m.EvidenceRemainingMS, r.visualDeadline, now)
		m.ProcessRemainingMS = capDeadline(m.ProcessRemainingMS, r.processDeadline, now)
		m.State = o.State
		m.Reason = o.LastError
		switch o.State {
		case "IN_MATCH":
			confirmed := capDeadline(remaining(sent, number(o.LastConfirmedAtMS), o.EvidenceTTLMS, r.transportMS, elapsed), r.confirmedDeadline, now)
			if !o.Running || o.GamePID == 0 || number(o.GameCreated) == 0 || o.Confidence != "REPEATED_VISUAL" || !sourceMatchesMode(o.Source, o.Mode) || (o.Mode == "CUSTOM_ROOM" && o.Page != "Room") || (o.Mode == "DEATHMATCH" && o.Page != "Combat") || o.Window.Minimized || o.Window.Occluded || !o.Window.Foreground || o.Window.BackgroundInput || number(o.LastConfirmedAtMS) != number(o.LastReliableVisualAtMS) || confirmed == 0 || m.EvidenceRemainingMS == 0 || m.ProcessRemainingMS == 0 {
				fail("match_evidence_unavailable")
			} else if confirmed < m.EvidenceRemainingMS {
				m.EvidenceRemainingMS = confirmed
			}
		case "OUT_OF_MATCH":
			if !o.Running || o.GamePID == 0 || number(o.GameCreated) == 0 || !oneOf(o.Confidence, "VISUAL REPEATED_VISUAL") || oneOf(o.Page, "None Combat Hero Respawn Joining") || m.EvidenceRemainingMS == 0 || m.ProcessRemainingMS == 0 || o.Window.Minimized || o.Window.Occluded || !o.Window.Foreground || o.Window.BackgroundInput {
				fail("visual_evidence_expired")
			}
		case "OFFLINE":
			if !o.Running || o.GamePID != 0 || number(o.GameCreated) != 0 || o.Confidence != "PROCESS" || m.ProcessRemainingMS == 0 {
				fail("process_evidence_expired")
			}
		case "UNVERIFIED":
		}
	}
	if !m.HostOnline {
		fail("host_offline")
	}
	if h := v.Hardware; h != nil {
		if r != nil && r.trusted && !now.Before(r.at) {
			sampleUTC, _ := utc(h.SampledAt)
			sendUTC, _ := utc(v.SentAt)
			if !sampleUTC.After(sendUTC.Add(maxClockLead)) {
				m.HardwareRemainingMS = remaining(number(v.SentAtMS), number(h.SampledTickMS), h.StaleAfterMS, r.transportMS, uint64(now.Sub(r.at)/time.Millisecond))
				m.HardwareRemainingMS = capDeadline(m.HardwareRemainingMS, r.hardwareDeadline, now)
				wallAge := now.Sub(sampleUTC)
				if wallAge < -maxClockLead || wallAge >= time.Duration(h.StaleAfterMS)*time.Millisecond {
					m.HardwareRemainingMS = 0
				}
			}
		}
		if m.HardwareRemainingMS == 0 || !m.HostOnline {
			for _, reading := range []*Reading{&h.CPUPackageTemperature, &h.CPUPackagePower, &h.GPUPower} {
				reading.Value = nil
				reading.Reason = "sample_expired"
			}
		}
	}
	return a
}

type persistedState struct {
	Snapshot
	Guards map[string]protocolGuard `json:"protocol_guards,omitempty"`
}

func validGuard(g protocolGuard) bool {
	if g.AgentPID == 0 {
		return g.AgentCreated == "" && g.Sequence == "" && g.SessionID == ""
	}
	if number(g.AgentCreated) == 0 || number(g.Sequence) == 0 || len(g.SessionID) > 256 || len(g.Retired) > maxSessionsPerRevision {
		return false
	}
	for _, v := range []string{g.SentTick, g.Revision, g.Observed, g.ProcessObserved} {
		if _, ok := decimal(v); !ok {
			return false
		}
	}
	for _, v := range []string{g.VisualProof, g.ConfirmedProof, g.HardwareProof} {
		if v != "" {
			if n, ok := decimal(v); !ok || n > number(g.SentTick) {
				return false
			}
		}
	}
	if number(g.Observed) > number(g.SentTick) || number(g.ProcessObserved) > number(g.SentTick) || (g.Running && (number(g.Revision) == 0 || g.SessionID == "")) {
		return false
	}
	for id, retired := range g.Retired {
		if len(id) > 256 || !retired {
			return false
		}
	}
	return true
}
func sourceMatchesMode(source, mode string) bool {
	if source == "EXPLICIT_START" {
		return true
	}
	if mode == "CUSTOM_ROOM" {
		return source == "TARGET_TITLE" || source == "TARGET_JOIN"
	}
	return source == "DEATHMATCH_QUEUE" || source == "DEATHMATCH_SCOREBOARD"
}

func capDeadline(base uint64, deadline, now time.Time) uint64 {
	if deadline.IsZero() || !deadline.After(now) {
		return 0
	}
	remaining := uint64(deadline.Sub(now) / time.Millisecond)
	if remaining < base {
		return remaining
	}
	return base
}
func proofDeadline(proof, oldProof string, same bool, oldDeadline time.Time, base uint64, now time.Time) time.Time {
	if base == 0 {
		return time.Time{}
	}
	candidate := now.Add(time.Duration(base) * time.Millisecond)
	if same && proof == oldProof {
		// A zero prior deadline means persisted/untrusted/expired evidence. A new
		// packet cannot authenticate that same fact again after restart or v1 cover.
		if oldDeadline.IsZero() {
			return time.Time{}
		}
		if oldDeadline.Before(candidate) {
			return oldDeadline
		}
	}
	return candidate
}
func anchoredReceipt(old protocolGuard, next *protocolGuard, v *TelemetryV2, now time.Time) *receipt {
	r := newReceipt(v, now)
	if v == nil {
		return r
	}
	sameInstance := old.AgentPID == v.AgentPID && old.AgentCreated == v.AgentCreated

	o := v.Observation
	sent := number(v.SentAtMS)
	base := func(tick string, ttl uint64) uint64 {
		if !r.trusted {
			return 0
		}
		return remaining(sent, number(tick), ttl, r.transportMS, 0)
	}
	r.visualDeadline = proofDeadline(o.LastReliableVisualAtMS, old.VisualProof, sameInstance, old.visualDeadline, base(o.LastReliableVisualAtMS, o.EvidenceTTLMS), now)
	r.confirmedDeadline = proofDeadline(o.LastConfirmedAtMS, old.ConfirmedProof, sameInstance, old.confirmedDeadline, base(o.LastConfirmedAtMS, o.EvidenceTTLMS), now)
	r.processDeadline = proofDeadline(o.ProcessObservedAtMS, old.ProcessObserved, sameInstance, old.processDeadline, base(o.ProcessObservedAtMS, o.ProcessTTLMS), now)
	next.VisualProof = o.LastReliableVisualAtMS
	next.ConfirmedProof = o.LastConfirmedAtMS
	next.visualDeadline = r.visualDeadline
	next.confirmedDeadline = r.confirmedDeadline
	next.processDeadline = r.processDeadline
	if h := v.Hardware; h != nil {
		r.hardwareDeadline = proofDeadline(h.SampledTickMS, old.HardwareProof, sameInstance, old.hardwareDeadline, base(h.SampledTickMS, h.StaleAfterMS), now)
		if sameInstance && number(h.SampledTickMS) < number(old.HardwareProof) {
			r.hardwareDeadline = time.Time{}
		} else {
			next.HardwareProof = h.SampledTickMS
			next.hardwareDeadline = r.hardwareDeadline
		}
	}
	return r
}
