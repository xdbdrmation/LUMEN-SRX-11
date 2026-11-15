# LUMEN-SRX-11
This website provides a easy way to view FITS images from spherex, it uses C++ as its backend making it fast
# Project Overview

A C++ backend application that hosts a local HTTP server, fetches astronomical catalog data, and processes FITS and XML formats.

## Known Issue: IRSA DR1 Service Availability

> **Warning:** The CSV catalog data is requested directly from **IRSA Data Release 1 (DR1)**. IRSA DR1 is currently experiencing intermittent **502 Bad Gateway** errors on their end. Because of this upstream server issue, data fetching requests may fail or time out until IRSA resolves the gateway outage.

---

## Tech Stack & Dependencies

* **Language:** C++ (C++17 or later recommended)
* **Local Server & HTTP Client:** `cpp-httplib` — Handles both the local HTTP server hosting and outbound requests to IRSA.
* **FITS File Processing:** `CCfits` (requires `cfitsio`) — Used for reading, writing, and manipulating FITS (Flexible Image Transport System) files.
* **XML Parsing:** `tinyxml2` — Used for lightweight parsing of XML metadata and service responses.

---

## Architecture & Workflow

1. **Local Server:** Starts a local HTTP server using `httplib::Server` to expose backend endpoints.
2. **Data Retrieval:** Queries IRSA DR1 via HTTP to download target CSV datasets *(subject to current 502 Bad Gateway issues)*.
3. **Metadata & FITS Handling:** Parses XML responses/configurations with `tinyxml2` and processes astronomical image/table data using `CCfits`.

---

## Prerequisites & Build

### Dependencies
Ensure the following libraries are installed on your system:
* `CCfits`
* `tinyxml2`
* `cpp-httplib` (header-only; include `httplib.h` in your include path)
* OpenSSL (optional, required if making HTTPS requests via `httplib`)

### Example Compilation
```bash
clang++ ServerV5.cpp -o ServerV5 -I/usr/include/opencv5 -lssl -lcrypto -lCCfits -lcfitsio -ltinyxml2 -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -pthread
```
*(Add `-DCPPHTTPLIB_OPENSSL_SUPPORT -lssl -lcrypto` if HTTPS support is enabled in `httplib`.)*

---

## Running the Server

1. Compile the binary using the command above.
2. Run the executable:
   ```bash
   ./server
   ```
3. Access the local endpoints via `http://localhost:<PORT>` (check your source configuration for the exact port number).

---

## Troubleshooting

* **502 Bad Gateway when fetching CSV:** This is an external issue with the IRSA DR1 endpoint, not a bug in the local C++ backend. Retry the request later or test with a locally cached CSV file if available.
* **Linker errors with `CCfits`:** Ensure `libcfitsio` is linked *after* `libCCfits` (`-lCCfits -lcfitsio`).


