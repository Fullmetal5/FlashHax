#include <stdio.h>
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
  if (argc != 3){
    printf("Usage ./ropFormat input.bin output.rop\n");
    return 0;
  }
  
  FILE *inputFile = fopen(argv[1], "rb");
  if (!inputFile){
    printf("Couldn't open file %s\n", argv[1]);
    return 0;
  }
  
  FILE *outputFile = fopen(argv[2], "wb");
  if (!outputFile){
    printf("Couldn't open file %s\n", argv[2]);
    return 0;
  }
  
  uint32_t data = 0;
  char format[] = "WriteDWORD(0x%.8x);\n";
  char buf[100];
  fseek(inputFile, 0L, SEEK_END);
  int inputSize = ftell(inputFile);
  rewind(inputFile);
  
  while (ftell(inputFile) != inputSize){
    fread(&data, 4, 1, inputFile);
    
    //Convert back to big endian
    data = (data & 0x000000FF) << 24u | (data & 0x0000FF00) << 8u | (data & 0x00FF0000) >> 8u | (data & 0xFF000000) >> 24u;
    
    snprintf(buf, sizeof(buf), format, data);
    
    fwrite(buf, strlen(buf), 1, outputFile);
  }
  
  fclose(inputFile);
  fclose(outputFile);
}