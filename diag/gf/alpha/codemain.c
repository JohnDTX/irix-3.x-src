/* codemain.c  -- just write microcode onto Alpha FBC board
 */

#include "relocate.h"

main()
{
  RELOCATE;
  codewrite();
}
