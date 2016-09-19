#include <CSed.h>
#include <CFile.h>
#include <CStrUtil.h>
#include <CStrParse.h>
#include <cstdio>
#include <cstdlib>

CSed::
CSed() :
 line_num_(-1), last_line_(false), silent_(false), file_(0), command_list_(0)
{
}

bool
CSed::
addCommandFile(CFile &file)
{
  sfile_ = &file;

  std::string line;

  while (file.readLine(line)) {
    if (! addCommand(line))
      return false;
  }

  return true;
}

bool
CSed::
addCommand(const std::string &cmd)
{
  CStrParse parse(cmd);

  parse.skipSpace();

  CSedPointCondition *start_condition = 0;
  CSedPointCondition *end_condition   = 0;

  if (parse.isDigit()) {
    int start = -1;

    parse.readInteger(&start);

    start_condition = new CSedLineCondition(this, start);

    if (parse.isChar(',')) {
      parse.skipChar();

      if      (parse.isDigit()) {
        int end = -1;

        parse.readInteger(&end);

        end_condition = new CSedLineCondition(this, end);
      }
      else if (parse.isChar('$')) {
        parse.skipChar();

        end_condition = new CSedLineCondition(this, -1);
      }
      else if (parse.isChar('+')) {
        parse.skipChar();

        if (parse.isDigit()) {
          int end;

          parse.readInteger(&end);

          end += start;

          end_condition = new CSedLineCondition(this, end);
        }
        else {
          std::cerr << "Invalid offset" << std::endl;
          return false;
        }
      }
      else if (parse.isChar('/')) {
        parse.skipChar();

        std::string expr;

        while (! parse.eof() && ! parse.isChar('/')) {
          char c;

          parse.readChar(&c);

          if (c == '\\' && ! parse.eof()) {
            expr += c;

            parse.readChar(&c);
          }

          expr += c;
        }

        if (parse.eof()) {
          std::cerr << "Missing '/'" << std::endl;
          return false;
        }

        parse.skipChar();

        end_condition = new CSedRegExpCondition(this, expr);
      }
      else {
        std::cerr << "Invalid character after ','" << std::endl;
        return false;
      }
    }
  }
  else if (parse.isChar('$')) {
    parse.skipChar();

    start_condition = new CSedLineCondition(this, -1);
  }
  else if (parse.isChar('/')) {
    parse.skipChar();

    std::string expr;

    while (! parse.eof() && ! parse.isChar('/')) {
      char c;

      parse.readChar(&c);

      if (c == '\\' && ! parse.eof()) {
        expr += c;

        parse.readChar(&c);
      }

      expr += c;
    }

    if (parse.eof()) {
      std::cerr << "Missing '/'" << std::endl;
      return false;
    }

    parse.skipChar();

    start_condition = new CSedRegExpCondition(this, expr);

    if (parse.isChar(',')) {
      parse.skipChar();

      if      (parse.isDigit()) {
        int end = -1;

        parse.readInteger(&end);

        end_condition = new CSedLineCondition(this, end);
      }
      else if (parse.isChar('$')) {
        parse.skipChar();

        end_condition = new CSedLineCondition(this, -1);
      }
      else if (parse.isChar('/')) {
        parse.skipChar();

        std::string expr;

        while (! parse.eof() && ! parse.isChar('/')) {
          char c;

          parse.readChar(&c);

          if (c == '\\' && ! parse.eof()) {
            expr += c;

            parse.readChar(&c);
          }

          expr += c;
        }

        if (parse.eof()) {
          std::cerr << "Missing '/'" << std::endl;
          return false;
        }

        parse.skipChar();

        end_condition = new CSedRegExpCondition(this, expr);
      }
      else {
        std::cerr << "Invalid character after ','" << std::endl;
        return false;
      }
    }
  }

  if (parse.eof()) {
    std::cerr << "Invalid null command" << std::endl;
    return false;
  }

  //--------------

  char c;

  parse.readChar(&c);

  bool invert = false;

  if (c == '!' || c == '~') {
    invert = true;

    parse.readChar(&c);
  }

  CSedCondition *condition =
    new CSedCondition(this, start_condition, end_condition, invert);

  //--------------

  CSedCommand *command = 0;

  if      (c == '#') {
    command = new CSedCommentCommand(this, condition);

    parse.skipToEnd();
  }
  else if (c == 'q') {
    int exit_code = 0;

    parse.skipSpace();

    if (parse.isDigit())
      parse.readInteger(&exit_code);

    command = new CSedQuitCommand(this, condition, exit_code);
  }
  else if (c == 'd')
    command = new CSedDeleteCommand(this, condition);
  else if (c == 'D')
    command = new CSedDeleteFirstCommand(this, condition);
  else if (c == 'p')
    command = new CSedPrintCommand(this, condition);
  else if (c == 'P')
    command = new CSedPrintFirstCommand(this, condition);
  else if (c == '=')
    command = new CSedPrintLineNumCommand(this, condition);
  else if (c == 'n')
    command = new CSedNextCommand(this, condition);
  else if (c == 'N')
    command = new CSedAppendNextCommand(this, condition);
  else if (c == 'r') {
    if (! parse.isChar(' ')) {
      std::cerr << "Missing space after r" << std::endl;
      return false;
    }

    parse.skipChar();

    std::string filename = parse.getAt();

    parse.skipToEnd();

    command = new CSedReadCommand(this, condition, filename);
  }
  else if (c == 'w') {
    if (! parse.isChar(' ')) {
      std::cerr << "Missing space after w" << std::endl;
      return false;
    }

    parse.skipChar();

    std::string filename = parse.getAt();

    parse.skipToEnd();

    command = new CSedWriteCommand(this, condition, filename);
  }
  else if (c == '{') {
    if (command_list_ != 0)
      command_list_list_.push_back(command_list_);

    command_list_ = new CSedCommandList(this, condition);
  }
  else if (c == '}') {
    if (command_list_ == 0) {
      std::cerr << "Open close brace mismatch" << std::endl;
      return false;
    }

    commands_.push_back(command_list_);

    if (! command_list_list_.empty()) {
      command_list_ = command_list_list_.back();

      command_list_list_.pop_back();
    }
    else
      command_list_ = 0;
  }
  else if (c == 's') { // (,.,.)s[/<regexp>/<replace>//[g]] - substitute
    char sep;

    if (! parse.readChar(&sep)) {
      std::cerr << "Invalid 's' command" << std::endl;
      return false;
    }

    std::string find, replace;

    while (! parse.eof() && ! parse.isChar(sep)) {
      char c;

      parse.readChar(&c);

      if (c == '\\' && ! parse.eof()) {
        find += c;

        parse.readChar(&c);
      }

      find += c;
    }

    if (parse.eof()) {
      std::cerr << "Missing '" << sep << "'" << std::endl;
      return false;
    }

    parse.skipChar();

    while (! parse.eof() && ! parse.isChar(sep)) {
      char c;

      parse.readChar(&c);

      if (c == '\\' && ! parse.eof()) {
        replace += c;

        parse.readChar(&c);
      }

      replace += c;
    }

    if (parse.eof()) {
      std::cerr << "Missing '" << sep << "'" << std::endl;
      return false;
    }

    parse.skipChar();

    parse.skipSpace();

    bool global = false;
    bool print  = false;

    if (! parse.eof()) {
      if (parse.isDigit()) {
        int n;

        parse.readInteger(&n);
      }
      else {
        char c;

        parse.readChar(&c);

        if      (c == 'g')
          global = true;
        else if (c == 'p')
          print = true;
        else if (c == 'w')
          ; // write
        else if (c == 'e')
          ; // execute
        else {
          std::cerr << "Invalid substitute modifier" << std::endl;
          return false;
        }
      }
    }

    command = new CSedSubstituteCommand(this, condition, find, replace, global, print);
  }
  else if (c == 'y') {
    if (parse.eof()) {
      std::cerr << "Invalid 'y' command" << std::endl;
      return false;
    }

    char sep;

    parse.readChar(&sep);

    std::string lhs, rhs;

    while (! parse.eof() && ! parse.isChar(sep)) {
      char c;

      parse.readChar(&c);

      if (c == '\\' && ! parse.eof()) {
        lhs += c;

        parse.readChar(&c);
      }

      lhs += c;
    }

    if (parse.eof()) {
      std::cerr << "Missing '" << sep << "'" << std::endl;
      return false;
    }

    parse.skipChar();

    while (! parse.eof() && ! parse.isChar(sep)) {
      char c;

      parse.readChar(&c);

      if (c == '\\' && ! parse.eof()) {
        rhs += c;

        parse.readChar(&c);
      }

      rhs += c;
    }

    if (parse.eof()) {
      std::cerr << "Missing '" << sep << "'" << std::endl;
      return false;
    }

    parse.skipChar();

    parse.skipSpace();

    if (! parse.eof()) {
      std::cerr << "Invalid 'y' command" << std::endl;
      return false;
    }

    command = new CSedTranslateCommand(this, condition, lhs, rhs);
  }
  else if (c == 'h')
    command = new CSedToHoldCommand(this, condition);
  else if (c == 'H')
    command = new CSedToHoldNewlineCommand(this, condition);
  else if (c == 'g')
    command = new CSedToPatternCommand(this, condition);
  else if (c == 'G')
    command = new CSedToPatternNewlineCommand(this, condition);
  else if (c == 'x')
    command = new CSedExchangeCommand(this, condition);
  else if (c == 'l')
    command = new CSedListCommand(this, condition);
  else if (c == 'a') {
    if (! parse.isChar('\\')) {
      std::cerr << "Expected '\\' after 'a'" << std::endl;
      return false;
    }

    parse.skipChar();

    if (! parse.eof()) {
      std::cerr << "Invalid extra characters '" <<
              cmd.substr(parse.getPos()) << "'" << std::endl;
      return false;
    }

    std::string line, line1;

    while (sfile_ && sfile_->readLine(line1)) {
      line += line1;

      if (line1.empty() || line1[line1.size() - 1] != '\\')
        break;

      line = line.substr(0, line.size() - 1) + "\n";
    }

    command = new CSedAppendCommand(this, condition, line);
  }
  else if (c == 'i') {
    if (! parse.isChar('\\')) {
      std::cerr << "Expected '\\' after 'a'" << std::endl;
      return false;
    }

    parse.skipChar();

    if (! parse.eof()) {
      std::cerr << "Invalid extra characters '" <<
              cmd.substr(parse.getPos()) << "'" << std::endl;
      return false;
    }

    std::string line, line1;

    while (sfile_ && sfile_->readLine(line1)) {
      line += line1;

      if (line1.empty() || line1[line1.size() - 1] != '\\')
        break;

      line = line.substr(0, line.size() - 1) + "\n";
    }

    command = new CSedInsertCommand(this, condition, line);
  }
  else if (c == 'c') {
    if (! parse.isChar('\\')) {
      std::cerr << "Expected '\\' after 'a'" << std::endl;
      return false;
    }

    parse.skipChar();

    if (! parse.eof()) {
      std::cerr << "Invalid extra characters '" <<
              cmd.substr(parse.getPos()) << "'" << std::endl;
      return false;
    }

    std::string line, line1;

    while (sfile_ && sfile_->readLine(line1)) {
      line += line1;

      if (line1.empty() || line1[line1.size() - 1] != '\\')
        break;

      line = line.substr(0, line.size() - 1) + "\n";
    }

    command = new CSedChangeCommand(this, condition, line);
  }
  else {
    std::cerr << "Invalid command '" << c << "'" << std::endl;
    return false;
  }

  if (! command)
    return true;

  if (command_list_ != 0)
    command_list_->addCommand(command);
  else
    commands_.push_back(command);

  if      (parse.isChar(';')) {
    parse.skipChar();

    return addCommand(cmd.substr(parse.getPos()));
  }
  else if (! parse.eof()) {
    std::cerr << "Invalid extra characters '" <<
            cmd.substr(parse.getPos()) << "'" << std::endl;
    return false;
  }

  return true;
}

