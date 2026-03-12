# Stage 1: Build Stage
# Use a standard C++ build environment with all necessary dependencies.
FROM gcc:latest as builder

# Install dependencies
RUN apt-get update && apt-get install -y cmake openssl libssl-dev zlib1g-dev

# Copy the entire project source into the build environment
WORKDIR /usr/src/app
COPY . .

# Build the C++ binary using the mandatory tradesecret.sh script.
# This ensures a consistent and secure build process.
RUN chmod +x tradesecret.sh
RUN ./tradesecret.sh --cli help > /dev/null

# ---
# Stage 2: Runtime Stage
# Use a minimal base image to reduce the final container size and attack surface.
FROM debian:slim

# Install only the necessary runtime libraries
RUN apt-get update && apt-get install -y libssl3 zlib1g

# Set up a non-root user for security
RUN useradd -ms /bin/bash agent
USER agent
WORKDIR /home/agent/app

# Create a dedicated volume for persistent blockchain data
VOLUME /home/agent/app/data

# Copy only the compiled binary and essential runtime files from the build stage
COPY --from=builder /usr/src/app/build/AgenticPipeline .
COPY --from=builder /usr/src/app/tradesecret.sh .
COPY --from=builder /usr/src/app/www ./www

# Set the entrypoint to a dedicated container script
COPY ./docker/docker-entrypoint.sh /home/agent/
ENTRYPOINT ["/home/agent/docker-entrypoint.sh"]

# Expose default ports
EXPOSE 8080 9000 9100 9200
