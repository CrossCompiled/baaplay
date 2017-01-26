#include <iostream>

#include <boost/program_options.hpp>
#include <boost/asio/io_service.hpp>

#include "AlsaPlayer.hpp"

int main(int argc, char* argv[])
{
    namespace po = boost::program_options;

    po::options_description desc;
    desc.add_options()
        ("input-file", po::value<std::string>(), "")
        ("help,h", "Show help")
        ;

    po::positional_options_description pd;
    pd.add("input-file", 1);

    po::variables_map options;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pd).run(), 
              options);
    po::notify(options);

    if (options.count("help") || options.count("input-file") != 1)
    {
        std::cerr << desc << std::endl;
        std::exit(0);
    }
    
    std::string filename = options["input-file"].as<std::string>();

    std::cerr << "Playing " << filename << std::endl;

    boost::asio::io_service ioService;

    baaplay::AlsaPlayer player(ioService, "default");
    player.Play(filename);

    ioService.run();

    return 0;
}
