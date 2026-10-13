FROM ubuntu:24.04
WORKDIR /app
COPY build/EmbeddedApp .
RUN chmod +x EmbeddedApp
CMD ["./EmbeddedApp"]
