package main

import "C"
import (
	"fmt"
	"encoding/json"
	"crypto/rand"
	"time"
)

// Level 1: Ultra simple - just return a number
//export Level1Simple
func Level1Simple() int {
	return 42
}

// Level 2: Simple with fmt
//export Level2Fmt
func Level2Fmt() int {
	fmt.Println("Level 2: Using fmt")
	return 42
}

// Level 3: JSON encoding
//export Level3Json
func Level3Json() int {
	fmt.Println("Level 3: Using JSON")
	data := map[string]string{"test": "value"}
	_, err := json.Marshal(data)
	if err != nil {
		return -1
	}
	return 42
}

// Level 4: Crypto
//export Level4Crypto
func Level4Crypto() int {
	fmt.Println("Level 4: Using crypto")
	buffer := make([]byte, 16)
	_, err := rand.Read(buffer)
	if err != nil {
		return -1
	}
	return 42
}

// Level 5: Time operations
//export Level5Time
func Level5Time() int {
	fmt.Println("Level 5: Using time")
	now := time.Now()
	fmt.Printf("Current time: %v\n", now)
	return 42
}

// Level 6: Memory allocation
//export Level6Memory
func Level6Memory() int {
	fmt.Println("Level 6: Memory allocation")
	data := make([]byte, 1024*1024) // 1MB
	data[0] = 42
	return int(data[0])
}

func main() {
	// Empty main for library
}