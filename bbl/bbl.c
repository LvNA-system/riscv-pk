#include "bbl.h"
#include "mtrap.h"
#include "atomic.h"
#include "vm.h"
#include "bits.h"
#include "config.h"
#include "fdt.h"
#include <string.h>

static const void* entry_point;

static uintptr_t dtb_output()
{
  extern char _payload_end;
  uintptr_t end = (uintptr_t) &_payload_end;
  return (end + MEGAPAGE_SIZE - 1) / MEGAPAGE_SIZE * MEGAPAGE_SIZE;
}

static void filter_dtb(uintptr_t source)
{
  uintptr_t dest = dtb_output();
  uint32_t size = fdt_size(source);
  memcpy((void*)dest, (void*)source, size);
}

void boot_other_hart(uintptr_t dtb)
{
  const void* entry;
  do {
    entry = entry_point;
    mb();
  } while (!entry);
  enter_supervisor_mode(entry, read_csr(mhartid), dtb_output());
}

void boot_loader(uintptr_t dtb)
{
  extern char _payload_start;
  filter_dtb(dtb);
#ifdef PK_ENABLE_LOGO
  print_logo();
#endif
  mb();
  entry_point = &_payload_start;
  boot_other_hart(dtb);
}
