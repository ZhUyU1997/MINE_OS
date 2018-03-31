#ifndef __NAND_H__
#define __NAND_H__
#define PAGE_DATA_SIZE  (2048)
#define PAGE_SPARE_SIZE (64)
#define PAGES_PER_BLOCK (64)
#define PAGE_SHIFT (11)
#define BLOCK_SHIFT (17)
#define PAGE_TO_BLOCK_SHIFT (6)
#define BLOCK_TO_PAGE_SHIFT (6)
#define PAGE_TO_BLOCK(page) ((page)>>PAGE_TO_BLOCK_SHIFT)
#define BLOCK_TO_PAGE(block) ((block)<<BLOCK_TO_PAGE_SHIFT)

int is_boot_from_nor_flash(void);
void copy_code_to_sdram(unsigned char *dest, unsigned char *src, unsigned int len);
void nand_init(void);
int nand_is_bad_block(unsigned int block_number);
void nand_read(unsigned char *buf,unsigned int addr , unsigned int len);
int nand_read_with_oob(unsigned int page,unsigned char *buf,unsigned int buf_len,unsigned char *oob,unsigned int oob_len);
int nand_read_page(unsigned int page,unsigned char *buf,unsigned int buf_len,unsigned char *spare,unsigned int spare_len);
int nand_write(unsigned char *buf,unsigned int addr , unsigned int len);
int nand_write_page(unsigned int page,unsigned char *buf,unsigned int buf_len,unsigned char *spare,unsigned int spare_len);
int nand_write_with_oob(unsigned int page,unsigned char *buf,unsigned int buf_len,unsigned char *oob,unsigned int oob_len);
int nand_mark_bad_block(unsigned int block_number);
int nand_erase_block(unsigned int block_number);
void nand_print(unsigned int page);
#endif
