#ifndef CSED_H
#define CSED_H

#include <CRegExp.h>
#include <memory>
#include <iostream>
#include <sys/types.h>

class CFile;

class CSed;
class CSedCommand;

class CSedPointCondition {
 public:
  CSedPointCondition(CSed *sed) :
   sed_(sed) {
  }

  virtual ~CSedPointCondition() { }

  virtual bool execute() { return true; }

 protected:
  CSed *sed_;
};

//---

class CSedCondition {
 public:
  enum Position {
    NO_LINE,
    START_LINE,
    MID_LINE,
    END_LINE
  };

 public:
  CSedCondition(CSed *sed, CSedPointCondition *start_condition,
                CSedPointCondition *end_condition, bool invert);

  void setCommand(CSedCommand *command) { command_ = command; }

  bool getInside() const { return inside_; }
  void setInside(bool inside) { inside_ = inside; }

  Position getPosition() const { return position_; }
  void setPosition(Position position) { position_ = position; }

  bool hasRange() const { return (end_condition_ != NULL); }

  bool execute();

 private:
  bool execute1();

 private:
  CSed               *sed_;
  CSedCommand        *command_;
  CSedPointCondition *start_condition_;
  CSedPointCondition *end_condition_;
  bool                invert_;
  bool                inside_;
  bool                end_triggered_;
  Position            position_;
};

//---

class CSedLineCondition : public CSedPointCondition {
 public:
  CSedLineCondition(CSed *sed, int line) :
   CSedPointCondition(sed), line_(line) {
  }

  bool execute() override;

 private:
  int line_;
};

//---

class CSedRegExpCondition : public CSedPointCondition {
 public:
  CSedRegExpCondition(CSed *sed, const std::string &expr) :
   CSedPointCondition(sed), expr_(expr) {
    //expr_.setMatchBOL(false);
    //expr_.setMatchEOL(false);
  }

  bool execute() override;

 private:
  CRegExp expr_;
};

//---

class CSedCommand {
 public:
  CSedCommand(CSed *sed, CSedCondition *condition);

  virtual ~CSedCommand() { }

  bool check();

  virtual bool execute() {
    std::cerr << "Not implemented" << std::endl;
    return false;
  }

 protected:
  CSed          *sed_;
  CSedCondition *condition_;
};

//---

