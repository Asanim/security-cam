#include "webstream_server.h"

WebStreamServer::WebStreamServer(int port) : _port(port), _cap(0) {
    if (!_cap.isOpened()) {
        std::cerr << "Error: Could not open webcam." << std::endl;
        exit(EXIT_FAILURE);
    }
}

WebStreamServer::~WebStreamServer() {
    _server->stop();
    delete _server;
}

void WebStreamServer::initialize(Application& self) {
    loadConfiguration();
    ServerApplication::initialize(self);
}

void WebStreamServer::uninitialize() {
    ServerApplication::uninitialize();
}

int WebStreamServer::main(const std::vector<std::string>& args) {
    Poco::Net::ServerSocket svs(_port);
    Poco::Net::HTTPServerParams* pParams = new Poco::Net::HTTPServerParams;
    pParams->setMaxQueued(100);
    pParams->setMaxThreads(16);
    _server = new Poco::Net::HTTPServer(new RequestHandlerFactory(_cap), svs, pParams);
    _server->start();
    waitForTerminationRequest();
    return Application::EXIT_OK;
}

WebSocketRequestHandler::WebSocketRequestHandler(cv::VideoCapture& cap) : _cap(cap) {}

void WebSocketRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    try {
        Poco::Net::WebSocket ws(request, response);
        ws.setReceiveTimeout(Poco::Timespan(1, 0));
        ws.setSendTimeout(Poco::Timespan(1, 0));

        cv::Mat frame;
        std::vector<uchar> buf;
        std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 90};

        while (true) {
            _cap >> frame;
            if (frame.empty()) break;

            cv::imencode(".jpg", frame, buf, params);
            std::ostringstream oss;
            Poco::Base64Encoder b64enc(oss);
            b64enc.write(reinterpret_cast<const char*>(buf.data()), buf.size());
            b64enc.close();

            std::string encoded = oss.str();
            ws.sendFrame(encoded.data(), encoded.size(), Poco::Net::WebSocket::FRAME_TEXT);
        }
    } catch (Poco::Net::WebSocketException& exc) {
        std::cerr << "WebSocketException: " << exc.displayText() << std::endl;
    }
}

RequestHandlerFactory::RequestHandlerFactory(cv::VideoCapture& cap) : _cap(cap) {}

Poco::Net::HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest& request) {
    if (request.getURI() == "/") {
        return new WebSocketRequestHandler(_cap);
    } else {
        return nullptr;
    }
}