bool
CSed::
process(CFile &file)
{
  file_ = &file;

  line_num_ = 0;

  while (nextLine()) {
    if (! executeCommands()) continue;

    if (! silent_)
      std::cout << pattern_ << std::endl;
  }

  return true;
}

bool
CSed::
nextLine()
{
  ++line_num_;

  std::string line;

  if (! file_ || ! file_->readLine(line))
    return false;

  pattern_ = line;

  int c;

  if ((c = file_->getC()) != EOF)
    file_->ungetC(c);

  last_line_ = file_->eof();

  return true;
}

bool
CSed::
executeCommands()
{
  CommandList::const_iterator p1 = commands_.begin();
  CommandList::const_iterator p2 = commands_.end  ();

  for ( ; p1 != p2; ++p1) {
    if (! (*p1)->check()) continue;

    if (! (*p1)->execute())
      return false;
  }

  return true;
}

//------

CSedCondition::
CSedCondition(CSed *sed, CSedPointCondition *start_condition,
              CSedPointCondition *end_condition, bool invert) :
 sed_(sed), command_(0), start_condition_(start_condition), end_condition_(end_condition),
 invert_(invert), inside_(false), position_(NO_LINE)
{
}

bool
CSedCondition::
execute()
{
  bool old_inside = inside_;

  inside_ = execute1();

  if (invert_)
    inside_ = ! inside_;

  if (! old_inside) {
    if (inside_)
      position_ = START_LINE;
    else
      position_ = NO_LINE;
  }
  else {
    if (end_triggered_ || sed_->isLastLine())
      position_ = END_LINE;
    else
      position_ = MID_LINE;
  }

  return inside_;
}

