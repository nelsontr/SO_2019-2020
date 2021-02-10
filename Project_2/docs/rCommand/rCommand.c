void renameFile(char* oldName,char* newName,tecnicofs *fs) {
	int hashNew = hash(newName,fs->hashMax);
	int hashOld = hash(oldName, fs->hashMax);
	while (!lookup(fs,newName)) {
		int iNumber = lookup(fs,oldName);
		if (lookup(fs,oldName)) {
			if (!lookup(fs,newName)) {
				if (hashNew == hashOld) {
					if (!syncMech_try_lock(&(fs->bstLock[hashNew]))) {
						delete(fs,oldName,1);
						create(fs,newName,iNumber,1);
						sync_unlock(&(fs->bstLock[hashNew]));
						return;
					} else {
						sync_unlock(&(fs->bstLock[hashNew]));
					}
				} else {
					if (!syncMech_try_lock(&(fs->bstLock[hashOld]))) {
						delete(fs,oldName,1);
						sync_unlock(&(fs->bstLock[hashOld]));
					}
					if (!syncMech_try_lock(&(fs->bstLock[hashNew]))) {
						create(fs,newName,iNumber,1);
						sync_unlock(&(fs->bstLock[hashNew]));
					}
				}
			}
		}
	}	
	return;
}