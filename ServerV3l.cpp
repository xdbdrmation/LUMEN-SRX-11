#include <iostream>
#include <vector>
#include <string>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <fstream>
#include <CCfits/CCfits>
#include <cstring>
#include "tinyxml2.h"

// http://127.0.0.1:8081 use this btw

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
                    </style>
                </head>
                <body>
                    <h1> LUMEN </h1>
                    <p class="Main_text"> Map out the space </p>
                    <p class="Note"> NOTE: Please enter the Right Ascension and Declination in DECIMAL FORMAT! </p>
                    <form action="/process" method="post">
                        <input type="text" name="RA" placeholder="Enter Right Ascension" class="RA">
                        <input type="text" name="DEC" placeholder="Enter Declination" class="DEC">
                        <br>
                        <button type="submit"> SUBMIT </button>
                    </form>
                </body>
            </html>
        )", "text/html");
    });

    server.Post("/process", [](const httplib::Request &req, httplib::Response &res) {

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

            // cli.enable_server_certificate_verification(false);

            if (auto irsa_res = cli.Get(reqcsv.c_str())) {

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

                                    for (int i = 0; i < fieldnames.size(); i++) {

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
                   for( const auto& fitsUrl : accessUrls ) {
                        const std::string prefix = "https://irsa.ipac.caltech.edu";

                        if (fitsUrl.rfind(prefix, 0) == 0){
                            std::cout << "Prefix found in URL: " << fitsUrl << std::endl;
                        }
                        else {
                            std::cout << "Prefix not found in URL: " << fitsUrl << std::endl;
                        }

                        if (fitsUrl.rfind(prefix, 0) == 0){
                            std::string path = fitsUrl.substr(prefix.length());
                            httplib::SSLClient fitsClient("irsa.ipac.caltech.edu");
                            fitsClient.set_follow_location(true);
                            auto fitsResponse = fitsClient.Get(path.c_str());
                            if (!fitsResponse){
                                std::cout << "FITS Connection Error:" << std::endl;
                            }
                            else if (fitsResponse->body.empty()){
                                std::cout <<"FITS response is empty!" << std::endl;
                            }
                            else {
                                std::cout << "FITS response received successfully! Size: " << fitsResponse->body.size() << " bytes" << std::endl;
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

                    res.set_content("Please wait" , "text/plain"    
                    );
                }

            }
            else {

                std::cout << "Connection Error: "
                          << irsa_res.error() << std::endl;

                res.set_content(
                    "IRSA connection failed. Check terminal.",
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