
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <RCF/RCF.hpp>

#include "MyService.hpp"

class MyServiceImpl
{
public:
    // Reverses the order of strings in the vector.
    void reverse(std::vector<std::string> &v)
    {
        std::cout << "Reversing a vector of strings...\n";
        std::vector<std::string> w;
        std::copy(v.rbegin(), v.rend(), std::back_inserter(w));
        v.swap(w);
    }
};

int main()
{
    RCF::RcfInit rcfInit;

	std::string networkInterface = "0.0.0.0";
	int port = 50001;
	std::cout << "Starting server on " << networkInterface << ":" << port << "." << std::endl;

    // Start a TCP server, and expose MyServiceImpl.
    MyServiceImpl myServiceImpl;
    RCF::RcfServer server( RCF::TcpEndpoint(networkInterface, port) );
    server.bind<MyService>(myServiceImpl);
    server.start();

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}
