package main

import "C"

// Simple function to warm up the Go runtime
//export WarmupGoRuntime
func WarmupGoRuntime() int32 {
	// This function does minimal work but ensures Go runtime is initialized
	return 1
}

func main() {
	// Empty main for library
}