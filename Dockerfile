# Use an official base image with build tools
FROM ubuntu:20.04

# Set environment variables for non-interactive installation
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary packages
RUN apt-get update && \
    apt-get install -y \
    git \
    build-essential \
    wget \
    curl \
    ncurses \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /usr/src/app

# Clone the GitHub repository (replace with your repository URL)
RUN git clone https://github.com/TwelveFacedJanus/BSDBook.git .

# Checkout the specific release tag (replace with your release tag)
RUN git checkout tags/v1.8.6 -b v1.8.6

# Compile the code using make
RUN make

EXPOSE 8080

# Set the entry point (replace with your application's entry point)
ENTRYPOINT ["./bin/bsdnotes --server"]

HEALTHCHECK CMD curl --fail http://localhost:8080 || exit 1
