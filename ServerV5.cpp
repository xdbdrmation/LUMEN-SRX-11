
#include <iostream>
#include <vector>
#include <string>
#include <iterator>
#include <chrono>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include <fstream>
#include <CCfits/CCfits>
#include <opencv2/opencv.hpp>
#include <valarray>
#include <algorithm>
#include <cmath>
#include <limits>
#include <cstring>
#include "tinyxml2.h"

// http://127.0.0.1:8081

int main() {

    httplib::Server server;

    server.Get("/", [](const httplib::Request& req, httplib::Response& res) {

        res.set_content(R"(
            <html>
                <head>
                    <link rel="preconnect" href="https://googleapis.com">
                    <link rel="preconnect" href="https://gstatic.com" crossorigin>
                    <link href="https://googleapis.com/css2?family=Archivo+Black&display=swap" rel="stylesheet">

                    <style>
                        body {
                            background-image: url('https://nasa.gov');
                            background-size: cover;
                            background-position: center;
                            background-attachment: fixed;
                            background-color: lightgray;
                        }

                        h1 {
                            color: white;
                            text-align: center;
                            font-family: 'Archivo Black', sans-serif;
                            font-size: 90px;
                            font-weight: 800;
                            text-shadow: 2px 2px 8px rgba(0,0,0,0.7);
                        }

                        .Main_text {
                            color: white;
                            text-align: center;
                            font-family: 'Archivo Black', sans-serif;
                            font-size: 40px;
                            text-shadow: 2px 2px 6px rgba(0,0,0,0.7);
                        }

                        .Note {
                            color: #ffcccc;
                            text-align: center;
                            font-family: 'Archivo Black', sans-serif;
                            font-size: 18px;
                            text-shadow: 1px 1px 4px rgba(0,0,0,0.7);
                        }

                        .RA, .DEC {
                            display: block;
                            margin: 10px auto;
                            width: 300px;
                            padding: 10px;
                            font-size: 16px;
                            border-radius: 5px;
                            border: 1px solid #ccc;
                            background: rgba(255, 255, 255, 0.9);
                        }

                        button {
                            display: block;
                            margin: 20px auto;
                            padding: 10px 20px;
                            font-size: 16px;
                            background-color: #c88b8b;
                            color: white;
                            border: none;
                            border-radius: 5px;
                            cursor: pointer;
                            font-family: 'Archivo Black', sans-serif;
                        }

                        button:hover {
                            background-color: #b07575;
                        }

                        .result-image {
                            display: block;
                            max-width: 90%;
                            max-height: 700px;
                            margin: 30px auto;
                        }

                        #loading {
                            display: none;
                            text-align: center;
                            color: white;
                            font-family: 'Archivo Black', sans-serif;
                            font-size: 30px;
                            margin-top: 30px;
                        }
                    </style>
                </head>

                <body>
                    <h1>LUMEN</h1>

                    <p class="Main_text">Map out the space</p>

                    <p class="Note">
                        NOTE: Please enter the Right Ascension and Declination in DECIMAL FORMAT!
                    </p>

                    <form action="/process" method="post">
                        <input type="text" name="RA"
                               placeholder="Enter Right Ascension" class="RA">

                        <input type="text" name="DEC"
                               placeholder="Enter Declination" class="DEC">

                        <br>

                        <button type="submit">SUBMIT</button>
                    </form>

                    <div id="loading">
                        PLEASE WAIT...
                        <p>Retrieving SPHEREx observations</p>
                    </div>

                    <img class="result-image" src="/image"
                         alt="SPHEREx image will appear here">

                    <script>
                        document.querySelector("form").addEventListener("submit", function() {
                            document.getElementById("loading").style.display = "block";
                        });
                    </script>
                </body>
            </html>
        )", "text/html");
    });


    server.Get("/image", [](const httplib::Request& req, httplib::Response& res) {

        std::ifstream file("output.png", std::ios::binary);

        if (!file) {
            res.status = 404;
            res.set_content("Image not generated yet!", "text/plain");
            return;
        }

        std::string image(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );

        res.set_content(image, "image/png");
    });


    server.Post("/process", [](const httplib::Request& req, httplib::Response& res) {

        if (req.has_param("RA") && req.has_param("DEC")) {

            std::string RA = req.get_param_value("RA");
            std::string DEC = req.get_param_value("DEC");

            std::string reqcsv =
                "/SIA?COLLECTION=spherex_dr1&POS=circle+" +
                RA + "+" + DEC + "+0.02&MAXREC=1";

            std::string url = "irsa.ipac.caltech.edu";

            std::cout << reqcsv << std::endl;

            httplib::SSLClient cli(url.c_str());

            cli.set_follow_location(true);

            // SIA request timeouts
            cli.set_connection_timeout(15, 0);
            cli.set_read_timeout(45, 0);
            cli.set_write_timeout(15, 0);

            // Measure SIA request duration
            auto start = std::chrono::steady_clock::now();

            std::cout << "Sending SIA request..." << std::endl;

            if (auto irsa_res = cli.Get(reqcsv.c_str())) {

                auto end = std::chrono::steady_clock::now();

                double elapsed =
                    std::chrono::duration<double>(end - start).count();

                std::cout << "SIA response received!" << std::endl;
                std::cout << "HTTP Status: " << irsa_res->status << std::endl;
                std::cout << "Time taken: " << elapsed << " seconds" << std::endl;

                if (irsa_res->status == 200) {

                    std::string xml = irsa_res->body;
                    std::vector<std::string> accessUrls;

                    tinyxml2::XMLDocument doc;
                    doc.Parse(xml.c_str());

                    if (doc.Error()) {

                        std::cout << "XML Parse Error: "
                                  << doc.ErrorStr() << std::endl;

                    }
                    else {

                        auto* root = doc.FirstChildElement("VOTABLE");

                        if (root != nullptr) {

                            auto* resource = root->FirstChildElement("RESOURCE");

                            if (resource != nullptr) {

                                auto* table = resource->FirstChildElement("TABLE");

                                if (table != nullptr) {

                                    std::vector<std::string> fieldnames;

                                    auto* field = table->FirstChildElement("FIELD");

                                    while (field != nullptr) {

                                        const char* name = field->Attribute("name");

                                        if (name != nullptr) {

                                            fieldnames.push_back(name);

                                            std::cout << "Field name: "
                                                      << name << std::endl;
                                        }

                                        field = field->NextSiblingElement("FIELD");
                                    }

                                    int accessUrlIndex = -1;

                                    for (int i = 0;
                                         i < static_cast<int>(fieldnames.size());
                                         i++) {

                                        if (fieldnames[i] == "access_url") {
                                            accessUrlIndex = i;
                                            break;
                                        }
                                    }

                                    std::cout << "Access URL index: "
                                              << accessUrlIndex << std::endl;

                                    if (accessUrlIndex != -1) {

                                        auto* data = table->FirstChildElement("DATA");

                                        auto* tabledata =
                                            data ? data->FirstChildElement("TABLEDATA") : nullptr;

                                        auto* tr =
                                            tabledata ? tabledata->FirstChildElement("TR") : nullptr;

                                        while (tr != nullptr) {

                                            auto* td = tr->FirstChildElement("TD");

                                            int currentCol = 0;

                                            while (td != nullptr) {

                                                if (currentCol == accessUrlIndex) {

                                                    const char* val = td->GetText();

                                                    if (val != nullptr) {
                                                        accessUrls.push_back(val);
                                                    }

                                                    break;
                                                }

                                                td = td->NextSiblingElement("TD");
                                                currentCol++;
                                            }

                                            tr = tr->NextSiblingElement("TR");
                                        }
                                    }
                                }
                                else {
                                    std::cout << "TABLE not found!" << std::endl;
                                }
                            }
                            else {
                                std::cout << "RESOURCE not found!" << std::endl;
                            }
                        }
                        else {
                            std::cout << "VOTABLE not found!" << std::endl;
                        }
                    }


                    std::string outputText = "Extracted access_urls:\n\n";

                    for (const auto& fitsUrl : accessUrls) {

                        const std::string prefix = "https://irsa.ipac.caltech.edu";

                        if (fitsUrl.rfind(prefix, 0) == 0) {
                            std::cout << "Prefix found in URL: "
                                      << fitsUrl << std::endl;
                        }
                        else {
                            std::cout << "Prefix not found in URL: "
                                      << fitsUrl << std::endl;
                        }

                        if (fitsUrl.rfind(prefix, 0) == 0) {

                            std::string path = fitsUrl.substr(prefix.length());

                            httplib::SSLClient fitsClient("irsa.ipac.caltech.edu");
                            fitsClient.set_follow_location(true);

                            // FITS download timeouts
                            fitsClient.set_connection_timeout(15, 0);
                            fitsClient.set_read_timeout(120, 0);
                            fitsClient.set_write_timeout(15, 0);

                            auto fitsStart = std::chrono::steady_clock::now();

                            std::cout << "Downloading FITS..." << std::endl;

                            auto fitsResponse = fitsClient.Get(path.c_str());

                            auto fitsEnd = std::chrono::steady_clock::now();

                            double fitsElapsed =
                                std::chrono::duration<double>(
                                    fitsEnd - fitsStart
                                ).count();

                            std::cout << "FITS download time: "
                                      << fitsElapsed << " seconds" << std::endl;

                            if (!fitsResponse) {

                                std::cout << "FITS Connection Error!" << std::endl;

                            }
                            else if (fitsResponse->body.empty()) {

                                std::cout << "FITS response is empty!" << std::endl;

                            }
                            else {

                                std::cout << "FITS HTTP status: "
                                          << fitsResponse->status << std::endl;

                                if (fitsResponse->status != 200) {
                                    std::cout << "FITS HTTP request failed!" << std::endl;
                                    continue;
                                }

                                std::cout << "FITS response received successfully! Size: "
                                          << fitsResponse->body.size()
                                          << " bytes" << std::endl;

                                if (fitsResponse->body.compare(0, 8, "SIMPLE  ") != 0) {

                                    std::cout << "Response does not have a standard FITS header!"
                                              << std::endl;

                                }
                                else {

                                    std::ofstream outFile("output.fits", std::ios::binary);

                                    outFile.write(
                                        fitsResponse->body.data(),
                                        static_cast<std::streamsize>(fitsResponse->body.size())
                                    );

                                    outFile.close();

                                    if (!outFile) {

                                        std::cout << "FITS file saving failed"
                                                  << std::endl;

                                    }
                                    else {

                                        std::cout << "FITS file saved successfully"
                                                  << std::endl;

                                        try {

                                            CCfits::FITS fits(
                                                "output.fits",
                                                CCfits::Read,
                                                true
                                            );

                                            std::cout << "FITS opened successfully"
                                                      << std::endl;

                                            std::cout << "Number of extensions: "
                                                      << fits.extension().size()
                                                      << std::endl;

                                            bool imageFound = false;

                                            for (int i = 1;
                                                 i <= static_cast<int>(fits.extension().size());
                                                 i++) {

                                                auto& hdu = fits.extension(i);

                                                long dimensions = hdu.axes();

                                                std::cout << "Extension " << i
                                                          << " dimensions: "
                                                          << dimensions << std::endl;

                                                if (dimensions < 2) {
                                                    continue;
                                                }

                                                long width = hdu.axis(0);
                                                long height = hdu.axis(1);

                                                if (width <= 0 || height <= 0) {
                                                    continue;
                                                }

                                                std::cout << "Image dimensions: "
                                                          << width << " x "
                                                          << height << std::endl;

                                                std::valarray<double> pixels;

                                                try {
                                                    hdu.read(pixels);
                                                }
                                                catch (const std::exception& e) {

                                                    std::cout << "Skipping extension: "
                                                              << e.what() << std::endl;

                                                    continue;
                                                }

                                                if (pixels.size() <
                                                    static_cast<size_t>(width * height)) {
                                                    continue;
                                                }

                                                double minValue =
                                                    std::numeric_limits<double>::max();

                                                double maxValue =
                                                    std::numeric_limits<double>::lowest();

                                                for (size_t j = 0;
                                                     j < static_cast<size_t>(width * height);
                                                     j++) {

                                                    if (std::isfinite(pixels[j])) {
                                                        minValue = std::min(minValue, pixels[j]);
                                                        maxValue = std::max(maxValue, pixels[j]);
                                                    }
                                                }

                                                if (!std::isfinite(minValue) ||
                                                    !std::isfinite(maxValue) ||
                                                    minValue == maxValue) {
                                                    continue;
                                                }

                                                cv::Mat image(
                                                    static_cast<int>(height),
                                                    static_cast<int>(width),
                                                    CV_8UC1
                                                );

                                                for (long y = 0; y < height; y++) {

                                                    for (long x = 0; x < width; x++) {

                                                        double value = pixels[
                                                            static_cast<size_t>(y * width + x)
                                                        ];

                                                        unsigned char pixel = 0;

                                                        if (std::isfinite(value)) {

                                                            double normalized =
                                                                (value - minValue) /
                                                                (maxValue - minValue) * 255.0;

                                                            pixel = static_cast<unsigned char>(
                                                                std::clamp(normalized, 0.0, 255.0)
                                                            );
                                                        }

                                                        image.at<unsigned char>(
                                                            static_cast<int>(y),
                                                            static_cast<int>(x)
                                                        ) = pixel;
                                                    }
                                                }

                                                if (cv::imwrite("output.png", image)) {

                                                    std::cout << "PNG CONVERSION SUCCESSFUL!"
                                                              << std::endl;

                                                    imageFound = true;
                                                    break;
                                                }
                                            }

                                            if (!imageFound) {
                                                std::cerr << "No convertible image extension found!"
                                                          << std::endl;
                                            }

                                        }
                                        catch (const std::exception& e) {

                                            std::cout << "Error opening FITS file: "
                                                      << e.what() << std::endl;
                                        }
                                    }
                                }
                            }
                        }
                    }


                    if (accessUrls.empty()) {

                        outputText +=
                            "No access_url found for RA=" +
                            RA + ", DEC=" + DEC + "\n";

                    }
                    else {

                        for (const auto& link : accessUrls) {
                            outputText += link + "\n";
                        }
                    }

                    std::cout << outputText << std::endl;

                    res.set_content(outputText, "text/plain");

                }
                else {

                    std::cout << "HTTP Error Status: "
                              << irsa_res->status << std::endl;

                    res.set_content("Please wait", "text/plain");
                }
            }
            else {

                std::cout << "IRSA connection failed or timed out!"
                          << std::endl;

                res.set_content(
                    "IRSA connection failed. Please try again.",
                    "text/plain"
                );
            }
        }
        else {

            res.set_content("Enter RA and DEC.", "text/plain");
        }
    });


    std::cout << "Running on http://127.0.0.1:8081" << std::endl;

    server.listen("127.0.0.1", 8081);

    return 0;
}
