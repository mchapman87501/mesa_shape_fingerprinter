#include <catch2/catch_test_macros.hpp>

#include "mesaac_arg_parser/arg_parser.hpp"

#include <sstream>
namespace mesaac::arg_parser {
namespace {

struct TestParams {
  Flag::Ptr flag = Flag::create("-f", "--flag", "Enable flag");
  Flag::Ptr verbose = Flag::create("-v", "--verbose", "Be noisy");

  Option<unsigned int>::Ptr opt_unsigned =
      Option<unsigned int>::create("-u", "--unsigned", "An unsigned optional");

  Argument<std::string>::Ptr sval =
      Argument<std::string>::create("sval", "A string value");
  Argument<float>::Ptr fval = Argument<float>::create("fval", "A float value");

  [[nodiscard]] ArgParser get_parser(std::ostream &outs) {
    return ArgParser({flag, verbose, opt_unsigned}, {sval, fval},
                     "Description goes here", outs);
  }
};

TEST_CASE("mesaac::arg_parser::ArgParser", "[mesaac][mesaac_arg_parser]") {
  SECTION("Test instantiation") {
    const std::string description("Description goes here.");
    ArgParser parser({}, {}, description);
  }

  SECTION("Test the impossible - no args at all") {
    TestParams test_params;
    auto parser = test_params.get_parser(std::cerr);

    const char *argv[] = {};
    const int argc = sizeof(argv) / sizeof(argv[0]);

    REQUIRE(parser.parse_args(argc, argv) == 2);
  }

  SECTION("Usage") {
    TestParams test_params;
    std::ostringstream outs;
    auto parser = test_params.get_parser(outs);
    parser.show_usage();

    // FRAGILE!
    std::string expected =
        R"USAGE(Usage: <program> [-h | --help] [-f | --flag] [-v | --verbose] [-u UNSIGNED | --unsigned UNSIGNED] sval fval

Description goes here

-h | --help
        Show this help message and exit
-f | --flag
        Enable flag
-v | --verbose
        Be noisy
-u UNSIGNED | --unsigned UNSIGNED
        An unsigned optional
sval
        A string value
fval
        A float value
)USAGE";

    REQUIRE(outs.str() == expected);
  }

  SECTION("Parsing w. no flags.") {
    TestParams test_params;
    auto parser = test_params.get_parser(std::cerr);

    const std::string sval("string-valued arg");
    const std::string fval_str("42.0");
    const float fval(42.0);

    const char *argv[] = {"<test_prog>", sval.c_str(), fval_str.c_str()};
    const int argc = sizeof(argv) / sizeof(argv[0]);

    REQUIRE(parser.parse_args(argc, argv) == 0);
    REQUIRE(test_params.sval->value() == sval);
    REQUIRE(test_params.fval->value() == fval);
  }

  SECTION("Parsing w. --help") {
    TestParams test_params;
    std::ostringstream outs;
    auto parser = test_params.get_parser(outs);

    const char *argv[] = {"<test_prog>", "--help"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    REQUIRE(parser.parse_args(argc, argv) == 0);
    REQUIRE(parser.usage_requested());
  }

  SECTION("Parsing w. too many positionals") {
    TestParams test_params;
    std::ostringstream outs;
    auto parser = test_params.get_parser(outs);

    const char *argv[] = {"<test_prog>", "sval", "42.0", "extra"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    REQUIRE(parser.parse_args(argc, argv) != 0);
  }

  SECTION("Parsing w. too few positionals") {
    TestParams test_params;
    std::ostringstream outs;

    auto parser = test_params.get_parser(outs);

    const char *argv[] = {"<test_prog>", "sval"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    REQUIRE(parser.parse_args(argc, argv) != 0);
  }

  SECTION("Parsing w. all valid args") {
    TestParams test_params;
    auto parser = test_params.get_parser(std::cerr);
    const char *argv[] = {"<test_prog>", "-f",   "--verbose", "-u",
                          "12",          "sval", "42.0"};
    const int argc = sizeof(argv) / sizeof(argv[0]);

    REQUIRE(parser.parse_args(argc, argv) == 0);
    REQUIRE(test_params.flag->value() == true);
    REQUIRE(test_params.verbose->value() == true);
    REQUIRE(test_params.opt_unsigned->value() == 12);
    REQUIRE(test_params.sval->value() == "sval");
    REQUIRE(test_params.fval->value() == 42.0f);
    REQUIRE(!parser.usage_requested());
  }

  SECTION("Parse with unknown flag/option") {
    TestParams test_params;
    std::ostringstream outs;
    auto parser = test_params.get_parser(outs);
    const char *argv[] = {"<test_prog>", "-malformed", "sval", "42.0"};
    const int argc = sizeof(argv) / sizeof(argv[0]);

    REQUIRE(parser.parse_args(argc, argv) != 0);
    const auto diagnostics(outs.str());

    const auto index = diagnostics.find("-malformed");
    if (index == std::string::npos) {
      std::cerr << "DEBUG: parse with unknown flag/option:" << std::endl
                << "error output:" << std::endl
                << diagnostics << std::endl;
    }
    REQUIRE(diagnostics.find("-malformed") != std::string::npos);
  }
}

} // namespace
} // namespace mesaac::arg_parser
