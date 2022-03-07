#pragma once
#include <iostream>
#include <fstream>
#include <boost/crc.hpp>

#include "thread_pool.h"

namespace signature_files
{

struct ISignatureFileCreator
{
	virtual ~ISignatureFileCreator() = default;
	virtual void Create(const std::string& inPath, const std::string& outPath, uint32_t blockSize) = 0;
};

class SignatureFileCreator final : public ISignatureFileCreator
{
	constexpr static const uint32_t DefaultBlockSize{1048576};
	constexpr static const uint32_t CountBlockForWriting{20};

public:

	void Create(const std::string& inPath, const std::string& outPath, uint32_t blockSize) override
	{
		if (!blockSize)
			blockSize = DefaultBlockSize;

		std::ifstream fileSteamIn{inPath, std::ios::binary};
		if (!fileSteamIn.is_open())
		{
			throw std::invalid_argument{std::string{"Is't open input file: "} + inPath};
		}

		std::ofstream fileSteamOut{outPath, std::ios::out | std::ios::binary | std::ios::trunc};
		if (!fileSteamOut.is_open())
		{
			throw std::invalid_argument{std::string{"Is't open output file: "} + outPath};
		}

		thread_utils::ThreadPool pool;

		std::vector<std::future<uint32_t>> futureResults;
		std::vector<std::future<uint32_t>> futureResultsTemp;
		futureResults.reserve(CountBlockForWriting);
		futureResultsTemp.reserve(CountBlockForWriting);

		std::future<void> futureWrite;
		do
		{
			std::vector<char> buffer(blockSize);
			fileSteamIn.read(buffer.data(), blockSize);

			futureResults.push_back(pool.Submit([buf = std::move(buffer)]{
					boost::crc_32_type result;
					result.process_bytes(buf.data(), buf.size());
					return result.checksum();
				}));

			if (futureResults.size() == CountBlockForWriting)
			{
				if (futureWrite.valid())
				{
					futureWrite.get();
				}

				std::swap(futureResults, futureResultsTemp);

				futureWrite = std::async(std::launch::async, [&futureResultsTemp, &fileSteamOut] {
					for (auto& f : futureResultsTemp)
					{
						fileSteamOut << f.get() << std::endl;
					}
					});

				futureResults.clear();
			}
		} while (fileSteamIn);

		if (!futureResults.empty())
		{
			for (auto& f : futureResults)
			{
				fileSteamOut << f.get() << std::endl;
			}
		}
	}
};

} //signature_files