#include "qcli.h"

#include <cstdio>
#include <sstream>

QCli* QCli::instance_ = nullptr;

QCli* QCli::Instance() {
  if (instance_) {
    return instance_;
  } else {
    return (instance_ = new QCli());
  }
}

QCli::Status QCli::Parse(const std::vector<QString>& args) {
  if (args.size() > 0) {
    QString name = args[0];
    if (commands_[args.front()].exec != nullptr) {
      Command_s command = commands_[args.front()];

      // tell everyone we are about to execute that command, then tell them we're done executing
      emit CommandExecuteBefore(args[0]);
      Status result = command.exec(args, command.data);
      emit CommandExecuteAfter(args[0]);

      switch (result) {
      default:
      case OK:
        break;
      case INCORRECT_ARGUMENT:
        emit CommandExecuteError(args[0]);
        output << "Incorrect parameters passed to function " << name << "." << endl;
        break;
      case INTERNAL_ERROR:
        emit CommandExecuteError(args[0]);
        output << "Internal error occured." << endl;
        break;
      }
      return result;
    } else {
      commands_.remove(args[0]);
      output << "Command " << name << " is unknown." << endl;
      commands_["help"].exec(args, nullptr);
    }
  }
  return INCORRECT_COMMAND;
}

void QCli::AddCommand(const QString& name, Command cmd, void* callback) {
  if (cmd != nullptr) {
    commands_[name] = { cmd, callback };
  }
}

int QCli::Run() {
  std::vector<QString> args;
  QString name;
  for (;;) {
    output << "> ";
    output.flush();
    QString lastarg = input.readLine();
    QStringList splitted = lastarg.split("&&");
    for (QString substr : splitted) {
      std::istringstream iss(substr.toStdString());
      std::string word;
      while (iss >> word) {
        args.push_back(QString(word.c_str()));
      }
      if (args[0] == "quit" || args[0] == "exit") {
        emit Exit();
        return OK;
      }
      QList<QString> tmp = storage_.keys();
      
      // output any info we need to output and execute the command
      output.flush();
      Status result = Parse(args);  // TODO: parse result

      for (QString key : tmp) {
        if (!storage_.keys().contains(key)) {
          output << "- " << name << ": key `" << key << "` was deleted from storage" << endl;
        }
      }
      for (QString key : storage_.keys()) {
        if (!tmp.contains(key)) {
          output << "+ " << name << ": key `" << key << "` was added to storage" << endl;
        }
      }
      args.clear();
    }
  }

  return OK;
}

std::shared_ptr<void> QCli::GetUnsafeStorage(const QString& key) {
  if (storage_.contains(key)) {
    return storage_[key];
  } else {
    return nullptr;
  }
}

void QCli::SetUnsafeStorage(const QString& key, std::shared_ptr<void> data) {
  storage_[key] = data;
}

void QCli::UnsetUnsafeStorage(const QString& key) {
  storage_.remove(key);
}

void QCli::ClearUnsafeStorage() {
  storage_.clear();
}

const QList<QString> QCli::SupportedCommands() const {
  return commands_.keys();
}

// commands

CLI_COMMAND(help) {
  Q_UNUSED(args);
  Q_UNUSED(data);

  QCli *i = QCli::Instance();

  i->output << "The following commands are currently supported: " << endl;
  for (QString command : i->SupportedCommands()) {
    i->output << "\t" << command << endl;
  }

  return QCli::OK;
}

CLI_COMMAND(cleanup) {
  Q_UNUSED(data);

  QCli *i = QCli::Instance();
  i->output << args[0] << ": Clearing unsafe storage... ";
  i->ClearUnsafeStorage();
  i->output << "Done." << endl;

  return QCli::OK;
}
#include <iostream>
QCli::QCli() {
  commands_["help"].exec = help;
  commands_["usage"].exec = help;
  commands_["cleanup"].exec = cleanup;
}

QCli::~QCli() {
  instance_->ClearUnsafeStorage();
  delete instance_;
}
