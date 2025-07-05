#include "mesaac_arg_parser/arg_parser.hpp"

namespace mesaac::arg_parser {

int ArgParser::parse_args(int argc, const char **const argv) {
  CLIArgs args{argv, argv + argc};

  if (args.empty()) {
    std::cerr << "Internal Error: can't find program name in argv."
              << std::endl;
    return 2;
  }

  // Consume the program name.
  m_prog = std::filesystem::path(args.front()).filename();
  args.pop_front();

  while (!args.empty()) {
    if (m_help.parse(args).matched()) {
      show_usage();
      return 0;
    }
    try {
      // TODO find a way to logically chain ParseResults: r1 || r2 || r3.
      ParseResult result = parse_options(args);
      if (!result.matched()) {
        result = check_unknown_flag_opt(args);
      }
      if (!result.matched()) {
        result = parse_arguments(args);
      }

      if (result.error_msg()) {
        show_usage(result.error_msg().value());
        return 1;
      }

      if (!result.matched() && !args.empty()) {
        report_unused_args(args);
        return 1;
      }
    } catch (std::exception &e) {
      show_usage(e);
      return 1;
    }
  }

  auto msg = missing_args_msg();
  if (msg.has_value()) {
    show_usage(msg.value());
    return 1;
  }
  return 0;
}

ParseResult ArgParser::parse_options(CLIArgs &args) {
  for (const auto &opt : m_options) {
    auto result = opt->parse(args);
    if (result.matched()) {
      return result;
    }
  }
  return ParseResult::no_match();
}

ParseResult ArgParser::check_unknown_flag_opt(CLIArgs &args) {
  // If the first arg starts with a "-", it must be an unknown flag/option.
  if (!args.empty()) {
    const auto first_arg = args.front();
    if (first_arg.starts_with("-")) {
      // If it's a number, don't treat it as an unknown option.
      try {
        std::optional<double> val;
        value_converter::convert(first_arg, val);
      } catch (std::exception &e) {
        return ParseResult::match_with_error("Unknown flag/option '" +
                                             first_arg + "'");
      }
    }
  }
  return ParseResult::no_match();
}

ParseResult ArgParser::parse_arguments(CLIArgs &args) {
  for (const auto &argument : m_arguments) {
    auto result = argument->parse(args);
    if (result.matched()) {
      return result;
    }
  }
  return ParseResult::no_match();
}

void ArgParser::report_unused_args(const CLIArgs &args) const {
  std::ostringstream outs;
  outs << "Unexpected argument(s):";
  for (const auto &arg : args) {
    outs << " " << arg;
  }
  show_usage(outs.str());
}

std::optional<std::string> ArgParser::missing_args_msg() const {
  bool args_are_missing = false;
  std::ostringstream outs;
  outs << "Missing argument(s):";
  for (const auto &argument : m_arguments) {
    if (!argument->has_value()) {
      outs << " " << argument->name();
      args_are_missing = true;
    }
  }

  return args_are_missing ? outs.str() : std::optional<std::string>{};
}

void ArgParser::show_usage(std::exception &e) const { show_usage(e.what()); }

void ArgParser::show_usage(const std::string &error_msg) const {
  m_outs << error_msg << std::endl << std::endl;
  show_usage();
}

void ArgParser::show_usage() const {
  // TODO compose the sequence of flags, options, required positionals
  m_outs << "Usage: " << m_prog;

  // Show short syntax for each flag, option, positional.
  m_outs << " [" << m_help.usage() << "]";
  for (const auto &option : m_options) {
    m_outs << " [" << option->usage() << "]";
  }
  for (const auto &pos : m_arguments) {
    m_outs << " " << pos->usage();
  }
  m_outs << std::endl;

  m_outs << std::endl << m_description << std::endl << std::endl;

  // Show details for each arg
  m_outs << m_help.help() << std::endl;
  for (const auto &option : m_options) {
    m_outs << option->help() << std::endl;
  }
  for (const auto &pos : m_arguments) {
    m_outs << pos->help() << std::endl;
  }
}

} // namespace mesaac::arg_parser