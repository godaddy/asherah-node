package main

import "C"
import "fmt"

//export TestInit
func TestInit() {
    fmt.Println("Go runtime initialized successfully")
}

func main() {
    // Empty main for library
}