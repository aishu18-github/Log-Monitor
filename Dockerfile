FROM gcc:latest

WORKDIR /app

COPY . /app

RUN make

CMD ["./logmon"]
