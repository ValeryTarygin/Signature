#include <iostream>
#include <chrono>
#include <memory>
#include <boost/program_options.hpp>

#include "../include/signature_file_creator.h"

using namespace signature_files;
using namespace boost::program_options;

struct CommandArg
{
    std::string inPath;
    std::string outPath;
    uint32_t blockSize{};
};

CommandArg GetArgumentsFromCommandLine(int argc, char const* argv[])
{
    CommandArg valueArgs;

    options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "example command: -i 'fileName' -o 'fileName' -s(optional) 1024 ")
        ("in,i", value(&valueArgs.inPath), "Input file")
        ("out,o", value(&valueArgs.outPath), "Output file")
        ("blockSize,s", value(&valueArgs.blockSize), "Block size");

    variables_map vm;
    store(command_line_parser(argc, argv).options(desc).run(), vm);
    notify(vm);

    if (vm.count("help") || !vm.count("in") || !vm.count("out"))
    {
        std::cerr << desc << "\n";
        throw std::invalid_argument{ std::string{"Incorrect arguments in command line"} };
    }

    return valueArgs;
}

int main(int argc, char const* argv[]) try
{
    auto valueArgs = GetArgumentsFromCommandLine(argc, argv);

    const auto tStart = std::chrono::high_resolution_clock::now();
    std::unique_ptr<ISignatureFileCreator> signatureCreator = std::make_unique<SignatureFileCreator>();
    signatureCreator->Create(valueArgs.inPath, valueArgs.outPath, valueArgs.blockSize);
    const auto tEnd = std::chrono::high_resolution_clock::now();

    std::cout << "Result time: " << std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count() << std::endl;

	return EXIT_SUCCESS;
}
catch (std::exception& ex)
{
	std::cerr << "Caught exception:\n"
		<< ex.what()
		<< '\n';
	std::exit(EXIT_FAILURE);
}
catch (...)
{
	std::cerr << "Caught unknown exception\n";
	std::exit(EXIT_FAILURE);
}