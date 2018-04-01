#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

int main(int argc, char *argv[]){
  
  if (argc != 6){
    printf("Usage: ./payloadBuilder payload.bin EGG_TAG loaderstub.bin loader.bin boot.elf\n");
    printf("payload.bin: The output file. Used by the flash exploit as the payload\n");
    printf("EGG_TAG: The 4 byte tag that will be used to find the payload in memory. Needs to match the tag in egghunter.s\n");
    printf("loaderstub.bin: Transition loader that setups memory for the real loader and copies it into place.\n");
    printf("loader.bin: Modified Team Twizzers loader made to load a payload from memory first, falls back to sd and gecko loader if in memory payload fails.\n");
    printf("boot.elf: The executable to be embedded in the payload. This is probably the only thing you want to change.\n");
    return 0;
  }
  
  FILE *outputFile = fopen(argv[1], "wb");
  if (!outputFile){
    printf("Couldn't open file %s\n", argv[1]);
    return 0;
  }
  
  if (strlen(argv[2]) != 4){
    printf("ERROR: EGG_TAG was not 4 bytes long.\n");
    return 0;
  }
  
  // Write EGG_TAG twice so the egghunter can find it.
  fwrite(argv[2], 4, 1, outputFile);
  fwrite(argv[2], 4, 1, outputFile);
  
  FILE *loaderstubFile = fopen(argv[3], "rb");
  if (!loaderstubFile){
    printf("Couldn't open file %s\n", argv[3]);
    return 0;
  }
  fseek(loaderstubFile, 0L, SEEK_END);
  int loaderstubSize = ftell(loaderstubFile);
  rewind(loaderstubFile);
  
  FILE *loaderFile = fopen(argv[4], "rb");
  if (!loaderFile){
    printf("Couldn't open file %s\n", argv[4]);
    return 0;
  }
  fseek(loaderFile, 0L, SEEK_END);
  int loaderSize = ftell(loaderFile);
  rewind(loaderFile);
  
  FILE *bootFile = fopen(argv[5], "rb");
  if (!bootFile){
    printf("Couldn't open file %s\n", argv[5]);
    return 0;
  }
  fseek(bootFile, 0L, SEEK_END);
  int bootSize = ftell(bootFile);
  rewind(bootFile);
  
  // This is the offset in the payload file to the boot.elf embedded inside.
  uint32_t offset = loaderstubSize + loaderSize + 8; // Plus 8 because the tag is 4 bytes and is in there twice
  uint32_t totalSize = (loaderstubSize + loaderSize + bootSize + 4) / 4; // Total size of payload that needs to be memcpy'd into place plus 4 divided by 4. (Lazy hack so that last dword isn't lost if not divisible by 4)

  //Convert back to big endian
  uint32_t offsetB = (offset & 0x000000FF) << 24u | (offset & 0x0000FF00) << 8u | (offset & 0x00FF0000) >> 8u | (offset & 0xFF000000) >> 24u;
  uint32_t bootSizeB = (bootSize & 0x000000FF) << 24u | (bootSize & 0x0000FF00) << 8u | (bootSize & 0x00FF0000) >> 8u | (bootSize & 0xFF000000) >> 24u;
  uint32_t totalSizeB = (totalSize & 0x000000FF) << 24u | (totalSize & 0x0000FF00) << 8u | (totalSize & 0x00FF0000) >> 8u | (totalSize & 0xFF000000) >> 24u;
  
  fwrite(&totalSizeB, 4, 1, outputFile);
	
  fwrite(&offsetB, 4, 1, outputFile);
  fwrite(&bootSizeB, 4, 1, outputFile);
  
  void* loaderstubMem = malloc(loaderstubSize);
  fread(loaderstubMem, loaderstubSize, 1, loaderstubFile);
  fwrite(loaderstubMem, loaderstubSize, 1, outputFile);
  free(loaderstubMem);
  
  void* loaderMem = malloc(loaderSize);
  fread(loaderMem, loaderSize, 1, loaderFile);
  fwrite(loaderMem, loaderSize, 1, outputFile);
  free(loaderMem);
  
  void* bootMem = malloc(bootSize);
  fread(bootMem, bootSize, 1, bootFile);
  fwrite(bootMem, bootSize, 1, outputFile);
  free(bootMem);
  
  fclose(outputFile);
  fclose(loaderstubFile);
  fclose(loaderFile);
  fclose(bootFile);
  
  return 0;
}