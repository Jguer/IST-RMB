package main

import (
	"flag"
	"fmt"
	"log"
	"net"
	"os"
	"time"
)

type server struct {
	name    string
	ip      string
	udpPort int
	tcpPort int
}

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
	defer conn.Close()
	if err != nil {
		log.Fatal(err)
		os.Exit(1)
	}

	var servers []server

	buf := make([]byte, 1024)
	timer := time.NewTimer(time.Minute * 1)

	for {
		select {
		case <-timer.C:
			servers = servers[:0]
		}

		n, addr, err := conn.ReadFromUDP(buf)
        if strings.Contains(buf[:], "REG") {
            
        }

	}
}
