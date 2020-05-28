

#include <RtioLog.h>
#include <iostream>
#include "IceClient.h"
#include "Listener.h"

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		std::cerr << "param error, exit." << std::endl;
		return -1;
	}
	const std::string config(argv[1]);

	auto t = time(NULL);
	std::cout << "server start "  << "@" << ctime(&t) << "config="<< config << std::endl;

	auto* iceClient = GlobalResources::IceClient::getInstance();

	bool isIceClientInitialed = false;
	for(int i = 0; i < AS_STARTUP_CONNECT_HUB_MAX_TIMES; ++i)
	{
		GlobalResources::RetCode ret = iceClient->init(argc, argv, config);
		if(GlobalResources::RetCode::Success == ret) // init success
		{
			isIceClientInitialed = true;
			break;
		}
		else if(GlobalResources::RetCode::FileExeption == ret)
		{
			break;
		}
		else
		{
			std::cerr << "iceClient->init try times=" << i + 1 << std::endl;
			sleep(1);
		}
	}
	if(!isIceClientInitialed)
	{
		std::cerr << "iceClient->init failed, server exit." << std::endl;
		return -1;
	}

	log2I(iceClient->getCommunicator(), "iceClient initialed.");

	// todo get config from ice config
    auto address = asio::ip::make_address("0.0.0.0");
    auto port = static_cast<unsigned short>(8090);
    auto const threads = std::max<int>(1, 5);

    // The io_context is required for all I/O
    asio::io_context ioc;
    // Create and launch a listening port
	std::make_shared<Listener>(ioc, tcp::endpoint{ address, port })->run();

	// Capture SIGINT and SIGTERM to perform a clean shutdown
	asio::signal_set signals(ioc, SIGINT, SIGTERM);
	signals.async_wait([&ioc](boost::system::error_code const&, int)
	{
		// Stop the io_context. This will cause run()
		// to return immediately, eventually destroying the
		// io_context and any remaining handlers in it.
			ioc.stop();
	});
    // Run the I/O service on the requested number of threads
	std::vector<std::thread> v;
	v.reserve(threads);
	for (auto i = threads; i > 0; --i)
	{
		v.emplace_back([&ioc]
		{
			ioc.run();
		});
	}
    // Block until all the threads exit
    for(auto& t : v)
    {
        t.join();
    }


    return EXIT_SUCCESS;
}
