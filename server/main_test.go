package main

import (
	"bytes"
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"os"
	"os/exec"
	"regexp"
	"sort"
	"strings"
	"testing"
	"time"
)

func request(s *Server, method, path, token string, body []byte) *httptest.ResponseRecorder {
	r := httptest.NewRequest(method, path, bytes.NewReader(body))
	if token != "" {
		r.Header.Set("Authorization", "Bearer "+token)
	}
	w := httptest.NewRecorder()
	s.handler().ServeHTTP(w, r)
	return w
}
func TestAuthAndValidation(t *testing.T) {
	s := NewServer("agent-secret", "view-secret", t.TempDir())
	h := Heartbeat{Machine: "pc-01", State: "ACTIVE", Detail: "目标房间", Score: .95, Version: "0.1.0"}
	b, _ := json.Marshal(h)
	for _, x := range []struct {
		method, path, token string
		body                []byte
		want                int
	}{{"POST", "/api/heartbeat", "", b, 401}, {"POST", "/api/heartbeat", "view-secret", b, 401}, {"GET", "/api/status", "agent-secret", nil, 401}, {"POST", "/api/heartbeat", "agent-secret", b, 200}, {"GET", "/api/status", "view-secret", nil, 200}, {"GET", "/api/heartbeat", "agent-secret", nil, 405}, {"POST", "/api/heartbeat", "agent-secret", []byte(`{"machine":"x","state":"BAD"}`), 400}, {"POST", "/api/heartbeat", "agent-secret", append(b, []byte(`{}`)...), 400}, {"POST", "/api/heartbeat", "agent-secret", []byte(strings.Repeat("x", 17000)), 400}} {
		if w := request(s, x.method, x.path, x.token, x.body); w.Code != x.want {
			t.Fatalf("%s %s: %d != %d", x.method, x.path, w.Code, x.want)
		}
	}
	if len(s.snapshot().Agents) != 1 {
		t.Fatal("invalid data mutated storage")
	}
}
func TestServerTimestampAndPersistence(t *testing.T) {
	dir := t.TempDir()
	s := NewServer("a", "v", dir)
	now := time.Date(2026, 9, 17, 12, 0, 0, 0, time.UTC)
	s.now = func() time.Time { return now }
	h := Heartbeat{Machine: "电脑-1", State: "ACTIVE", Score: 1}
	b, _ := json.Marshal(h)
	request(s, "POST", "/api/heartbeat", "a", b)
	if !s.snapshot().Agents[0].Seen.Equal(now) {
		t.Fatal("receipt time must come from server")
	}
	if err := s.save(); err != nil {
		t.Fatal(err)
	}
	s2 := NewServer("a", "v", dir)
	if err := s2.load(); err != nil {
		t.Fatal(err)
	}
	if len(s2.snapshot().Agents) != 1 {
		t.Fatal("restart lost agent")
	}
	request(s, "POST", "/api/heartbeat", "a", b)
	if len(s.snapshot().Events) != 1 {
		t.Fatal("unchanged heartbeat should not grow events")
	}
	now = now.Add(2 * time.Minute)
	if s.snapshot().Now.Sub(s.snapshot().Agents[0].Seen) <= 90*time.Second {
		t.Fatal("stale heartbeat age")
	}
}
func TestBackgroundAndDesktopStates(t *testing.T) {
	s := NewServer("a", "v", t.TempDir())
	for _, state := range []string{"BACKGROUND_INPUT", "BACKGROUND_UNVERIFIED", "BACKGROUND_INPUT_BLOCKED", "WAITING_DESKTOP"} {
		body, _ := json.Marshal(Heartbeat{Machine: "pc", State: state, Version: "0.2.0"})
		if response := request(s, "POST", "/api/heartbeat", "a", body); response.Code != 200 {
			t.Fatalf("new client state %s rejected: %d", state, response.Code)
		}
		if s.snapshot().Agents[0].State != state {
			t.Fatalf("state %s was not delivered to dashboard", state)
		}
	}
}
func TestBoundedEventsAndEscaping(t *testing.T) {
	s := NewServer("a", "v", t.TempDir())
	for i := 0; i < 260; i++ {
		h := Heartbeat{Machine: "<script>alert(1)</script>", State: "ACTIVE", Detail: strings.Repeat("x", i)}
		b, _ := json.Marshal(h)
		request(s, "POST", "/api/heartbeat", "a", b)
	}
	if len(s.snapshot().Agents) != 0 {
		t.Fatal("invalid slash in machine")
	}
	for i := 0; i < 260; i++ {
		h := Heartbeat{Machine: "pc", State: "ACTIVE", Detail: strings.Repeat("x", i)}
		b, _ := json.Marshal(h)
		request(s, "POST", "/api/heartbeat", "a", b)
	}
	if len(s.snapshot().Events) != 200 {
		t.Fatal("event cap not enforced")
	}
}
func TestStaticHeadersAndUnknownRoutes(t *testing.T) {
	s := NewServer("a", "v", t.TempDir())
	w := request(s, "GET", "/", "", nil)
	if w.Code != 200 || w.Header().Get("Content-Security-Policy") == "" {
		t.Fatal("missing UI/CSP")
	}
	if request(s, "GET", "/api/nope", "", nil).Code != 404 {
		t.Fatal("unknown API leaked UI")
	}
}
func TestEventStream(t *testing.T) {
	s := NewServer("a", "v", t.TempDir())
	ts := httptest.NewServer(s.handler())
	defer ts.Close()
	r, _ := http.NewRequest("GET", ts.URL+"/api/events", nil)
	r.Header.Set("Authorization", "Bearer v")
	resp, err := ts.Client().Do(r)
	if err != nil {
		t.Fatal(err)
	}
	defer resp.Body.Close()
	buf := make([]byte, 256)
	n, err := resp.Body.Read(buf)
	if err != nil {
		t.Fatal(err)
	}
	if !strings.Contains(string(buf[:n]), "event: snapshot") {
		t.Fatal("missing initial SSE snapshot")
	}
}
func TestCorruptStateFailsClosed(t *testing.T) {
	dir := t.TempDir()
	_ = os.WriteFile(dir+"/state.json", []byte("broken"), 0600)
	if NewServer("a", "v", dir).load() == nil {
		t.Fatal("corrupt state silently accepted")
	}
}

