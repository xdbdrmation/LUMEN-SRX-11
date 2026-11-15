
# LUMEN-SRX-11
-This website provides a easy way to view FITS images from spherex, it uses C++ as its backend making it fast
-PLEASE RUN THE ServerV5 as it is the FUlly developed version
# IMAGE - 1 | RA = 133.79513  RD = -7.245147 NAME = WISE 0855 (Please zoom in to see them as the radius was defined very high)
<img width="2040" height="2040" alt="image (1)" src="https://github.com/user-attachments/assets/e8bff1f1-fa4f-4b48-8bb4-2fe9ef822308" />
# IMAGE - 2 RA = 83.82 RD = -5.38 NAME = ORION NEBULAE (Please zoom in to see them as the radius was defined very high)
<img width="2040" height="2040" alt="image (2)" src="https://github.com/user-attachments/assets/c7e9183c-cac8-47d6-a143-e638b6b9cd6d" />

# Project Overview

A C++ backend application that hosts a local HTTP server, fetches astronomical catalog data, and processes FITS and XML formats.

## Known Issue: IRSA DR1 Service Availability

> **Warning:** The CSV catalog data is requested directly from **IRSA Data Release 1 (DR1)**. IRSA DR1 is currently experiencing intermittent **502 Bad Gateway** errors on their end. Because of this upstream server issue, data fetching requests may fail or time out until IRSA resolves the gateway outage.

---

## Tech Stack & Dependencies

* **Language:** C++ (C++17 or later recommended)
* **Local Server & HTTP Client:** `cpp-httplib` — Handles both the local HTTP server hosting and outbound requests to IRSA.
* **FITS File Processing:** `CCfits` — Used for reading, writing, and manipulating FITS (Flexible Image Transport System) files.
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
   ./ServerV5
   ```
3. Access the local endpoints via `http://localhost:<PORT>` (check your source configuration for the exact port number).

---

## Troubleshooting

* **502 Bad Gateway when fetching CSV:** This is an external issue with the IRSA DR1 endpoint, not a bug in the local C++ backend. Retry the request later or test with a locally cached CSV file if available.
* **Linker errors with `CCfits`:** Ensure `libcfitsio` is linked *after* `libCCfits` (`-lCCfits -lcfitsio`).


