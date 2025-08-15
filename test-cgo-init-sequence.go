package main

/*
#include <stdio.h>
#include <pthread.h>

static void check_thread() {
    pthread_t tid = pthread_self();
    fprintf(stderr, "CGO thread check: %p\n", (void*)tid);
}

static void init_callback() {
    fprintf(stderr, "CGO init callback called\n");
}
*/
import "C"
import (
	"fmt"
	"runtime"
	"time"
)

func init() {
	fmt.Println("Go init() called")
	C.check_thread()
}

//export TestInit
func TestInit() int {
	fmt.Println("TestInit: Starting")
	C.check_thread()
	return 1
}

//export TestInitWithGoroutine
func TestInitWithGoroutine() int {
	fmt.Println("TestInitWithGoroutine: Starting")
	
	// Start a goroutine that needs CGO
	done := make(chan bool)
	go func() {
		fmt.Println("In goroutine")
		C.check_thread()
		done <- true
	}()
	
	select {
	case <-done:
		fmt.Println("Goroutine completed")
		return 2
	case <-time.After(100 * time.Millisecond):
		fmt.Println("Goroutine timed out")
		return -1
	}
}

//export TestRuntimeInit
func TestRuntimeInit() int {
	fmt.Println("TestRuntimeInit: Starting")
	
	// Force runtime initialization
	runtime.GOMAXPROCS(runtime.GOMAXPROCS(0))
	fmt.Printf("GOMAXPROCS: %d\n", runtime.GOMAXPROCS(0))
	
	// Try to trigger CGO initialization
	C.init_callback()
	
	return 3
}

func main() {
	// Empty main for library
}