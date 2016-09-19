#include <CSed.h>
#include <CFile.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

void
usage(const char *prog)
{
  std::cerr << "Usage: " << prog << "-e <script> -f <fileName> <fileName> ..." << std::endl;
}

int
main(int argc, char **argv)
{
  if (argc < 2) {
    usage(argv[0]);
    exit(1);
  }

  std::string              script;
  std::vector<std::string> sfilenames;
  std::vector<std::string> pfilenames;
  bool                     silent = false;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if      (argv[i][1] == 'e' || strcmp(argv[1], "--expression") == 0) {
        ++i;

        if (i < argc)
          script = argv[i];
      }
      else if (argv[i][1] == 'f' || strcmp(argv[1], "--file") == 0) {
        ++i;

        if (i < argc)
          sfilenames.push_back(argv[i]);
      }
      else if (argv[i][1] == 'n' || strcmp(argv[1], "--quiet") == 0 ||
               strcmp(argv[1], "--silent") == 0)
        silent = true;
      else
        std::cerr << "Invalid option " << argv[i] << std::endl;
    }
    else {
      if (script == "" && sfilenames.empty())
        script = argv[i];
      else
        pfilenames.push_back(argv[i]);
    }
  }

  //----

  CSed sed;

  if (silent)
    sed.setSilent(true);

  if (script != "")
    sed.addCommand(script);

  uint num_sfilenames = sfilenames.size();

  for (uint i = 0; i < num_sfilenames; ++i) {
    CFile file(sfilenames[i]);

    sed.addCommandFile(file);
  }

  uint num_pfilenames = pfilenames.size();

  if (num_pfilenames > 0) {
    for (uint i = 0; i < num_pfilenames; ++i) {
      CFile file(pfilenames[i]);

      sed.process(file);
    }
  }
  else {
    CFile file(stdin);

    sed.process(file);
  }

  return 0;
}
