package main

import (
	"context"
	"crypto/rand"
	"crypto/subtle"
	"embed"
	"encoding/base64"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io"
	"io/fs"
	"log"
	"math"
	"net/http"
	"os"
	"os/signal"
	"path/filepath"
	"sort"
	"strings"
	"sync"
	"syscall"
	"time"
	"unicode"
)

//go:embed web/*
var webFiles embed.FS

type Heartbeat struct {
	Machine  string  `json:"machine"`
	State    string  `json:"state"`
	Detail   string  `json:"detail"`
	PID      uint32  `json:"pid"`
	Uptime   uint64  `json:"uptime"`
	Restarts uint32  `json:"restarts"`
	Score    float64 `json:"score"`
	DryRun   bool    `json:"dry_run"`
	Version  string  `json:"version"`
}
type Agent struct {
	Heartbeat
	Seen time.Time `json:"seen"`
}
type Event struct {
	Machine string    `json:"machine"`
	State   string    `json:"state"`
	Detail  string    `json:"detail"`
	At      time.Time `json:"at"`
}
type Snapshot struct {
	Agents       []Agent   `json:"agents"`
	Events       []Event   `json:"events"`
	Now          time.Time `json:"now"`
	OfflineAfter int       `json:"offline_after_seconds"`
}
type Server struct {
	mu                             sync.Mutex
	agents                         map[string]Agent
	events                         []Event
	subscribers                    map[chan struct{}]struct{}
	agentToken, viewToken, dataDir string
	dirty                          bool
	now                            func() time.Time
}

var states = map[string]bool{}

