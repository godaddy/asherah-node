package main

/*
#include <stdint.h>
*/
import "C"
import "fmt"

//export WarmupGoRuntime
func WarmupGoRuntime() C.int32_t {
	// Simple operation to warm up Go runtime
	_ = fmt.Sprintf("warmup")
	return 1
}

func main() {
	// This is a library, not an executable
}