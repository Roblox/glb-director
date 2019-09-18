#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include "config_types.h"

/*
 * This file contains code for a utility to dump the contents of a binary
 * forwarding table (BFT) used by GLB-director.
 * 
 * In the future, the displayed items can be displayed in more fancy ways.
 * This is just a start.
 */


typedef enum {FALSE, TRUE} boolean;


/*
 *
 */
void glb_ftctl_usage()
{
    printf("Usage: glb-director-ftctl <ft.bin>\n");
}

/*
 * fread_ret_check()
 *
 * Function to check if the returned count by a call to fread() can be 
 * considered proper.
 *
 */

static int
glb_fread_ret_check(size_t ret, size_t num_expect_to_read,
                    boolean check_num_expect)
{
    /* Improper read */
    if (check_num_expect) {
        if (ret < num_expect_to_read) {
            return 0;
        }
    } else {
        if (!ret) {
            return 0;
        }
    }

    /* Proper read */
    return 1;
}


static int
glb_read_per_table_fields(FILE *in, bin_file_header *bfh)
{
    uint32_t i = 0;
    uint32_t num_backends;
    
    backend_entry *backendp;
    bind_entry *bindp;
    table_entry *tablep;

    uint32_t num_binds;
    char hash_key[16];
    char ip[INET_ADDRSTRLEN];
    size_t ret;

    /* Read the # of backends that this BFT file knows about */
    ret = fread(&num_backends, sizeof(uint32_t), 1, in);
    if (!glb_fread_ret_check(ret, 1, TRUE)) {
        return -1;;
    }
    
    /* Deal with backends : read each backend & display same */
    backendp = (backend_entry*) malloc(bfh->max_num_backends * \
                                       sizeof(backend_entry));
    if (!backendp) {
        return -1;;
    }   
    ret = fread(backendp, sizeof(backend_entry), bfh->max_num_backends, in);
    if (!glb_fread_ret_check(ret, bfh->max_num_backends, FALSE)) {
        return -1;;
    }

    printf("\nBackends\n");
    for (i = 0; i < num_backends; i++) {
      printf("\n i = %u, \tFamily = %d, \tState = %d, \tHealth = %d, "  \
             "\tIP = %s",
             i, backendp[i].inet_family, backendp[i].state,
             backendp[i].health,
             inet_ntop(AF_INET, (const void *)&backendp[i].ip, ip,
                       INET_ADDRSTRLEN));
    }


    
    /* Number of binds */
    ret = fread(&num_binds, sizeof(uint32_t), 1, in);
    if (!glb_fread_ret_check(ret, 1, TRUE)) {
        return -1;;
    }

    
    /* Deal with binds: read & display each bind entry */
    bindp = (bind_entry *)malloc(bfh->max_num_binds * sizeof(bind_entry));
    if (!bindp) {
        return -1;;
    }
    
    ret = fread(bindp, sizeof(bind_entry), bfh->max_num_binds, in);
    if (!glb_fread_ret_check(ret, bfh->max_num_binds, TRUE)) {
        return -1;;
    }
#if 0
    printf("\nBinds:\n");
    for (i = 0; i < bfh->max_num_binds; i++) {

    }
#endif
    free(bindp);

    
    /* Hash key */
    ret = fread(hash_key, 16, 1, in);
    if (!glb_fread_ret_check(ret, 1, TRUE)) {
        return -1;
    }

    printf("\n\nHash-key: 0x");
    for (i = 0; i < 16; i++) {
        printf("%x", (unsigned char)hash_key[i]);
    }
    printf("\n");
    
    /* Deal with the rendezvous hash-table */
    tablep = (table_entry *)malloc(sizeof(table_entry));
    if (!tablep) {
        return -1;;
    }
    
    printf("\n\nForwarding table:");
    for (i = 0; i < bfh->table_entries; i++) {
        int j = 0;
        uint32_t num_idxs;

        /* Read the # of idxs in this row */
        ret = fread(&num_idxs, sizeof(num_idxs), 1, in);
        if (!glb_fread_ret_check(ret, 1, TRUE)) {
          return -1;;
        }

        ret = fread(tablep->idxs, MAX_NUM_BACKEND_IDXS*sizeof(uint32_t), 1, in);
        if (!glb_fread_ret_check(ret, 1, TRUE)) {
          return -1;;
        }

        printf("\n\nEntry: 0x%x\n", i);
        for (j = 0; j < num_idxs; j++) {
            if ((j % 4 )) {
                printf("\t");
            } else {
                printf("\n");
            }
            printf("index= 0x%x %s", tablep->idxs[j],
                   inet_ntop(AF_INET,
                             (const void *)&backendp[tablep->idxs[j]].ip, ip,
                             INET_ADDRSTRLEN));
        }
    }
    printf("\n");
    free(tablep);
    free(backendp);
    return 0;
}

int main(int argc, char **argv)
{
    char buffer[256];
    uint32_t i = 0;
    bin_file_header *bfh;
    size_t ret;
    
    if (argc != 2) {
        glb_ftctl_usage();
        return -1;
    }
    
    const char *src_binary = argv[1];

    /* Open the binary forwarding table file for reading */
    FILE *in = fopen(src_binary, "rb");
    if (in == NULL) {
        printf("Could not open forwarding table file for reading.");
        return -1;
    }
    
    /* Read magic word */
    ret = fread(buffer, 4, 1, in);
    if (!glb_fread_ret_check(ret, 1, TRUE)) {
        return -1;
    }
    
    /* Read file header */
    bfh = malloc(sizeof(bin_file_header));
    if (!bfh) {
        return -1;
    }   
    ret = fread(bfh, sizeof(bin_file_header), 1, in);
    if (!glb_fread_ret_check(ret, 1, TRUE)) {
        return -1;
    }

    /* For each table, read the fields */
    for (i = 0; i < bfh->num_tables; i++) {
        ret = glb_read_per_table_fields(in, bfh);
    }
    
    
    free(bfh);
    return(ret);

}
