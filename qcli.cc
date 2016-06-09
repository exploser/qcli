#include "qcli.h"

#include <iostream>
#include <sstream>

CLI* CLI::instance_ = nullptr;

CLI* CLI::Instance() {
  if (instance_) {
    return instance_;
  } else {
    return (instance_ = new CLI());
  }
}

CLI::Status CLI::Parse(const std::vector<QString>& args) {
  if (args.size() > 0) {
    std::string name = args[0].toStdString();
    if (commands_[args.front()].exec != nullptr) {
      Command_s command = commands_[args.front()];
      Status result = command.exec(args, command.data);
      switch (result) {
      default:
      case OK:
        break;
      case INCORRECT_ARGUMENT:
        std::cout << "Incorrect parameters passed to function " << name << "." << std::endl;
        break;
      case INTERNAL_ERROR:
        std::cout << "Internal error occured." << std::endl;
        break;
      }
      return result;
    } else {
      commands_.remove(args[0]);
      std::cout << "Command " << name << " is unknown." << std::endl;
      commands_["help"].exec(args, nullptr);
    }
  }
  return INCORRECT_COMMAND;
}

void CLI::AddCommand(const QString& name, Command cmd, void* callback) {
  if (cmd != nullptr) {
    commands_[name] = { cmd, callback };
  }
}

int CLI::Run() {
  std::vector<QString> args;
  char lastarg[1024];
  std::string name;
  for (;;) {
    std::cout << "> ";
    std::cin.getline(lastarg, sizeof(lastarg));
    QStringList splitted = QString(lastarg).split("&&");
    for (QString substr : splitted) {
      std::istringstream iss(substr.toStdString());
      std::string word;
      while (iss >> word) {
        args.push_back(QString(word.c_str()));
      }
      if (args[0] == "quit" || args[0] == "exit") {
        return OK;
      }
      QList<QString> tmp = storage_.keys();
      Status result = Parse(args);  // TODO: parse result
      for (QString key : tmp) {
        if (!storage_.keys().contains(key)) {
          std::cout << "- " << name << ": key `" << key.toStdString() << "` was deleted from storage" << std::endl;
        }
      }
      for (QString key : storage_.keys()) {
        if (!tmp.contains(key)) {
          std::cout << "+ " << name << ": key `" << key.toStdString() << "` was added to storage" << std::endl;
        }
      }
      args.clear();
    }
  }

  return OK;
}

std::shared_ptr<void> CLI::GetUnsafeStorage(const QString& key) {
  if (storage_.contains(key)) {
    return storage_[key];
  } else {
    return nullptr;
  }
}

void CLI::SetUnsafeStorage(const QString& key, std::shared_ptr<void> data) {
  storage_[key] = data;
}

void CLI::UnsetUnsafeStorage(const QString& key) {
  storage_.remove(key);
}

void CLI::ClearUnsafeStorage() {
  storage_.clear();
}

const QList<QString> CLI::SupportedCommands() const {
  return commands_.keys();
}

// commands

COMMAND(help) {
  Q_UNUSED(args);
  Q_UNUSED(data);

  CLI *i = CLI::Instance();

  std::cout << "The following commands are currently supported: " << std::endl;
  for (QString command : i->SupportedCommands()) {
    std::cout << "\t" << command.toStdString() << std::endl;
  }

  return CLI::OK;
}

COMMAND(cleanup) {
  Q_UNUSED(data);

  CLI *i = CLI::Instance();
  std::cout << args[0].toStdString() << ": Clearing unsafe storage... ";
  i->ClearUnsafeStorage();
  std::cout << "Done." << std::endl;

  return CLI::OK;
}

CLI::CLI() {
  commands_["help"].exec = help;
  commands_["usage"].exec = help;
  commands_["cleanup"].exec = cleanup;
}

CLI::~CLI() {
  delete instance_;
}
