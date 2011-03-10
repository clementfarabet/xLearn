#ifndef _LIB_CROP_H_
#define _LIB_CROP_H_

//definitions constantes
#define BUFFERSIZE 1000

uint32_t read_size_on_disk(FILE *fd, uint64_t *rs, uint64_t *cs, uint64_t *d);
uint32_t crop_on_disk(FILE *fd, struct xvimage** image_decoupee, uint64_t cx, uint64_t cy, uint64_t cz, uint64_t width, uint64_t height, uint64_t dpth);
uint32_t crop_raw_on_disk(FILE *fd, struct xvimage* image_decoupee, uint64_t cx, uint64_t cy, uint64_t cz, uint64_t width, uint64_t height, uint64_t dpth, uint64_t rs, uint64_t cs, uint64_t d, uint64_t header_size, uint64_t size_voxel, char compteur);
uint32_t crop_raw_on_disk_to_disk(FILE *fd, FILE *image_decoupee, uint64_t cx, uint64_t cy, uint64_t cz, uint64_t width, uint64_t height, uint64_t dpth, uint64_t rs, uint64_t cs, uint64_t d, uint64_t header_size, uint64_t size_voxel, char compteur);
#endif
