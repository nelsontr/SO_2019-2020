// In main.c

  // In processInput
    case 'r';

  // In applyCommands
    case 'r':
      renameFile(name,hashcode,fs,hashMax);
      break;

// In fs.c
  #include "lib/hash.h"

  void renameFile(char* names,int hashCode,tecnicofs *fs,int hashMax) {
    char *oldName,*newName;
    oldName = malloc(sizeof(char));
    newName = malloc(sizeof(char));
    int iNumber,newHash;
    sscanf(names,"%s %s",oldName,newName);
    iNumber = lookup(fs,oldName,hashCode);
    delete(fs,oldName,hashCode);
    newHash = hash(newName,hashMax);
    create(fs,newName,iNumber,newHash);
    free(oldName);
    free(newName);
}

// In fs.h

  void renameFile(char* names,int hashCode,tecnicofs *fs,int hashMax);
