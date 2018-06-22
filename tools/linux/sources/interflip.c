/* interflip: hastily written program that flips the interwork bit in ARM ELF object files. * Use it to hush interwork mismatch warnings by setting the interwork bit on binary files * objcopied to object files. * * by Jason Wilkins (fenix AT io DOT com) * * Contains some GPLed code (the ELF header struct from BFD), but the rest is (C) 2001 by me. * distribute under the terms of the GPL (www.gnu.org)  */#include <stdio.h>#include <unistd.h>#include <stdlib.h>#include <stdio.h>#include <string.h>#include <errno.h>#include <limits.h>#include <ctype.h>/* ELF Header (32-bit implementations) */typedef struct {  unsigned char e_ident[16];            /* ELF "magic number" */  unsigned char e_type[2];              /* Identifies object file type */  unsigned char e_machine[2];           /* Specifies required architecture */  unsigned char e_version[4];           /* Identifies object file version */  unsigned char e_entry[4];             /* Entry point virtual address */  unsigned char e_phoff[4];             /* Program header table file offset */  unsigned char e_shoff[4];             /* Section header table file offset */  unsigned char e_flags[4];             /* Processor-specific flags */  unsigned char e_ehsize[2];            /* ELF header size in bytes */  unsigned char e_phentsize[2];         /* Program header table entry size */  unsigned char e_phnum[2];             /* Program header table entry count */  unsigned char e_shentsize[2];         /* Section header table entry size */  unsigned char e_shnum[2];             /* Section header table entry count */  unsigned char e_shstrndx[2];          /* Section header string table index */} Elf32_External_Ehdr;#define EF_INTERWORK 0x04#define EM_ARM 40static char scratch[PATH_MAX * 2];void usage(char* argv0){   fprintf(stderr, "Usage: %s {-mthumb-interwork|-mno-thumb-interwork} file(s)\n", argv0);}int flip_file(char* path, int interwork){   Elf32_External_Ehdr header;   int low_byte;   FILE* f;   errno = 0;   f = fopen(path, "r+b");   if (f) {      errno = 0;      if (fread(&header, sizeof(Elf32_External_Ehdr), 1, f) == 1) {         if (header.e_ident[0] == 0x7f &&             header.e_ident[1] == 'E'  &&             header.e_ident[2] == 'L'  &&             header.e_ident[3] == 'F'  &&             header.e_ident[4] == 1) {            if (header.e_ident[5] == 1) {               low_byte = 0; }            else if (header.e_ident[5] == 2) {               low_byte = 2; }            else {               fprintf(stderr, "File corrupted? The byte order identifier in '%s' is invalid.\n", path);               goto cleanup; }            if (header.e_machine[low_byte] == EM_ARM) {               if (interwork) {                  header.e_flags[low_byte] |= EF_INTERWORK; }               else {                  header.e_flags[low_byte] &= ~EF_INTERWORK; }               errno = 0;               if (fseek(f, 0, SEEK_SET) == 0) {                  errno = 0;                  if (fwrite(&header, sizeof(Elf32_External_Ehdr), 1, f) == 1) {                     errno = 0;                     if (fclose(f) == 0) { }                     else {                        sprintf("Failed to close '%s'", path);                        perror(scratch);                        goto cleanup; } }                  else {                     sprintf(scratch, "Failed to read '%s'", path);                     perror(scratch);                     goto cleanup; } }               else {                  sprintf(scratch, "Failed to fseek on '%s'", path);                  perror(scratch);                  goto cleanup; } }            else {               fprintf(stderr, "The machine type for '%s' is not ARM.\n", path);               goto cleanup; } }         else {            fprintf(stderr, "'%s' is not an elf32 file.\n", path);            goto cleanup; } }      else {         sprintf(scratch, "Failed to read '%s'", path);         perror(scratch);         goto cleanup; } }   else {      sprintf(scratch, "Failed to open '%s'", path);      perror(scratch);      goto cleanup; }   return EXIT_SUCCESS;   cleanup: {      if (f) {         fclose(f); }      return EXIT_FAILURE; }}int main(int argc, char** argv){   if (argc >= 3) {      int interwork;      if (strcmp(argv[1], "-mthumb-interwork") == 0) {         interwork = 1; }      else if (strcmp(argv[1], "-mno-thumb-interwork") == 0) {         interwork = 0; }      else {         usage(argv[0]);         return EXIT_FAILURE; }      argv += 2;      while (*argv) {         flip_file(*argv, interwork);         argv++; } }   else {      usage(argv[0]);      return EXIT_FAILURE; }      return EXIT_SUCCESS;}