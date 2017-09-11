//
//  version.hpp
//  Semver
//
//  Copyright © 2017 Dsperados (info@dsperados.com). All rights reserved.
//  Licensed under the BSD 3-clause license.
//

#ifndef MODITONE_SEMVER_HPP
#define MODITONE_SEMVER_HPP

#include <cctype>
#include <istream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <ostream>
#include <vector>

#include <moditone/jsonata/value.hpp>

namespace semver
{
	//! Comparable version for software, modeled after http://semver.org/
	class Version
	{
	public:
		//! Construct a 0.0.0 version
		Version() = default;

		//! Construct a version by passing the elements explicitly
		Version(unsigned int major, unsigned int minor, unsigned int patch, const std::vector<std::string>& prerelease = {}, const std::vector<std::string>& build = {}) :
			major(major),
			minor(minor),
			patch(patch),
			prerelease(prerelease),
			build(build)
		{
			for (auto& element : prerelease)
			{
				if (element.empty())
					throw std::invalid_argument("semver prerelease element may not be empty");
			}

			for (auto& element : build)
			{
				if (element.empty())
					throw std::invalid_argument("semver build element may not be empty");
			}
		}

		Version(const char* str)
		{
			std::istringstream stream(str);
			parse(stream);
		}

		Version(const std::string& str)
		{
			std::istringstream stream(str);
			parse(stream);
		}

		Version(std::istringstream& stream)
		{
			parse(stream);
		}

		//! Construct a version from json
		Version(const json::Value& json)
		{
			if (!json.isObject())
				throw std::invalid_argument("semver json is not an object");

			if (!json.hasKey("major") || !json["major"].isUnsignedInteger())
				throw std::invalid_argument("semver json does not contain a 'major' positive integer");
			major = static_cast<unsigned int>(json["major"].asUnsignedInteger());

			if (!json.hasKey("minor") || !json["minor"].isUnsignedInteger())
				throw std::invalid_argument("semver json does not contain a 'minor' positive integer");
			minor = static_cast<unsigned int>(json["minor"].asUnsignedInteger());

			if (!json.hasKey("patch") || !json["patch"].isUnsignedInteger())
				throw std::invalid_argument("semver json does not contain a 'patch' positive integer");
			patch = static_cast<unsigned int>(json["patch"].asUnsignedInteger());

			if (json.hasKey("prerelease"))
			{
				if (!json["prerelease"].isArray())
					throw std::invalid_argument("semver json 'prerelease' is not an array");

				for (auto& element : json["prerelease"].asArray())
				{
					if (!element.isString())
						throw std::invalid_argument("semver json 'prerelease' contains a non-string element");

					if (element.asString().empty())
						throw std::invalid_argument("semver json 'prerelease' element may not be empty");

					prerelease.emplace_back(element.asString());
				}
			}

			if (json.hasKey("build"))
			{
				if (!json["build"].isArray())
					throw std::invalid_argument("semver json 'build' is not an array");

				for (auto& element : json["build"].asArray())
				{
					if (!element.isString())
						throw std::invalid_argument("semver json 'build' contains a non-string element");

					if (element.asString().empty())
						throw std::invalid_argument("semver json 'build' element may not be empty");

					build.emplace_back(element.asString());
				}
			}
		}

		//! Convert the version to string
		std::string toString() const
		{
			std::ostringstream stream;
			stream << major << '.' << minor << '.' << patch;

			if (!prerelease.empty())
			{
				stream << "-";
				for (auto i = 0; i < prerelease.size() - 1; ++i)
					stream << prerelease[i] << '.';
				stream << prerelease.back();
			}

			if (!build.empty())
			{
				stream << "+";
				for (auto i = 0; i < build.size() - 1; ++i)
					stream << build[i] << '.';
				stream << build.back();
			}

			return stream.str();
		}

		//! Convert the version to json
		json::Value toJson() const
		{
			json::Value::Object json;

			json["major"] = major;
			json["minor"] = minor;
			json["patch"] = patch;

			if (!prerelease.empty())
			{
				for (auto& element : prerelease)
					json["prerelease"].append(element);
			}

			if (!build.empty())
			{
				for (auto& element : build)
					json["build"].append(element);
			}

			return json;
		}

	public:
		//! Major version, for making incompatible API changes
		unsigned int major = 0;

		//! Minor version when you add functionality in a backwards-compatible manner
		unsigned int minor = 0;

		//! Patch version when you make backwards-compatible bug fixes
		unsigned int patch = 0;

		//! Prerelease tags
		/*! These will be lexicographically compared if major, minor and patch are equal */
		std::vector<std::string> prerelease;

		//! Build tags
		/*! These do not feature in comparisons */
		std::vector<std::string> build;

	private:
		void parse(std::istringstream& stream)
		{
			stream >> major;
			if (stream.get() != '.')
				throw std::runtime_error("unexpected character in version string");

			stream >> minor;
			if (stream.get() != '.')
				throw std::runtime_error("unexpected character in version string");

			stream >> patch;

			if (stream.peek() == '-')
			{
				stream.ignore();

				prerelease.emplace_back();
				while (true)
				{
					if (std::isalnum(stream.peek()))
					{
						prerelease.back() += stream.get();
					} else if (stream.peek() == '.') {
						stream.ignore();
						prerelease.emplace_back();
					} else {
						break;
					}
				}
			}
		}
	};

	//! Compare two versions for equality
	inline static bool operator==(const Version& lhs, const Version& rhs)
	{
		return (lhs.major == rhs.major) && (lhs.minor == rhs.minor) && (lhs.patch == rhs.patch) && (lhs.prerelease == rhs.prerelease);
	}

	//! Compare two versions for inequality
	inline static bool operator!=(const Version& lhs, const Version& rhs)
	{
		return !(lhs == rhs);
	}

	//! Compare two versions for ordinality
	inline static bool operator<(const Version& lhs, const Version& rhs)
	{
		if (lhs.major < rhs.major)
			return true;
		else if (lhs.major > rhs.major)
			return false;

		if (lhs.minor < rhs.minor)
			return true;
		else if (lhs.minor > rhs.minor)
			return false;

		if (lhs.patch < rhs.patch)
			return true;
		else if (lhs.patch > rhs.patch)
			return false;

		if (lhs.prerelease < rhs.prerelease)
			return true;
		else if (lhs.prerelease > rhs.prerelease)
			return false;

		return false;
	}

	//! Compare two versions for ordinality or equality
	inline static bool operator<=(const Version& lhs, const Version& rhs)
	{
		return (lhs == rhs) || (lhs < rhs);
	}

	//! Compare two versions for ordinality
	inline static bool operator>(const Version& lhs, const Version& rhs)
	{
		return !(lhs <= rhs);
	}

	//! Compare two versions for ordinality or equality
	inline static bool operator>=(const Version& lhs, const Version& rhs)
	{
		return !(lhs < rhs);
	}

	//! Write a version to an output stream
	inline static std::ostream& operator<<(std::ostream& stream, const Version& version)
	{
		return stream << version.toString();
	}
}

#endif
