package main

import (
	"bytes"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"os"
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
