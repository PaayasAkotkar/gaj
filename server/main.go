package main

import (
	"log"
	"net/http"
)

func f(wr http.ResponseWriter, req *http.Request) {
	log.Println("Incoming request path:", req.URL.Path)
	if req.Method != http.MethodGet {
		http.Error(wr, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}
	wr.Header().Set("Content-Type", "text/plain; charset=utf-8")
	_, err := wr.Write([]byte("suceed send 🤗"))
	log.Println("message 🤗")
	if err != nil {
		panic(err)
	}
}

func main() {
	http.HandleFunc("/get/", f)

	log.Println("Server running on :1080...")
	if err := http.ListenAndServe("127.0.0.1:1080", nil); err != nil {
		log.Fatal(err)
	}
}
