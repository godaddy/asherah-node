package main

import "C"
import (
	"fmt"
	"runtime"
	"sync"
	"time"
)

// Working simple function
//export SimpleWork
func SimpleWork() int {
	fmt.Println("SimpleWork: Starting")
	time.Sleep(10 * time.Millisecond) 
	fmt.Println("SimpleWork: Done")
	return 42
}

// Test with goroutines (like Asherah might use)
//export GoroutineWork  
func GoroutineWork() int {
	fmt.Println("GoroutineWork: Starting")
	
	var wg sync.WaitGroup
	wg.Add(1)
	
	go func() {
		defer wg.Done()
		fmt.Println("GoroutineWork: In goroutine")
		time.Sleep(10 * time.Millisecond)
		fmt.Println("GoroutineWork: Goroutine done")
	}()
	
	fmt.Println("GoroutineWork: Waiting...")
	wg.Wait()
	fmt.Println("GoroutineWork: All done")
	return 43
}

// Test with channels
//export ChannelWork
func ChannelWork() int {
	fmt.Println("ChannelWork: Starting")
	
	ch := make(chan int, 1)
	
	go func() {
		fmt.Println("ChannelWork: Sending")
		ch <- 123
	}()
	
	fmt.Println("ChannelWork: Receiving")
	result := <-ch
	fmt.Printf("ChannelWork: Got %d\n", result)
	return result
}

// Test with runtime operations
//export RuntimeWork
func RuntimeWork() int {
	fmt.Println("RuntimeWork: Starting")
	
	// Force GC
	runtime.GC()
	fmt.Println("RuntimeWork: GC done")
	
	// Check goroutines
	numGoroutines := runtime.NumGoroutine()
	fmt.Printf("RuntimeWork: %d goroutines\n", numGoroutines)
	
	return numGoroutines
}

func main() {
	// Empty main for library
}