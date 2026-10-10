package main

import (
	"crypto/rand"
	"encoding/base64"
	"time"
)

// Alert state is independent of the game observer. Only fresh accepted v2
// heartbeats can establish expected running; runtime timers are never restored.
type AlertView struct {
	Enabled        bool   `json:"enabled"`
	EpisodeID      string `json:"episode_id,omitempty"`
	Kind           string `json:"kind"`
	Reason         string `json:"reason"`
	DurationMS     uint64 `json:"duration_ms"`
	TypeDurationMS uint64 `json:"type_duration_ms"`
	ThresholdMS    uint64 `json:"threshold_ms"`
	Reached        bool   `json:"reached"`
}
type alertEpisode struct {
	agentPID                                uint32
	created, revision                       string
	since, typeSince, healthyUntil, lastNow time.Time
	kind, reason, id                        string
	reached                                 bool
}

func alertID() string {
	var b [16]byte
	if _, err := rand.Read(b[:]); err != nil {
		panic("cannot create alert episode identity")
	}
	return base64.RawURLEncoding.EncodeToString(b[:])
}
func earliest(a, b time.Time) time.Time {
	if a.IsZero() || (!b.IsZero() && b.Before(a)) {
		return b
	}
	return a
}
func matchDeadline(a Agent) time.Time {
	r := a.receipt
	if r == nil {
		return time.Time{}
	}
	return earliest(earliest(r.visualDeadline, r.confirmedDeadline), r.processDeadline)
}
func (s *Server) acceptAlertHeartbeatLocked(a Agent, now time.Time) {
	old, exists := s.alerts[a.Machine]
	v := a.V2
	if v == nil || !v.Observation.Running || a.DryRun || a.receipt == nil {
		if exists {
			s.closeAlertLocked(a, old, "expectation_cleared", now)
			delete(s.alerts, a.Machine)
		}
		return
	}
	if !a.receipt.trusted {
		if exists && old.agentPID == v.AgentPID && old.created == v.AgentCreated && old.revision == v.Observation.Revision {
			// A clock failure is not a user stop; an existing same-generation running
			// expectation continues, with observation conservatively unverified.
			s.refreshAlertLocked(a, now)
		} else if exists {
			s.closeAlertLocked(a, old, "untrusted_running_generation", now)
			delete(s.alerts, a.Machine)
		}
		return
	}
	if !exists || old.agentPID != v.AgentPID || old.created != v.AgentCreated || old.revision != v.Observation.Revision || now.Before(old.lastNow) {
		if exists {
			s.closeAlertLocked(a, old, "running_generation_changed", now)
		}
		old = alertEpisode{agentPID: v.AgentPID, created: v.AgentCreated, revision: v.Observation.Revision, kind: "NONE", lastNow: now}
	}
	// game PID/session changes within the same running revision preserve the
	// continuous abnormal interval. Only genuine IN_MATCH ends it.
	s.alerts[a.Machine] = old
	s.refreshAlertLocked(a, now)
}
func (s *Server) alertEventLocked(a Agent, e alertEpisode, event, reason string, now time.Time) {
	s.events = append(s.events, Event{Machine: a.Machine, State: a.State, Detail: reason, At: now.UTC(), MatchState: deriveAgent(a, now).Monitor.State, AlertEvent: event, AlertKind: e.kind, AlertID: e.id})
	if len(s.events) > 200 {
		s.events = append([]Event{}, s.events[len(s.events)-200:]...)
	}
	s.dirty = true
	for c := range s.subscribers {
		select {
		case c <- struct{}{}:
		default:
		}
	}
}
func (s *Server) closeAlertLocked(a Agent, e alertEpisode, reason string, now time.Time) {
	if !e.since.IsZero() {
		s.alertEventLocked(a, e, "resolved", reason, now)
	}
}
func (s *Server) refreshAlertLocked(a Agent, now time.Time) {
	e, exists := s.alerts[a.Machine]
	if !exists {
		return
	}
	if now.Before(e.lastNow) {
		s.closeAlertLocked(a, e, "clock_reset", now)
		delete(s.alerts, a.Machine)
		return
	}
	e.lastNow = now
	view := deriveAgent(a, now).Monitor
	kind, reason := "UNVERIFIED", "match_unverified"
	if !view.HostOnline {
		kind, reason = "OFFLINE", "heartbeat_missing"
	} else {
		switch view.State {
		case "IN_MATCH":
			s.closeAlertLocked(a, e, "verified_in_match", now)
			e.since = time.Time{}
			e.typeSince = time.Time{}
			e.id = ""
			e.kind = "NONE"
			e.reason = ""
			e.reached = false
			e.healthyUntil = matchDeadline(a)
			s.alerts[a.Machine] = e
			return
		case "OUT_OF_MATCH":
			kind, reason = "OUT_OF_MATCH", "visually_out_of_match"
		case "OFFLINE":
			kind, reason = "OUT_OF_MATCH", "game_process_absent"
		}
	}
	boundary := now
	if kind == "OFFLINE" {
		boundary = a.Seen.Add(90 * time.Second)
		if a.receipt != nil {
			boundary = a.receipt.at.Add(90 * time.Second)
		}
	}
	typeBoundary := boundary
	if e.since.IsZero() {
		if !e.healthyUntil.IsZero() && e.healthyUntil.Before(boundary) {
			boundary = e.healthyUntil
		}
		e.since = boundary
		e.typeSince = typeBoundary
		e.kind = kind
		e.reason = reason
		e.id = alertID()
		e.reached = false
		s.alertEventLocked(a, e, "started", reason, now)
	} else if e.kind != kind || e.reason != reason {
		// Never backdate a current type across a mixed unknown/out-of-match episode.
		e.typeSince = boundary
		if e.typeSince.Before(e.since) {
			e.typeSince = e.since
		}
		e.kind = kind
		e.reason = reason
		s.alertEventLocked(a, e, "kind_changed", reason, now)
	}
	if !e.reached && now.Sub(e.since) >= s.alertAfter {
		e.reached = true
		s.alertEventLocked(a, e, "threshold_reached", e.reason, now)
	}
	s.alerts[a.Machine] = e
}
func (s *Server) refreshAlertsLocked(now time.Time) {
	for machine := range s.alerts {
		if a, ok := s.agents[machine]; ok {
			s.refreshAlertLocked(a, now)
		}
	}
}
func (s *Server) alertViewLocked(a Agent, now time.Time) *AlertView {
	v := &AlertView{Kind: "NONE", Reason: "awaiting_fresh_running_heartbeat", ThresholdMS: uint64(s.alertAfter / time.Millisecond)}
	e, ok := s.alerts[a.Machine]
	if !ok {
		if a.V2 == nil {
			v.Reason = "legacy_protocol"
		} else if a.DryRun {
			v.Reason = "dry_run"
		} else if !a.V2.Observation.Running {
			v.Reason = "assistant_stopped"
		}
		return v
	}
	v.Enabled = true
	v.EpisodeID = e.id
	v.Kind = e.kind
	v.Reason = e.reason
	v.Reached = e.reached
	if !e.since.IsZero() && !now.Before(e.since) {
		v.DurationMS = uint64(now.Sub(e.since) / time.Millisecond)
	}
	if !e.typeSince.IsZero() && !now.Before(e.typeSince) {
		v.TypeDurationMS = uint64(now.Sub(e.typeSince) / time.Millisecond)
	}
	return v
}
