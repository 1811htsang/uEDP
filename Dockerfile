# syntax=docker/dockerfile:1
# Select Python Slim (Debian)
FROM python:3.13-slim
# Kconfig install
RUN pip install --no-cache-dir kconfiglib
# Jinja2 install
RUN pip install --no-cache-dir jinja2
# Pytest install
RUN pip install --no-cache-dir pytest
# Pyserial install
RUN pip install --no-cache-dir pyserial
# Create folder /uedp-libs for base source code
RUN mkdir -p /uedp-libs/
# Create folder /test for PLTF
RUN mkdir -p /uedp-test/
# Select /uedp in docker space
WORKDIR /uedp-libs
# Setup color for environment
ENV TERM=xterm-256color
# Copy current source code in to /uedp folder
COPY . /uedp-libs
# Entry with entrypoint script
COPY entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh
# Call entrypoint procedure
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]