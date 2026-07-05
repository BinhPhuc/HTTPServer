#!/bin/bash

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}========================================${NC}"
echo -e "${YELLOW}  HTTP Server - Generate local CA + cert${NC}"
echo -e "${YELLOW}========================================${NC}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CERT_DIR="$PROJECT_ROOT/certs"
mkdir -p "$CERT_DIR"
cd "$CERT_DIR"

CA_DAYS=3650     
LEAF_DAYS=825    
CN="ZEROHTTP Local"

if [ ! -f "rootCA.key" ] || [ ! -f "rootCA.pem" ]; then
    echo -e "${YELLOW}[1/3] Creating Root CA...${NC}"
    openssl genrsa -out rootCA.key 4096
    openssl req -x509 -new -nodes -key rootCA.key -sha256 -days "$CA_DAYS" \
        -out rootCA.pem \
        -subj "/C=VN/ST=Hanoi/L=Hanoi/O=ZEROHTTP/OU=Dev/CN=ZEROHTTP Local Root CA"
else
    echo -e "${GREEN}[1/3] Reusing existing Root CA (rootCA.pem)${NC}"
fi

echo -e "${YELLOW}[2/3] Creating server key + CSR...${NC}"
openssl genrsa -out key.pem 2048
openssl req -new -key key.pem -out server.csr \
    -subj "/C=VN/ST=Hanoi/L=Hanoi/O=ZEROHTTP/OU=Dev/CN=$CN"

cat > server.ext <<'EOF'
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage=digitalSignature,keyEncipherment
extendedKeyUsage=serverAuth
subjectAltName=@alt_names

[alt_names]
DNS.1 = localhost
IP.1  = 127.0.0.1
IP.2  = ::1
EOF

echo -e "${YELLOW}[3/3] Signing server cert with Root CA...${NC}"
openssl x509 -req -in server.csr \
    -CA rootCA.pem -CAkey rootCA.key -CAcreateserial \
    -out cert.pem -days "$LEAF_DAYS" -sha256 -extfile server.ext

rm -f server.csr server.ext rootCA.srl

echo -e "${GREEN}Done. Files in $CERT_DIR:${NC}"
echo -e "  ${GREEN}cert.pem${NC}   -> server certificate (loaded by the server)"
echo -e "  ${GREEN}key.pem${NC}    -> server private key (loaded by the server)"
echo -e "  ${GREEN}rootCA.pem${NC} -> import THIS into your browser/OS trust store"
echo
openssl x509 -in cert.pem -noout -subject -issuer -dates -ext subjectAltName
