cmake_minimum_required(VERSION 3.18)

project(Http)

add_executable(main main.cc HttpContext.cc HttpRequest.cc HttpResponse.cc HttpServer.cc util.cc ../Poller.cc ../Channel.cc ../EventLoop.cc ../TimerWheel.cc ../Connection.cc ../Any.cc ../Acceptor.cc ../TcpServer.cc)
