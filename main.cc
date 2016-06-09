#include "qcli.h"

CLI_COMMAND(test) {
  Q_UNUSED(args); 
  Q_UNUSED(data);
  
  QCli::Instance()->output << "success!" << endl;
  return QCli::OK;
}

int main(int argc, char *argv[]) {
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  QCli::Instance()->AddCommand("test", test);
  QCli::Instance()->Run();
}
