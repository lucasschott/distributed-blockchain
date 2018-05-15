#include <iostream>
#include <RCF/RCF.hpp>
#include <string>

using namespace std;

string str="";

// Define the I_PrintService RCF interface.
RCF_BEGIN(I_NoeudBloc, "I_NoeudBloc")
    RCF_METHOD_V1(void, Print, const std::string &)
RCF_END(I_NoeudBloc)

// Server implementation of the I_PrintService RCF interface.
class NoeudBloc
{
    public:
        void Print(const std::string & s)
        {
            str = s;
        }
};


int main(int argc, char **argv)
{
    try
    {
        // Initialize RCF.
        RCF::RcfInitDeinit rcfInit;
        // Instantiate a RCF server.
        RCF::RcfServer server(RCF::TcpEndpoint(argv[1], atoi(argv[2])));
        // Bind the I_PrintService interface.
        NoeudBloc noeudBloc;
        server.bind<I_NoeudBloc>(noeudBloc);
        // Start the server.
        server.start();

        if(argc>3)
        {
            std::cout << "Calling the I_PrintService Print() method." << std::endl;
            // Instantiate a RCF client.
            RcfClient<I_NoeudBloc> client( RCF::TcpEndpoint(argv[3], atoi(argv[4])));
            // Connect to the server and call the Print() method.
            client.Print("Hello World");
        }
        else
        {
            while(str=="")
            {
                sleep(1);
            }
            cout << str << endl;
        }
    }
    catch ( const RCF::Exception & e )
    {
        std::cout << "Error: " << e.getErrorString() << std::endl;
    }
    return 0;
}
