package main

import "C"

//export MinimalStub
func MinimalStub() C.int {
	// Absolutely minimal - just return a value
	// No imports, no complex operations, just basic Go runtime
	return 42
}

func main() {}