// Pin every pre-P0 server wire name: the shared manifest is additive for v0.5.13.
func TestLegacyAutomationStatesRemainAccepted(t *testing.T) {
	for _, state := range strings.Fields("STOPPED DETECTING DRY_RUN ACTIVE BACKGROUND_INPUT BACKGROUND_UNVERIFIED BACKGROUND_INPUT_BLOCKED WAITING_DESKTOP INPUT_BLOCKED RECOVERING NEEDS_USER BACKOFF STARTING WINDOW_HUNG GAME_NOT_RUNNING NEEDS_CONFIG WAITING_WINDOW WAITING_FOCUS CAPTURE_FAILED NEEDS_CALIBRATION AMBIGUOUS UNKNOWN CONFIRMING LOGIN_STUCK NAVIGATING STANDBY MATCHMAKING WAITING_PAGE WAITING_GAME_EXIT IDLE NEAC_RETRY") {
		if !validHeartbeat(Heartbeat{Machine: "legacy", State: state}) {
			t.Errorf("legacy state %s was removed", state)
		}
	}
	for _, state := range []string{"", "active", "ACTIVE ", "IN_MATCH", "NOT_A_STATE"} {
		if validHeartbeat(Heartbeat{Machine: "legacy", State: state}) {
			t.Errorf("invalid automation state %q was accepted", state)
		}
	}
}

func TestAutomationManifestRejectsMalformedEntries(t *testing.T) {
	for _, manifest := range []string{"", "OWH_AUTOMATION_STATE()", "OWH_AUTOMATION_STATE(bad)", "ACTIVE", "OWH_AUTOMATION_STATE(A) extra", "OWH_AUTOMATION_STATE(A)\nOWH_AUTOMATION_STATE(A)"} {
		t.Run(manifest, func(t *testing.T) {
			defer func() {
				if recover() == nil {
					t.Fatal("malformed shared manifest did not fail closed")
				}
			}()
			parseAutomationStates(manifest)
		})
	}
}

