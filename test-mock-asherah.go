package main

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// Mimic Cobhan buffer structure
typedef struct {
    int32_t length;
    int32_t reserved;
    char data[];
} CobhanBuffer;

static void log_call(const char* func) {
    pthread_t tid = pthread_self();
    fprintf(stderr, "%s called on thread %p\n", func, (void*)tid);
}

static const char* read_cobhan_string(void* buffer) {
    if (buffer == NULL) return NULL;
    CobhanBuffer* cb = (CobhanBuffer*)buffer;
    return cb->data;
}
*/
import "C"
import (
	"encoding/json"
	"fmt"
	"os"
	"runtime"
	"sync"
	"time"
	"unsafe"
)

var setupDone bool
var setupMutex sync.Mutex

// Mock SetEnv - this is what hangs in real Asherah!
//export SetEnv
func SetEnv(envJson unsafe.Pointer) int32 {
	C.log_call(C.CString("SetEnv"))
	
	// Try to read the buffer
	if envJson == nil {
		fmt.Println("SetEnv: nil buffer")
		return -1
	}
	
	// This is where real Asherah might do something complex
	fmt.Println("SetEnv: Reading environment JSON")
	
	// Simulate environment variable setting
	envStr := C.GoString(C.read_cobhan_string(envJson))
	fmt.Printf("SetEnv: Got env string: %s\n", envStr)
	
	// Parse JSON and set env vars
	var envMap map[string]string
	if err := json.Unmarshal([]byte(envStr), &envMap); err != nil {
		fmt.Printf("SetEnv: Failed to parse JSON: %v\n", err)
		return -1
	}
	
	for k, v := range envMap {
		os.Setenv(k, v)
		fmt.Printf("SetEnv: Set %s=%s\n", k, v)
	}
	
	return 0
}

// Mock SetupJson - progressively add complexity
//export SetupJson
func SetupJson(configJson unsafe.Pointer) int32 {
	C.log_call(C.CString("SetupJson"))
	
	setupMutex.Lock()
	defer setupMutex.Unlock()
	
	if setupDone {
		fmt.Println("SetupJson: Already initialized")
		return -101
	}
	
	fmt.Println("SetupJson: Starting initialization")
	
	// Step 1: Basic runtime operations
	fmt.Printf("SetupJson: GOMAXPROCS=%d\n", runtime.GOMAXPROCS(0))
	
	// Step 2: Create some goroutines
	fmt.Println("SetupJson: Starting background workers")
	var wg sync.WaitGroup
	for i := 0; i < 3; i++ {
		wg.Add(1)
		go func(id int) {
			defer wg.Done()
			fmt.Printf("SetupJson: Worker %d started\n", id)
			time.Sleep(10 * time.Millisecond)
			fmt.Printf("SetupJson: Worker %d done\n", id)
		}(i)
	}
	
	// Step 3: Wait for workers
	fmt.Println("SetupJson: Waiting for workers")
	wg.Wait()
	
	// Step 4: Parse config
	if configJson != nil {
		configStr := C.GoString(C.read_cobhan_string(configJson))
		fmt.Printf("SetupJson: Config: %s\n", configStr)
		
		var config map[string]interface{}
		if err := json.Unmarshal([]byte(configStr), &config); err != nil {
			fmt.Printf("SetupJson: Failed to parse config: %v\n", err)
			return -105
		}
	}
	
	setupDone = true
	fmt.Println("SetupJson: Initialization complete")
	return 0
}

// Simplified versions for testing
//export SimpleSetupJson
func SimpleSetupJson(configJson unsafe.Pointer) int32 {
	C.log_call(C.CString("SimpleSetupJson"))
	fmt.Println("SimpleSetupJson: Just return success")
	return 0
}

//export ComplexSetupJson
func ComplexSetupJson(configJson unsafe.Pointer) int32 {
	C.log_call(C.CString("ComplexSetupJson"))
	
	// Do everything real SetupJson does but add more
	result := SetupJson(configJson)
	
	// Additional complex operations
	fmt.Println("ComplexSetupJson: Doing additional work")
	
	// Force GC
	runtime.GC()
	fmt.Println("ComplexSetupJson: GC complete")
	
	// Create channels
	ch := make(chan int, 10)
	go func() {
		for i := 0; i < 10; i++ {
			ch <- i
		}
		close(ch)
	}()
	
	sum := 0
	for v := range ch {
		sum += v
	}
	fmt.Printf("ComplexSetupJson: Channel sum=%d\n", sum)
	
	return result
}

//export Shutdown
func Shutdown() {
	C.log_call(C.CString("Shutdown"))
	setupMutex.Lock()
	defer setupMutex.Unlock()
	setupDone = false
	fmt.Println("Shutdown: Complete")
}

func main() {
	// Empty main for library
}