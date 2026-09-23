#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DEPS_DIR="${SCRIPT_DIR}/deps"
INSTALL_DIR="${SCRIPT_DIR}/install"
mkdir -p "${DEPS_DIR}" "${INSTALL_DIR}"

echo "=========================================="
echo " Step 1: Clone dependencies"
echo "=========================================="

# tea-cpp (darabonba_core)
if [ ! -d "${DEPS_DIR}/tea-cpp" ]; then
    echo "Cloning tea-cpp..."
    git clone --depth 1 https://github.com/aliyun/tea-cpp.git "${DEPS_DIR}/tea-cpp"
fi

# credentials-cpp
if [ ! -d "${DEPS_DIR}/credentials-cpp" ]; then
    echo "Cloning credentials-cpp..."
    git clone --depth 1 https://github.com/aliyun/credentials-cpp.git "${DEPS_DIR}/credentials-cpp"
fi

# alibabacloud-open-api-v2
if [ ! -d "${DEPS_DIR}/alibabacloud-open-api-v2" ]; then
    echo "Cloning alibabacloud-open-api-v2..."
    git clone --depth 1 https://github.com/alibabacloud-sdk-cpp/alibabacloud-open-api-v2.git "${DEPS_DIR}/alibabacloud-open-api-v2"
fi

echo "=========================================="
echo " Step 2: Build tea-cpp (darabonba_core)"
echo "=========================================="
cd "${DEPS_DIR}/tea-cpp"
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
         -DENABLE_UNIT_TESTS=OFF \
         -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_CXX_FLAGS="-I${SCRIPT_DIR}/include"
make -j$(nproc) 2>&1 | tail -20
make install 2>&1 | tail -10

echo "=========================================="
echo " Step 3: Build credentials-cpp"
echo "=========================================="
cd "${DEPS_DIR}/credentials-cpp"
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
         -DENABLE_UNIT_TESTS=OFF \
         -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) 2>&1 | tail -20
make install 2>&1 | tail -10

echo "=========================================="
echo " Step 4: Build alibabacloud-open-api-v2"
echo "=========================================="
cd "${DEPS_DIR}/alibabacloud-open-api-v2"
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
         -DENABLE_UNIT_TESTS=OFF \
         -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_PREFIX_PATH="${INSTALL_DIR}"
make -j$(nproc) 2>&1 | tail -20
make install 2>&1 | tail -10

echo "=========================================="
echo " Step 5: Build dypnsapi-20170525"
echo "=========================================="
cd "${SCRIPT_DIR}/dypnsapi-20170525"
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
         -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_PREFIX_PATH="${INSTALL_DIR}" \
         -DCMAKE_CXX_FLAGS="-I${SCRIPT_DIR}/include"
make -j$(nproc) 2>&1 | tail -30
make install 2>&1 | tail -10

echo "=========================================="
echo " All done! SDK installed to: ${INSTALL_DIR}"
echo "=========================================="
ls -la "${INSTALL_DIR}/lib/" 2>/dev/null
ls -la "${INSTALL_DIR}/include/" 2>/dev/null