bool
CSedCondition::
execute1()
{
  if (! start_condition_ && ! end_condition_)
    return true;

  if (hasRange()) {
    if (! inside_) {
      end_triggered_ = false;

      return start_condition_->execute();
    }
    else {
      if (! end_triggered_) {
        end_triggered_ = end_condition_->execute();

        return true;
      }
      else
        return false;
    }
  }
  else
    return start_condition_->execute();
}

//------

bool
CSedLineCondition::
execute()
{
  if (line_ == -1)
    return sed_->isLastLine();

  return ((int) sed_->getLineNum() == line_);
}

bool
CSedRegExpCondition::
execute()
{
  return expr_.find(sed_->getPattern());
}

//------

CSedCommand::
CSedCommand(CSed *sed, CSedCondition *condition) :
 sed_(sed), condition_(condition)
{
  condition_->setCommand(this);
}

bool
CSedCommand::
check()
{
  return condition_->execute();
}

//------

bool
CSedCommentCommand::
execute()
{
  return true;
}

bool
CSedDeleteCommand::
execute()
{
  return false;
}

bool
CSedDeleteFirstCommand::
execute()
{
  std::string::size_type pos = sed_->getPattern().find('\n');

  if (pos == std::string::npos)
    return false;

  sed_->setPattern(sed_->getPattern().substr(pos + 1));

  return true;
}