class CSedCommentCommand : public CSedCommand {
 public:
  CSedCommentCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedDeleteCommand : public CSedCommand {
 public:
  CSedDeleteCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedDeleteFirstCommand : public CSedCommand {
 public:
  CSedDeleteFirstCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedNextCommand : public CSedCommand {
 public:
  CSedNextCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedAppendNextCommand : public CSedCommand {
 public:
  CSedAppendNextCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedReadCommand : public CSedCommand {
 public:
  CSedReadCommand(CSed *sed, CSedCondition *condition, const std::string &filename) :
   CSedCommand(sed, condition), filename_(filename) {
  }

  bool execute() override;

 private:
  std::string filename_;
};

//---

class CSedWriteCommand : public CSedCommand {
 public:
  CSedWriteCommand(CSed *sed, CSedCondition *condition, const std::string &filename) :
   CSedCommand(sed, condition), filename_(filename) {
  }

  bool execute() override;

 private:
  using FileP = std::unique_ptr<CFile>;

  std::string filename_;
  FileP       file_;
};

//---

class CSedPrintCommand : public CSedCommand {
 public:
  CSedPrintCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedPrintFirstCommand : public CSedCommand {
 public:
  CSedPrintFirstCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedPrintLineNumCommand : public CSedCommand {
 public:
  CSedPrintLineNumCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedQuitCommand : public CSedCommand {
 public:
  CSedQuitCommand(CSed *sed, CSedCondition *condition, int exit_code) :
   CSedCommand(sed, condition), exit_code_(exit_code) {
  }

  bool execute() override;

 private:
  int exit_code_;
};

//---

class CSedSubstituteCommand : public CSedCommand {
 public:
  CSedSubstituteCommand(CSed *sed, CSedCondition *condition, const std::string &find,
                        const std::string &replace, bool global, bool print) :
   CSedCommand(sed, condition), find_(find), replace_(replace), global_(global), print_(print) {
    //find_.setMatchBOL(false);
    //find_.setMatchEOL(false);
  }

  bool isPrint() const { return print_; }

  bool execute() override;

 private:
  CRegExp     find_;
  std::string replace_;
  bool        global_;
  bool        print_;
};

//---

class CSedTranslateCommand : public CSedCommand {
 public:
  CSedTranslateCommand(CSed *sed, CSedCondition *condition, const std::string &lhs,
                       const std::string &rhs) :
   CSedCommand(sed, condition), lhs_(lhs), rhs_(rhs) {
  }

  bool execute() override;

 private:
  std::string lhs_;
  std::string rhs_;
};

//---

class CSedToHoldCommand : public CSedCommand {
 public:
  CSedToHoldCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedToHoldNewlineCommand : public CSedCommand {
 public:
  CSedToHoldNewlineCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedToPatternCommand : public CSedCommand {
 public:
  CSedToPatternCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedToPatternNewlineCommand : public CSedCommand {
 public:
  CSedToPatternNewlineCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedExchangeCommand : public CSedCommand {
 public:
  CSedExchangeCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedListCommand : public CSedCommand {
 public:
  CSedListCommand(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  bool execute() override;
};

//---

class CSedAppendCommand : public CSedCommand {
 public:
  CSedAppendCommand(CSed *sed, CSedCondition *condition, const std::string &line) :
   CSedCommand(sed, condition), line_(line) {
  }

  bool execute() override;

 private:
  std::string line_;
};

//---

class CSedInsertCommand : public CSedCommand {
 public:
  CSedInsertCommand(CSed *sed, CSedCondition *condition, const std::string &line) :
   CSedCommand(sed, condition), line_(line) {
  }

  bool execute() override;

 private:
  std::string line_;
};

//---

class CSedChangeCommand : public CSedCommand {
 public:
  CSedChangeCommand(CSed *sed, CSedCondition *condition, const std::string &line) :
   CSedCommand(sed, condition), line_(line) {
  }

  bool execute() override;

 private:
  std::string line_;
};

//---

class CSedCommandList : public CSedCommand {
 public:
  CSedCommandList(CSed *sed, CSedCondition *condition) :
   CSedCommand(sed, condition) {
  }

  void addCommand(CSedCommand *command) {
    commands_.push_back(command);
  }

  bool execute() override;

 private:
  typedef std::vector<CSedCommand *> CommandList;

  CommandList commands_;
};

//---

class CSed {
 public:
  CSed();
 ~CSed() { }

  bool addCommandFile(CFile &file);

  bool addCommand(const std::string &cmd);

  bool process(CFile &file);

  bool nextLine();

  bool executeCommands();

  const std::string &getPattern() const { return pattern_; }

  void setPattern(const std::string &pattern) { pattern_ = pattern; }

  const std::string &getHold() const { return hold_; }

  void setHold(const std::string &hold) { hold_ = hold; }

  uint getLineNum() const { return line_num_; }

  bool isLastLine() const { return last_line_; }

  bool getSilent() const { return silent_; }
  void setSilent(bool silent) { silent_ = silent; }

 private:
  typedef std::vector<CSedCommand *>     CommandList;
  typedef std::vector<CSedCommandList *> CommandListList;

  CommandList      commands_;
  std::string      pattern_;
  std::string      hold_;
  uint             line_num_     { 0 };
  bool             last_line_    { false };
  bool             silent_       { false };
  CFile*           sfile_        { nullptr };
  CFile*           file_         { nullptr };
  CSedCommandList* command_list_ { nullptr };
  CommandListList  command_list_list_;
};

#endif
