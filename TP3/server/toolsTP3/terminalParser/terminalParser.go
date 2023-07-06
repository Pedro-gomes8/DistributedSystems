package terminalParser

import (
	"SD2023Tp3/server/models"
	"fmt"
	"google.golang.org/grpc"
)

func printMap(mp map[int32]int32) string {
	output := "Process ID \t Times served\n"
	for k, v := range mp {
		output += fmt.Sprintf("%d \t\t %d\n", k, v)
	}
	return output
}

func TerminalParser(s *grpc.Server, servBone *models.Tp3RPCServer) {
	fmt.Println("Please choose the corresponding command:")
	fmt.Println("1: Print the current queue")
	fmt.Println("2: Print how many times each process has been served")
	fmt.Println("3: Forcefully stop the server: Cancels all RPCs and open connections and stops the server")

	firstRequest := true
outsideLoop:
	for {
		if firstRequest {
			fmt.Print("What would you like to do? ")
			firstRequest = false
		} else {
			fmt.Print("What would you like to do now? ")
		}
		var chosenResponse int32
		fmt.Scanln(&chosenResponse)
		fmt.Println("")

		switch chosenResponse {
		case 1:
			fmt.Println(servBone.ShowQueue())
			fmt.Println("")
		case 2:
			served := servBone.ShowServed()
			fmt.Println(printMap(served))
			fmt.Println("")
		case 3:
			fmt.Println("Forcefully stopping the server")
			s.Stop()
			break outsideLoop
		}
	}

}
