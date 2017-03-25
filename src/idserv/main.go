package main

import (
	"bytes"
	"flag"
	"fmt"
	"log"
	"net"
	"os"
	"strconv"
	"strings"
	"time"
)

type server struct {
	name    string
	ip      string
	udpPort int
	tcpPort int
}

type serverSlice []server

func usage() {
	fmt.Println(`Example Usage: ../../bin/rmb [-i siip] [-p sipt]
    Arguments:
    -i              server ip
    -p              server port
    -h              show help
    return`)
	os.Exit(0)
}

func main() {
	var ip = flag.String("i", "127.0.0.1", "server ip")
	var port = flag.Int("p", 59000, "server port")
	var help = flag.Bool("h", false, "show help")

	if *help {
		usage()
	}
	flag.Parse()

	addr := net.UDPAddr{
		Port: *port,
		IP:   net.ParseIP(*ip),
	}

	conn, err := net.ListenUDP("udp", &addr)
	if err != nil {
		log.Fatal(err)
		os.Exit(1)
	}
	defer conn.Close()

	var servers serverSlice
	serverRecv := make(chan server, 10)
	askForServers := make(chan serverSlice)

	timer := time.NewTimer(time.Minute * 1)

	go handleUDPConn(conn, serverRecv, askForServers)

	for {
		select {
		case <-timer.C:
			servers = servers[:0]
            log.Println("Wiped Servers")
		case serverToAdd := <-serverRecv:
			for _, o := range servers {
				if (o.name == serverToAdd.name) &&
					(o.ip == serverToAdd.ip) &&
					(o.tcpPort == serverToAdd.tcpPort) &&
					(o.udpPort == serverToAdd.udpPort) {
					break
				}
			}
			log.Println("Server Added:", serverToAdd.name, "IP:", serverToAdd.ip, "Ports:", serverToAdd.tcpPort, serverToAdd.udpPort)
			servers = append(servers, serverToAdd)
		case <-askForServers:
			askForServers <- servers
		}
	}
}

func handleRegistrations(data string, serverRecv chan server) {
	buffer := strings.Split(data, ";")
	var err error

	if len(buffer) < 4 {
        log.Println("Invalid Register")
		return
	}

	newServer := server{buffer[0], buffer[1], 0, 0}

	newServer.udpPort, err = strconv.Atoi(buffer[2])
	if err != nil {
		fmt.Println(err)
		return
	}

	newServer.tcpPort, err = strconv.Atoi(buffer[3])
	if err != nil {
		fmt.Println(err)
		return
	}

	serverRecv <- newServer
}

func handleServerRequests(askForServers chan serverSlice, addr *net.UDPAddr, conn *net.UDPConn) {
	askForServers <- nil
	servers := <-askForServers
	var buffer bytes.Buffer

	buffer.WriteString("SERVERS\n")
	for _, srv := range servers {
		buffer.WriteString(fmt.Sprintf("%s;%s;%d;%d\n", srv.name, srv.ip, srv.udpPort, srv.tcpPort))
	}

	log.Println("Sending Servers to", addr.String())
	conn.WriteToUDP(buffer.Bytes(), addr)
}

func handleUDPConn(conn *net.UDPConn, serverRecv chan server, askForServers chan serverSlice) {
	buf := make([]byte, 1024)

	for {
		n, addr, err := conn.ReadFromUDP(buf)
		if err != nil {
			log.Println(err)
		}

		switch {
		case strings.Contains(string(buf[0:4]), "REG"):
			handleRegistrations(string(buf[4:n-2]), serverRecv)
		case strings.Contains(string(buf[0:11]), "GET_SERVERS"):
			go handleServerRequests(askForServers, addr, conn)
		default:
			log.Println("Unknown Command:", string(buf[0:n]))
		}
	}
}
