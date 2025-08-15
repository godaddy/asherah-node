package main

import "C"

//export Warmup
func Warmup() C.int {
	return 1
}

func main() {}