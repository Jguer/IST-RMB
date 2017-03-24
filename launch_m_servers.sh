#!/bin/bash

# Parameter
## User setting
## Administrator setting
readonly WORK_DIR=$(dirname "$(readlink -f "$0")")

readonly NAMES=( "Москва"
                 "Екатеринбург"
                 "Смоленск"
                 "Сталингра́д"
                 "Самара"
                 "Нижний Новгород"
                 "Новосибирск"
                 "Санкт-Петербург" )

readonly CAPACITY=( "20"
                 "100"
                 "200"
                 "250"
                 "300"
                 "200"
                 "1000"
                 "1000" )
# Usage Function
function usage() {
cat <<_EOT_
Usage:
  $0 -m number of servers
_EOT_
exit 1
}

# Main
## check options and arguments

if [ $# = 0 ]; then
  usage
  exit 1
fi

if [ "$OPTIND" = 1 ]; then
  while getopts m:h OPT
  do
    case $OPT in
      m)
        readonly M=$OPTARG
        ;;
      h)
        echo "[debug]h option. display help"           # for debug
        usage
        ;;
      \?)
        echo "Try to enter the h option" 1>&2
        ;;
    esac
  done
else
  echo "No installed getopts-command" 1>&2
  exit 1
fi

shift $((OPTIND - 1))

if [[ -z $M ]]; then
  echo "No specify argument(s) of -m option" 1>&2
  exit 1
fi

IP=$(curl -s icanhazip.com)
UDP_PORT=58000
TCP_PORT=57000

## main
make server

for (( i=0; $i < ${#NAMES[@]} && $i < $M; ++i)); do
  ./bin/msgserv -d -n "${NAMES[$i]}" -j "${IP}" -u "${UDP_PORT}" -t "${TCP_PORT}" -m "${CAPACITY[$i]}" &
  ((UDP_PORT++))
  ((TCP_PORT++))
done