func init() {
	for _, s := range strings.Fields("STOPPED DETECTING DRY_RUN ACTIVE BACKGROUND_INPUT WAITING_DESKTOP INPUT_BLOCKED RECOVERING NEEDS_USER BACKOFF STARTING WINDOW_HUNG GAME_NOT_RUNNING NEEDS_CONFIG WAITING_WINDOW WAITING_FOCUS CAPTURE_FAILED NEEDS_CALIBRATION AMBIGUOUS UNKNOWN CONFIRMING LOGIN_STUCK NAVIGATING") {
		states[s] = true
	}
}
func NewServer(agentToken, viewToken, dir string) *Server {
	return &Server{agents: map[string]Agent{}, subscribers: map[chan struct{}]struct{}{}, agentToken: agentToken, viewToken: viewToken, dataDir: dir, now: time.Now}
}
func (s *Server) snapshot() Snapshot { s.mu.Lock(); defer s.mu.Unlock(); return s.snapshotLocked() }
func (s *Server) snapshotLocked() Snapshot {
	x := Snapshot{Agents: make([]Agent, 0, len(s.agents)), Events: append([]Event{}, s.events...), Now: s.now().UTC(), OfflineAfter: 90}
	for _, a := range s.agents {
		x.Agents = append(x.Agents, a)
	}
	sort.Slice(x.Agents, func(i, j int) bool { return x.Agents[i].Machine < x.Agents[j].Machine })
	return x
}
func reply(w http.ResponseWriter, code int, v any) {
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.WriteHeader(code)
	_ = json.NewEncoder(w).Encode(v)
}
func (s *Server) auth(token string, next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		a := r.Header.Get("Authorization")
		expected := "Bearer " + token
		if token == "" || subtle.ConstantTimeCompare([]byte(a), []byte(expected)) != 1 {
			reply(w, 401, map[string]string{"error": "invalid_token"})
			return
		}
		next(w, r)
	}
}
func validHeartbeat(h Heartbeat) bool {
	if len(h.Machine) < 1 || len(h.Machine) > 64 || len(h.Detail) > 2000 || len(h.Version) > 32 || !states[h.State] || math.IsNaN(h.Score) || math.IsInf(h.Score, 0) || h.Score < 0 || h.Score > 1 {
		return false
	}
	for _, r := range h.Machine {
		if unicode.IsControl(r) || r == '/' || r == '\\' {
			return false
		}
	}
	return true
}
func (s *Server) handleHeartbeat(w http.ResponseWriter, r *http.Request) {
	if r.Method != "POST" {
		w.Header().Set("Allow", "POST")
		reply(w, 405, map[string]string{"error": "method"})
		return
	}
	r.Body = http.MaxBytesReader(w, r.Body, 16*1024)
	defer r.Body.Close()
	d := json.NewDecoder(r.Body)
	d.DisallowUnknownFields()
	var h Heartbeat
	if d.Decode(&h) != nil || !validHeartbeat(h) {
		reply(w, 400, map[string]string{"error": "invalid_heartbeat"})
		return
	}
	var tail any
	if d.Decode(&tail) != io.EOF {
		reply(w, 400, map[string]string{"error": "invalid_json"})
		return
	}
	s.mu.Lock()
	previous, exists := s.agents[h.Machine]
	if !exists && len(s.agents) >= 128 {
		s.mu.Unlock()
		reply(w, 429, map[string]string{"error": "agent_limit"})
		return
	}
	now := s.now().UTC()
	s.agents[h.Machine] = Agent{Heartbeat: h, Seen: now}
	if !exists || previous.State != h.State || previous.Detail != h.Detail || previous.DryRun != h.DryRun {
		s.events = append(s.events, Event{Machine: h.Machine, State: h.State, Detail: h.Detail, At: now})
		if len(s.events) > 200 {
			s.events = append([]Event{}, s.events[len(s.events)-200:]...)
		}
	}
	s.dirty = true
	for c := range s.subscribers {
		select {
		case c <- struct{}{}:
		default:
		}
	}
	s.mu.Unlock()
	reply(w, 200, map[string]bool{"ok": true})
}
func (s *Server) handleStatus(w http.ResponseWriter, r *http.Request) {
	if r.Method != "GET" {
		reply(w, 405, map[string]string{"error": "method"})
		return
	}
	reply(w, 200, s.snapshot())
}
func (s *Server) handleEvents(w http.ResponseWriter, r *http.Request) {
	if r.Method != "GET" {
		reply(w, 405, map[string]string{"error": "method"})
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		reply(w, 500, map[string]string{"error": "stream_unavailable"})
		return
	}
	c := make(chan struct{}, 1)
	s.mu.Lock()
	if len(s.subscribers) >= 32 {
		s.mu.Unlock()
		reply(w, 429, map[string]string{"error": "viewer_limit"})
		return
	}
	s.subscribers[c] = struct{}{}
	s.mu.Unlock()
	defer func() { s.mu.Lock(); delete(s.subscribers, c); s.mu.Unlock() }()
	w.Header().Set("Content-Type", "text/event-stream")
	w.Header().Set("X-Accel-Buffering", "no")
	w.Header().Set("Cache-Control", "no-store")
	send := func() bool {
		b, err := json.Marshal(s.snapshot())
		if err != nil {
			return false
		}
		_ = http.NewResponseController(w).SetWriteDeadline(time.Now().Add(10 * time.Second))
		_, err = fmt.Fprintf(w, "event: snapshot\ndata: %s\n\n", b)
		if err != nil {
			return false
		}
		flusher.Flush()
		return true
	}
	if !send() {
		return
	}
	tick := time.NewTicker(15 * time.Second)
	defer tick.Stop()
	for {
		select {
		case <-r.Context().Done():
			return
		case <-c:
			if !send() {
				return
			}
		case <-tick.C:
			if !send() {
				return
			}
		}
	}
}
func (s *Server) handler() http.Handler {
	mux := http.NewServeMux()
	mux.HandleFunc("/api/heartbeat", s.auth(s.agentToken, s.handleHeartbeat))
	mux.HandleFunc("/api/status", s.auth(s.viewToken, s.handleStatus))
	mux.HandleFunc("/api/events", s.auth(s.viewToken, s.handleEvents))
	mux.HandleFunc("/healthz", func(w http.ResponseWriter, r *http.Request) {
		reply(w, 200, map[string]string{"status": "ok", "version": "0.2.0"})
	})
	files, _ := fs.Sub(webFiles, "web")
	static := http.FileServer(http.FS(files))
	mux.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		if r.URL.Path != "/" && r.URL.Path != "/app.js" && r.URL.Path != "/style.css" {
			http.NotFound(w, r)
			return
		}
		static.ServeHTTP(w, r)
	})
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("X-Content-Type-Options", "nosniff")
		w.Header().Set("Referrer-Policy", "no-referrer")
		w.Header().Set("Cache-Control", "no-store")
		w.Header().Set("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self'; img-src 'self' data:; connect-src 'self'; frame-ancestors 'none'; base-uri 'none'; form-action 'self'")
		mux.ServeHTTP(w, r)
	})
}
func (s *Server) save() error {
	s.mu.Lock()
	if !s.dirty {
		s.mu.Unlock()
		return nil
	}
	snapshot := s.snapshotLocked()
	s.dirty = false
	s.mu.Unlock()
	b, err := json.MarshalIndent(snapshot, "", "  ")
	if err == nil {
		var f *os.File
		f, err = os.CreateTemp(s.dataDir, ".state-*.tmp")
		if err == nil {
			name := f.Name()
			defer os.Remove(name)
			if err = f.Chmod(0600); err == nil {
				_, err = f.Write(b)
			}
			if err == nil {
				err = f.Sync()
			}
			closeErr := f.Close()
			if err == nil {
				err = closeErr
			}
			if err == nil {
				err = os.Rename(name, filepath.Join(s.dataDir, "state.json"))
			}
		}
	}
	if err != nil {
		s.mu.Lock()
		s.dirty = true
		s.mu.Unlock()
	}
	return err
}
func (s *Server) load() error {
	b, err := os.ReadFile(filepath.Join(s.dataDir, "state.json"))
	if errors.Is(err, os.ErrNotExist) {
		return nil
	}
	if err != nil {
		return err
	}
	if len(b) > 2*1024*1024 {
		return fmt.Errorf("state file too large")
	}
	var x Snapshot
	if err = json.Unmarshal(b, &x); err != nil {
		return err
	}
	if len(x.Agents) > 128 || len(x.Events) > 200 {
		return fmt.Errorf("state limits exceeded")
	}
	for _, a := range x.Agents {
		if !validHeartbeat(a.Heartbeat) {
			return fmt.Errorf("invalid persisted agent")
		}
		s.agents[a.Machine] = a
	}
	s.events = x.Events
	return nil
}
func token(dir, name, env string) (string, error) {
	if t := os.Getenv(env); t != "" {
		if len(t) < 32 || len(t) > 512 || strings.ContainsAny(t, "\r\n") {
			return "", fmt.Errorf("%s must have 32–512 characters", env)
		}
		return t, nil
	}
	path := filepath.Join(dir, name)
	b, err := os.ReadFile(path)
	if err == nil {
		v := strings.TrimSpace(string(b))
		if len(v) < 32 || len(v) > 512 || strings.ContainsAny(v, "\r\n") {
			return "", fmt.Errorf("invalid token file %s", path)
		}
		return v, nil
	}
	if !errors.Is(err, os.ErrNotExist) {
		return "", err
	}
	raw := make([]byte, 32)
	if _, err = rand.Read(raw); err != nil {
		return "", err
	}
	t := base64.RawURLEncoding.EncodeToString(raw)
	err = os.WriteFile(path, []byte(t+"\n"), 0600)
	return t, err
}
func main() {
	listen := flag.String("listen", "127.0.0.1:8765", "HTTP listen address; use a TLS reverse proxy for remote access")
	dir := flag.String("data", "owh-server-data", "state and token directory")
	flag.Parse()
	if err := os.MkdirAll(*dir, 0700); err != nil {
		log.Fatal(err)
	}
	a, err := token(*dir, "agent-token.txt", "OWH_AGENT_TOKEN")
	if err != nil {
		log.Fatal(err)
	}
	v, err := token(*dir, "view-token.txt", "OWH_VIEW_TOKEN")
	if err != nil {
		log.Fatal(err)
	}
	if subtle.ConstantTimeCompare([]byte(a), []byte(v)) == 1 {
		log.Fatal("agent and viewer tokens must differ")
	}
	s := NewServer(a, v, *dir)
	if err = s.load(); err != nil {
		log.Fatalf("cannot load saved state: %v (keep a backup before repairing)", err)
	}
	ctx, cancel := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer cancel()
	httpServer := &http.Server{Addr: *listen, Handler: s.handler(), ReadHeaderTimeout: 5 * time.Second, ReadTimeout: 10 * time.Second, IdleTimeout: 60 * time.Second, MaxHeaderBytes: 16 * 1024}
	done := make(chan struct{})
	go func() {
		defer close(done)
		ticker := time.NewTicker(10 * time.Second)
		defer ticker.Stop()
		for {
			select {
			case <-ctx.Done():
				return
			case <-ticker.C:
				if err := s.save(); err != nil {
					log.Printf("save failed: %v", err)
				}
			}
		}
	}()
	shutdownDone := make(chan struct{})
	go func() {
		defer close(shutdownDone)
		<-ctx.Done()
		c, stop := context.WithTimeout(context.Background(), 5*time.Second)
		defer stop()
		if httpServer.Shutdown(c) != nil {
			_ = httpServer.Close()
		}
	}()
	log.Printf("OverwatchHelper dashboard: http://%s", *listen)
	log.Printf("Read agent-token.txt for the Windows agent and view-token.txt for the browser in: %s", *dir)
	err = httpServer.ListenAndServe()
	cancel()
	<-shutdownDone
	<-done
	if saveErr := s.save(); saveErr != nil {
		log.Printf("final save failed: %v", saveErr)
	}
	if err != nil && !errors.Is(err, http.ErrServerClosed) {
		log.Fatal(err)
	}
}
