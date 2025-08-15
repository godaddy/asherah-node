package main

import "C"
import (
	"fmt"
	"reflect"
	"runtime"
	"unsafe"
	
	"github.com/godaddy/asherah/go/appencryption"
	"github.com/godaddy/asherah/go/appencryption/pkg/crypto"
	"github.com/godaddy/asherah/go/appencryption/pkg/kms"
	"github.com/godaddy/asherah/go/appencryption/pkg/persistence"
)

// Simple function that works
//export SimpleTest
func SimpleTest() int {
	fmt.Println("SimpleTest called")
	return 42
}

// Function that imports Asherah but doesn't use it
//export ImportOnly
func ImportOnly() int {
	fmt.Println("ImportOnly called")
	// Just importing should be fine
	return 43
}

// Function that creates basic Asherah types
//export CreateTypes
func CreateTypes() int {
	fmt.Println("CreateTypes called - creating basic types")
	
	// Try creating just the basic types without initialization
	var config appencryption.Config
	config.ServiceName = "test"
	config.ProductID = "test"
	
	fmt.Printf("Config created: %+v\n", config)
	return 44
}

// Function that tries minimal Asherah setup
//export MinimalSetup
func MinimalSetup() int {
	fmt.Println("MinimalSetup called - attempting minimal setup")
	
	// Try the most basic configuration
	config := &appencryption.Config{
		ServiceName: "test",
		ProductID:   "test",
		KMS:         crypto.NewStatic(),
		Metastore:   persistence.NewMemory(),
	}
	
	fmt.Printf("Basic config created: %+v\n", config)
	
	// This is where it probably hangs - creating the session factory
	fmt.Println("About to create SessionFactory...")
	sessionFactory := appencryption.NewSessionFactory(config)
	if sessionFactory == nil {
		return -1
	}
	
	fmt.Println("SessionFactory created successfully!")
	return 45
}

// Log runtime state before Asherah operations
//export LogRuntimeState  
func LogRuntimeState() int {
	fmt.Println("=== Runtime State Analysis ===")
	fmt.Printf("NumGoroutine: %d\n", runtime.NumGoroutine())
	fmt.Printf("GOMAXPROCS: %d\n", runtime.GOMAXPROCS(0))
	
	// Check if we're in a CGO call
	fmt.Printf("Pointer size: %d\n", unsafe.Sizeof(uintptr(0)))
	
	// Memory stats
	var m runtime.MemStats
	runtime.ReadMemStats(&m)
	fmt.Printf("Alloc: %d KB\n", m.Alloc/1024)
	fmt.Printf("Sys: %d KB\n", m.Sys/1024)
	fmt.Printf("NumGC: %d\n", m.NumGC)
	
	// Check types 
	configType := reflect.TypeOf((*appencryption.Config)(nil)).Elem()
	fmt.Printf("Config type: %v\n", configType)
	
	return 46
}

func main() {
	// Empty main for library
}