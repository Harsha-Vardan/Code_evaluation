# Use a lightweight GCC image
FROM gcc:latest

# Install make and other build essentials (gcc image already has them, but just in case)
RUN apt-get update && apt-get install -y make

# Set working directory
WORKDIR /app

# Copy the runner source code
COPY ./runner /app/runner
COPY ./testcases /app/testcases

# Build the runner
WORKDIR /app/runner
RUN make clean && make

# Set the entrypoint to the runner binary
ENTRYPOINT ["./runner"]