// This runs under CTest with the real compiled C++ enum/name serializer. Adding
// a client state without server acceptance, or emitting a different wire name,
// breaks the build-windows verification instead of silently losing heartbeats.
func TestAutomationStateWireContract(t *testing.T) {
	fixture := os.Getenv("OWH_PROTOCOL_FIXTURE")
	if fixture == "" {
		t.Skip("run CTest/build-windows.ps1 for the compiled C++ to Go contract")
	}
	output, err := exec.Command(fixture, "--wire-states").Output()
	if err != nil {
		t.Fatal(err)
	}
	names := strings.Fields(string(output))
	seen := make(map[string]bool)
	for _, name := range names {
		if seen[name] {
			t.Fatalf("duplicate client wire state %s", name)
		}
		seen[name] = true
		if !states[name] {
			t.Errorf("C++ emits %s but Go rejects it", name)
		}
	}
	if len(seen) != len(states) {
		t.Fatalf("client/server states differ: client=%d server=%d", len(seen), len(states))
	}
	testAutomationStateRoundTrips(t, names)
}

func TestAllAutomationStatesRoundTrip(t *testing.T) {
	names := make([]string, 0, len(states))
	for state := range states {
		names = append(names, state)
	}
	sort.Strings(names)
	testAutomationStateRoundTrips(t, names)
}

func testAutomationStateRoundTrips(t *testing.T, names []string) {
	t.Helper()
	for _, state := range names {
		t.Run(state, func(t *testing.T) {
			dir := t.TempDir()
			s := NewServer("a", "v", dir)
			h := Heartbeat{Machine: "pc", State: state, Detail: "协议回归", Version: "0.5.13"}
			body, err := json.Marshal(h)
			if err != nil {
				t.Fatal(err)
			}
			if w := request(s, "POST", "/api/heartbeat", "a", body); w.Code != http.StatusOK {
				t.Fatalf("heartbeat %s rejected: %d %s", state, w.Code, w.Body.String())
			}
			var status Snapshot
			w := request(s, "GET", "/api/status", "v", nil)
			if w.Code != http.StatusOK || json.Unmarshal(w.Body.Bytes(), &status) != nil {
				t.Fatal("status response failed")
			}
			assertAutomationStateSnapshot(t, status, state)

			// Cancel after the first complete SSE frame, without a sleep or socket.
			ctx, cancel := context.WithCancel(context.Background())
			defer cancel()
			r := httptest.NewRequest("GET", "/api/events", nil).WithContext(ctx)
			r.Header.Set("Authorization", "Bearer v")
			stream := &cancelOnFlushRecorder{ResponseRecorder: httptest.NewRecorder(), cancel: cancel}
			s.handler().ServeHTTP(stream, r)
			if stream.Code != http.StatusOK {
				t.Fatal("SSE response failed")
			}
			data := strings.TrimSuffix(strings.TrimPrefix(stream.Body.String(), "event: snapshot\ndata: "), "\n\n")
			var pushed Snapshot
			if err := json.Unmarshal([]byte(data), &pushed); err != nil {
				t.Fatalf("SSE decode failed: %v", err)
			}
			assertAutomationStateSnapshot(t, pushed, state)
			if err := s.save(); err != nil {
				t.Fatal(err)
			}
			reloaded := NewServer("a", "v", dir)
			if err := reloaded.load(); err != nil {
				t.Fatal(err)
			}
			assertAutomationStateSnapshot(t, reloaded.snapshot(), state)
		})
	}
}

type cancelOnFlushRecorder struct {
	*httptest.ResponseRecorder
	cancel context.CancelFunc
}

func (w *cancelOnFlushRecorder) Flush() {
	w.ResponseRecorder.Flush()
	w.cancel()
}

func assertAutomationStateSnapshot(t *testing.T, snapshot Snapshot, state string) {
	t.Helper()
	if len(snapshot.Agents) != 1 || snapshot.Agents[0].State != state ||
		len(snapshot.Events) != 1 || snapshot.Events[0].State != state {
		t.Fatalf("state %s lost in snapshot: %+v", state, snapshot)
	}
}

func TestDashboardLabelsCoverAutomationStates(t *testing.T) {
	script, err := webFiles.ReadFile("web/app.js")
	if err != nil {
		t.Fatal(err)
	}
	labels := make(map[string]bool)
	for _, match := range regexp.MustCompile(`\b([A-Z_]+):"`).FindAllSubmatch(script, -1) {
		labels[string(match[1])] = true
	}
	for state := range states {
		if !labels[state] {
			t.Errorf("dashboard needs a label for automation state %s", state)
		}
	}
}
