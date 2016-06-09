#pragma once

#include <memory>
#include <vector>
#include <QMap>
#include <QList>
#include <QTextStream>
#include <QThread>
#include <QString>
#include <QObject>

#define CLI_COMMAND(cmd) QCli::Status cmd(const std::vector<QString>& args, void* data)

class QCli : public QObject {
  Q_OBJECT

 public:
  enum Status {
    OK,
    INCORRECT_COMMAND,
    INCORRECT_ARGUMENT,
    INTERNAL_ERROR,
  };

  typedef Status (*Command)(const std::vector<QString>& args, void* data);

  struct Command_s {
    Command exec;
    void* data;
  };

 private:
  QCli();
  QCli(const QCli&) {};
  ~QCli();
  QCli& operator=(QCli&) {};

  static QCli* instance_;
  QMap<QString, Command_s> commands_;

  QMap<QString, std::shared_ptr<void>> storage_;

 signals:
  void CommandExecuteBefore(QString command_name);
  void CommandExecuteAfter(QString command_name);
  void CommandExecuteError(QString command_name);
  void Exit();

 public:
  static QCli* Instance();

  // Parse entered command. Args should contain command name and (optional) its arguments
  Status Parse(const std::vector<QString>& args);

  // Add a command to CLI. name is the literal name by which the command will be called, cmd is function pointer to command handler
  // and callback is an optional pointer to whatever you might need to be passed to that function (like external function, object, variable, whatever).
  // It is caller's job to make sure that callback is used responsibly and it won't break anything. A good practice is to pass pointer to struct
  void AddCommand(const QString& name, Command cmd, void* data = nullptr);

  // Run the CLI command line. Blocks execution until "quit" or "exit" is encountered
  int Run();

  // Get unsafe storage pointer
  std::shared_ptr<void> GetUnsafeStorage(const QString& key);

  // Set unsafe storage to specified pointer
  void  SetUnsafeStorage(const QString& key, std::shared_ptr<void> data);

  // Remove unsafe storage entry
  void  UnsetUnsafeStorage(const QString& key);

  // Clear unsafe storage
  void ClearUnsafeStorage();

  const QList<QString> SupportedCommands() const;

  QTextStream output;
};
