package main

import (
	"bufio"
	"encoding/json"
	"flag"
	"fmt"
	"html/template"
	"io/ioutil"
	"net/http"
	"os"
	"path/filepath"
	"strings"

	"github.com/pressly/chi"
	"github.com/pressly/chi/middleware"
	"github.com/pressly/chi/render"
)

type DeviceStruct struct {
	DeviceID string `jason:"deviceID"`
	ID       string `json:"id"`
	Command  string `json:"command"`
}

type Model struct {
	Title   string
	Devices []string
}

const deviceListName = "device_list.txt"

func main() {

	if _, err := os.Stat(deviceListName); os.IsNotExist(err) {
		println("Device list does not exist. Creating.")
		f, err := os.Create(deviceListName)
		check(err)

		defer f.Close()
	}

	flag.Parse()

	r := chi.NewRouter()

	r.Use(middleware.RequestID)
	r.Use(middleware.Logger)
	r.Use(middleware.Recoverer)
	r.Use(render.SetContentType(render.ContentTypeJSON))

	workDir, _ := os.Getwd()
	filesDir := filepath.Join(workDir, "images")
	r.FileServer("/images", http.Dir(filesDir))

	r.Route("/", func(r chi.Router) {
		r.Get("/", Index)
	})

	r.Get("/ping", func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte("pong"))
	})

	// RESTy routes for "articles" resource
	r.Route("/v1/devices", func(r chi.Router) {
		r.Post("/", CreateDevice) // POST /devices
		r.Route("/:deviceID", func(r chi.Router) {
			r.Get("/", GetDevice)    // GET /deviceID/123
			r.Put("/", UpdateDevice) // PUT /deviceID/123
		})
	})

	http.ListenAndServe(":3333", r)

}

//Index Default landing page to set the colors.
func Index(w http.ResponseWriter, r *http.Request) {
	model := Model{Title: "LED Conrtoller"}

	//Update device's command.
	file, err := os.Open(deviceListName)
	check(err)
	defer file.Close()
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		part := strings.Split(scanner.Text(), ",")

		model.Devices = append(model.Devices, part[0])
	}
	fmt.Printf("%v\n", model.Devices)

	tpl := template.Must(template.New("main").ParseGlob("html/*.html"))

	tpl.ExecuteTemplate(w, "index.html", model)
}

//CreateDevice adds device to the file.
func CreateDevice(w http.ResponseWriter, r *http.Request) {
	iExist := false

	body, err := ioutil.ReadAll(r.Body)
	check(err)

	data := DeviceStruct{}

	err = json.Unmarshal(body, &data)
	check(err)

	//Check if device is in list.
	file, err := os.Open(deviceListName)
	check(err)
	defer file.Close()
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		part := strings.Split(scanner.Text(), ",")
		compare := strings.EqualFold(part[0], data.DeviceID)
		if compare {
			iExist = true
		}
	}

	fmt.Printf("Device %s is in list: %t\n", data.DeviceID, iExist)

	//If its created, return 201. If it already exists return 200.
	if iExist {
		//Device is already there. Return 200
		http.Error(w, http.StatusText(http.StatusOK), http.StatusOK)
	} else {
		updated := addToList(data.DeviceID)
		if updated {
			// Added to the list correctly. Return 201
			http.Error(w, http.StatusText(http.StatusCreated), http.StatusCreated)
		} else {
			// Error adding to the list. Return 500
			http.Error(w, http.StatusText(http.StatusInternalServerError), http.StatusInternalServerError)
		}
	}

}

//GetDevice returns the current command in the device list file.
func GetDevice(w http.ResponseWriter, r *http.Request) {
	deviceID := chi.URLParam(r, "deviceID")
	var returnConfig string

	// Load config from file
	file, err := os.Open(deviceListName)
	check(err)
	defer file.Close()
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		part := strings.Split(scanner.Text(), ",")
		compare := strings.EqualFold(part[0], deviceID)
		if compare {
			returnConfig = part[1]
		}
	}
	w.Write([]byte(returnConfig))

}

//UpdateDevice changes the command after the device in the device list file.
func UpdateDevice(w http.ResponseWriter, r *http.Request) {
	deviceID := chi.URLParam(r, "deviceID")
	body, err := ioutil.ReadAll(r.Body)
	check(err)

	data := DeviceStruct{}

	err = json.Unmarshal(body, &data)
	check(err)

	input, err := ioutil.ReadFile(deviceListName)
	check(err)
	lines := strings.Split(string(input), "\n")

	for i, line := range lines {
		part := strings.Split(line, ",")
		compare := strings.EqualFold(part[0], deviceID)
		if compare {
			lines[i] = deviceID + "," + data.Command
		}
	}
	output := strings.Join(lines, "\n")
	err = ioutil.WriteFile(deviceListName, []byte(output), 0666)
	if err != nil {
		// Error updating device. Return 500
		http.Error(w, http.StatusText(http.StatusInternalServerError), http.StatusInternalServerError)
	} else {
		//Device updated. Return 200
		http.Error(w, http.StatusText(http.StatusOK), http.StatusOK)
	}
}

func addToList(device string) bool {
	listUpdate := false
	f, err := os.OpenFile(deviceListName, os.O_APPEND, 0666)
	check(err)

	w := bufio.NewWriter(f)
	_, err = fmt.Fprintf(w, "%s,0\n", device)
	if err != nil {
		listUpdate = false
	} else {
		listUpdate = true
	}
	w.Flush()
	f.Close()
	return listUpdate
}

func check(e error) {
	if e != nil {
		panic(e)
	}
}
