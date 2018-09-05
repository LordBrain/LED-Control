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

	"github.com/go-chi/chi"
	"github.com/go-chi/chi/middleware"
	"github.com/go-chi/render"
)

//DeviceStruct struct for the devices.
type DeviceStruct struct {
	DeviceID string `json:"deviceID"`
	ID       string `json:"id"`
	Command  string `json:"command"`
}

//Model is our model
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
	FileServer(r, "/images", http.Dir(filesDir))

	r.Route("/", func(r chi.Router) {
		r.Get("/", Index)
	})

	///ping endpoint, returns wiht pong.
	r.Get("/ping", func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte("pong"))
	})

	//Main API endpoints
	r.Route("/v1/devices/{deviceID}", func(r chi.Router) {
		r.Post("/", CreateDevice) // POST /devices
		r.Get("/", GetDevice)     // GET /deviceID/123
		r.Put("/", UpdateDevice)  // PUT /deviceID/123
	})
	//Sets the default port
	PORT := "3333"

	//Checks to see if there is a environment variable to change the port.
	val, ok := os.LookupEnv("PORT")
	if !ok {
		fmt.Printf("%s not set. Using default 3333\n", "PORT")
	} else {
		fmt.Printf("%s=%s\n", "PORT", val)
		PORT = val
	}

	http.ListenAndServe(":"+PORT, r)

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
	deviceID := chi.URLParam(r, "deviceID")
	iExist := false

	//Check if device is in list.
	file, err := os.Open(deviceListName)
	check(err)
	defer file.Close()
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		part := strings.Split(scanner.Text(), ",")
		compare := strings.EqualFold(part[0], deviceID)
		if compare {
			iExist = true
		}
	}

	fmt.Printf("Device %s is in list: %t\n", deviceID, iExist)

	//If its created, return 201. If it already exists return 200.
	if iExist {
		//Device is already there. Return 200
		http.Error(w, http.StatusText(http.StatusOK), http.StatusOK)
	} else {
		// updated := addToList(data.DeviceID)
		updated := addToList(deviceID)

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
	w.Write([]byte(returnConfig + "\r\n"))

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
	f, err := os.OpenFile(deviceListName, os.O_RDWR|os.O_APPEND|os.O_CREATE, 0666)
	check(err)
	println(device)
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

// FileServer Load the color image for the root /
func FileServer(r chi.Router, path string, root http.FileSystem) {
	if strings.ContainsAny(path, ":*") {
		panic("FileServer does not permit URL parameters.")
	}

	fs := http.StripPrefix(path, http.FileServer(root))

	if path != "/" && path[len(path)-1] != '/' {
		r.Get(path, http.RedirectHandler(path+"/", 301).ServeHTTP)
		path += "/"
	}
	path += "*"

	r.Get(path, http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		fs.ServeHTTP(w, r)
	}))
}
