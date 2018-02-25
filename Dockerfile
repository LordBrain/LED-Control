FROM instrumentisto/glide:latest
WORKDIR /go/src/
RUN mkdir github.com && cd github.com
RUN git clone https://github.com/blee1170/LED-Control.git
RUN cd LED-Control
RUN glide install
RUN GOOS=linux GOARCH=amd64 go build

FROM golang:1.10-alpine
COPY --from=0 /go/src/github.com/LED-Control/LED-Control .
CMD ["./LED-Control"]
