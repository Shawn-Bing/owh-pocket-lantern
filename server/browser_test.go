package main

import (
	"os/exec"
	"testing"
)

func TestBrowserFreshness(t *testing.T) {
	node, err := exec.LookPath("node")
	if err != nil {
		t.Skip("Node.js unavailable; run node browser_test.cjs for browser freshness tests")
	}
	output, err := exec.Command(node, "browser_test.cjs").CombinedOutput()
	if err != nil {
		t.Fatalf("browser freshness failed: %v\n%s", err, output)
	}
	t.Log(string(output))
}
