# syntax=docker/dockerfile:1
# Select Python Slim (Debian)
FROM python:3.13-slim
# Install dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
  git \
  wget \
  cmake \
  binutils \
  gcc \
  make \
  g++ \
  gdb \
  && apt-get clean && rm -rf /var/lib/apt/lists/*
# Install ESP-IDF dependencies
ENV IDF_PATH=/opt/esp-idf
RUN git clone --recursive -b v5.1 https://github.com/espressif/esp-idf.git $IDF_PATH \
  && $IDF_PATH/install.sh all
# Install Python dependencies
RUN pip install --no-cache-dir \
  kconfiglib \
  jinja2 \
  pytest \
  pyserial
# Create folder /uedp-libs for base source code
RUN mkdir -p /uedp-libs/
# Create folder /test for PLTF
RUN mkdir -p /uedp-test/
# Install gosu to setup user
RUN apt-get update && apt-get install -y gosu && rm -rf /var/lib/apt/lists/*
# Select /uedp in docker space
WORKDIR /uedp-libs
# Setup color for environment
ENV TERM=xterm-256color
# Copy current source code in to /uedp folder
COPY . /uedp-libs
# Add ESP-IDF environment variables to bashrc
RUN echo "source $IDF_PATH/export.sh > /dev/null 2>&1" >> ~/.bashrc
# Entry with entrypoint script
COPY entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh
# Call entrypoint procedure
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]