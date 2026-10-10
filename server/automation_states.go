package main

import (
	_ "embed"
	"fmt"
	"strings"
)

// The C++ enum and server validator consume the same wire-name manifest.
//
//go:embed protocol/automation_states.inc
var automationStateManifest string

var states = parseAutomationStates(automationStateManifest)

func parseAutomationStates(manifest string) map[string]bool {
	result := make(map[string]bool)
	for line, raw := range strings.Split(manifest, "\n") {
		entry := strings.TrimSpace(raw)
		if entry == "" || strings.HasPrefix(entry, "//") {
			continue
		}
		name, ok := strings.CutPrefix(entry, "OWH_AUTOMATION_STATE(")
		if !ok || !strings.HasSuffix(name, ")") {
			panic(fmt.Sprintf("invalid automation state manifest line %d", line+1))
		}
		name = strings.TrimSuffix(name, ")")
		if name == "" || result[name] {
			panic(fmt.Sprintf("empty or duplicate automation state on line %d", line+1))
		}
		for _, character := range name {
			if (character < 'A' || character > 'Z') && character != '_' {
				panic(fmt.Sprintf("invalid automation state on line %d", line+1))
			}
		}
		result[name] = true
	}
	if len(result) == 0 {
		panic("empty automation state manifest")
	}
	return result
}
