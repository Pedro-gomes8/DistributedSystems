package writelog

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"time"
)

var _, currentFilePath, _, _ = runtime.Caller(0)

var pathToWriteLog = filepath.Dir(filepath.Dir(filepath.Dir(currentFilePath)))

// WriteLog writes a message to the log file
func WriteLog(messageType string, pId int32) {

	now := time.Now()

	file, err := os.OpenFile(pathToWriteLog+"log.txt", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		panic(err)
	}
	defer file.Close()

	fileInfo, err := file.Stat()
	if err != nil {
		panic(err)
	}

	if fileInfo.Size() == 0 {
		headers := "Message\tProcess ID\tTime\n"
		if _, err := file.WriteString(headers); err != nil {
			panic(err)
		}
	}
	if _, err := file.WriteString(messageType + "\t" + fmt.Sprint(pId) + "\t" + now.Format("2006-01-02 15:04:04.000000") + "\n"); err != nil {
		panic(err)
	}

}
