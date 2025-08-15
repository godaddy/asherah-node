package main

/*
#include <stdint.h>
*/
import "C"
import (
	"encoding/json"
	"fmt"
	"unsafe"
)

// Simple warmup function that MUST be called first
//export WarmupGoRuntime
func WarmupGoRuntime() C.int32_t {
	// Just existing is enough to initialize the Go runtime
	// But we can do a minimal operation to be sure
	_ = fmt.Sprintf("warmup")
	return 1
}

// The actual SetupJson that would normally hang if called first
//export SetupJson
func SetupJson(configPtr unsafe.Pointer) C.int32_t {
	if configPtr == nil {
		return -1
	}
	
	// Read config from Cobhan buffer
	length := *(*int32)(configPtr)
	configBytes := (*[1 << 30]byte)(unsafe.Pointer(uintptr(configPtr) + 8))[:length:length]
	
	var config map[string]interface{}
	if err := json.Unmarshal(configBytes, &config); err != nil {
		fmt.Printf("SetupJson: Failed to parse config: %v\n", err)
		return -1
	}
	
	fmt.Printf("SetupJson: Config parsed: %v\n", config)
	
	// Simulate Asherah's complex initialization
	fmt.Println("asherah-cobhan: \"*** WARNING WARNING WARNING USING MEMORY METASTORE - THIS IS FOR TEST/DEBUG ONLY ***\"")
	fmt.Println("asherah-cobhan: \"*** WARNING WARNING WARNING USING STATIC MASTER KEY - THIS IS FOR TEST/DEBUG ONLY ***\"")
	
	return 0
}

func main() {
	// Empty main for library
}