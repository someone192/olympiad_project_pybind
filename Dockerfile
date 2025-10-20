FROM archlinux:base-devel

RUN pacman -Syu --noconfirm base-devel gcc cmake python python-headers gtest git

WORKDIR /app
COPY . /app

# Pybind11: Если нет в pacman, clone
RUN git clone --branch v2.13.5 https://github.com/pybind/pybind11.git /usr/include/pybind11

# Сборка через CMake
RUN cmake -S . -B build && cmake --build build

CMD ["python", "main.py"]