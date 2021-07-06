FROM ubuntu:20.10

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get upgrade -y
RUN apt-get install make vim g++ -y

WORKDIR /cpp-socket-library

COPY . .

RUN chmod a+rx dependencies.sh
RUN ./dependencies.sh

RUN make all
