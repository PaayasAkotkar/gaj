package main

import (
	"net/http"
)

func f(wr http.ResponseWriter, req *http.Request) {
	_, err := wr.Write([]byte("suceed send 🤗"))
	if err != nil {
		panic(err)
	}
}

func main() {
	http.HandleFunc("/test", f)
	http.ListenAndServe(":8080", nil)
}
