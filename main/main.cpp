#include <iostream>
#include <map>

#include "dtls/dtls_socket.h"
#include "main/ffmpeg_src.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/http/HttpRequest.h"
#include "muduo/net/http/HttpResponse.h"
#include "muduo/net/http/HttpServer.h"
#include "webrtctransport/utils.h"
#include "webrtctransport/webrtc_transport.h"

using namespace muduo;
using namespace muduo::net;

static std::map<int, std::shared_ptr<WebRtcTransport>> s_WebRTCSession;
static int s_sessionid = 1;

int main(int argc, char* argv[]) {
  std::string strIP = "192.168.2.131";
  if (argc > 1) {
    strIP = argv[1];
  }
  FFmpegSrc::GetInsatance()->Start();
  Utils::Crypto::ClassInit();
  dtls::DtlsSocketContext::Init();

  int threads_num = 0;
  EventLoop loop;
  HttpServer server(&loop, InetAddress(8000), "webrtc", TcpServer::kReusePort);
  server.setHttpCallback([&loop, &strIP](const HttpRequest& req, HttpResponse* resp) {
    if (req.path() == "/webrtc") {
      resp->setStatusCode(HttpResponse::k200Ok);
      resp->setStatusMessage("OK");
      resp->setContentType("text/plain");
      resp->addHeader("Access-Control-Allow-Origin", "*");
      std::shared_ptr<WebRtcTransport> session(new WebRtcTransport(&loop, strIP));
      s_WebRTCSession.insert(std::make_pair(s_sessionid, session));
      s_sessionid++;
      session->Start();

      FFmpegSrc::GetInsatance()->AddClient(session);
      resp->setBody(session->GetLocalSdp());
      std::cout << session->GetLocalSdp() << std::endl;
    }
  });
  server.setThreadNum(threads_num);
  server.start();
  loop.loop();
}