bool
CSedNextCommand::
execute()
{
  if (! sed_->getSilent())
    std::cout << sed_->getPattern() << std::endl;

  sed_->nextLine();

  return true;
}

bool
CSedAppendNextCommand::
execute()
{
  std::string line = sed_->getPattern();

  if (! sed_->nextLine())
    return false;

  sed_->setPattern(line + "\n" + sed_->getPattern());

  return true;
}

bool
CSedReadCommand::
execute()
{
  CFile file(filename_);

  std::string line;

  while (file.readLine(line))
    std::cout << line << std::endl;

  return true;
}

bool
CSedWriteCommand::
execute()
{
  if (! file_.isValid())
    file_ = new CFile(filename_);

  std::string line;

  file_->write(sed_->getPattern() + "\n");

  return true;
}

bool
CSedPrintCommand::
execute()
{
  std::cout << sed_->getPattern() << std::endl;

  return true;
}

bool
CSedPrintFirstCommand::
execute()
{
  std::string::size_type pos = sed_->getPattern().find('\n');

  if (pos == std::string::npos)
    std::cout << sed_->getPattern() << std::endl;
  else
    std::cout << sed_->getPattern().substr(pos + 1) << std::endl;

  return true;
}

bool
CSedPrintLineNumCommand::
execute()
{
  std::cout << sed_->getLineNum() << std::endl;

  return true;
}

bool
CSedQuitCommand::
execute()
{
  exit(exit_code_);
}

bool
CSedSubstituteCommand::
execute()
{
  std::string pattern = sed_->getPattern();

  if (find_.find(pattern)) {
    pattern = find_.replace(replace_, global_);

    sed_->setPattern(pattern);
  }

  return true;
}

bool
CSedTranslateCommand::
execute()
{
  sed_->setPattern(CStrUtil::translate(sed_->getPattern(), lhs_, rhs_));

  return true;
}

bool
CSedToHoldCommand::
execute()
{
  sed_->setHold(sed_->getPattern());

  return true;
}

bool
CSedToHoldNewlineCommand::
execute()
{
  sed_->setHold(sed_->getHold() + "\n" + sed_->getPattern());

  return true;
}

bool
CSedToPatternCommand::
execute()
{
  sed_->setPattern(sed_->getHold());

  return true;
}

bool
CSedToPatternNewlineCommand::
execute()
{
  sed_->setPattern(sed_->getPattern() + "\n" + sed_->getHold());

  return true;
}

bool
CSedExchangeCommand::
execute()
{
  std::string pattern = sed_->getPattern();

  sed_->setPattern(sed_->getHold());
  sed_->setHold   (pattern);

  return true;
}

bool
CSedListCommand::
execute()
{
  std::cout << CStrUtil::insertEscapeCodes(sed_->getPattern()) << '$' << std::endl;

  return true;
}

bool
CSedAppendCommand::
execute()
{
  sed_->setPattern(sed_->getPattern() + "\n" + line_);

  return true;
}

bool
CSedInsertCommand::
execute()
{
  sed_->setPattern(line_ + "\n" + sed_->getPattern());

  return true;
}

bool
CSedChangeCommand::
execute()
{
  if (! condition_->hasRange())
    sed_->setPattern(line_);
  else {
    if (condition_->getPosition() != CSedCondition::END_LINE)
      return false;

    sed_->setPattern(line_);
  }

  return true;
}

bool
CSedCommandList::
execute()
{
  CommandList::const_iterator p1 = commands_.begin();
  CommandList::const_iterator p2 = commands_.end  ();

  for ( ; p1 != p2; ++p1) {
    if (! (*p1)->check()) continue;

    if (! (*p1)->execute())
      return false;
  }

  return true;
}
