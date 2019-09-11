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
glb_fread_ret_check(size_t ret, size_t num_expect_to_read, boolean check_num_expect)
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


int main(int argc, char **argv)
{
    char buffer[256];
    uint32_t i = 0;
    bin_file_header *bfh;
    uint32_t num_backends;
    
    backend_entry *backendp;
    bind_entry *bindp;
    table_entry *tablep;

    //uint32_t num_tables;
    uint32_t num_binds;
    char hash_key[16];
    char ip[INET_ADDRSTRLEN];
    size_t ret;
    
    if (argc != 2) {
        glb_ftctl_usage();
        return 1;
    }
    
    const char *src_binary = argv[1];

    /* Open the binary forwarding table file for reading */
    FILE *in = fopen(src_binary, "rb");
    if (in == NULL) {
        printf("Could not open forwarding table file for reading.");
        return 0;
    }
    
    /* Read magic word */
    ret = fread(buffer, 4, 1, in);
    if (!glb_fread_ret_check(ret, 1, TRUE)) {
        return 0;
    }
    
    /* Read file header */
    bfh = malloc(sizeof(bin_file_header));
    if (!bfh) {
        return 0;
    }   
    ret = fread(bfh, sizeof(bin_file_header), 1, in);
    if (!glb_fread_ret_check(ret, 1, TRUE)) {
        return 0;
    }

    
    /* Read the # of backends that this BFT file knows about */
    ret = fread(&num_backends, sizeof(uint32_t), 1, in);
    if (!glb_fread_ret_check(ret, 1, TRUE)) {
        return 0;
    }
    
    /* Deal with backends : read each backend & display same */
    backendp = (backend_entry*) malloc(bfh->max_num_backends * \
                                       sizeof(backend_entry));
    if (!backendp) {
        return 0;
    }   
    ret = fread(backendp, sizeof(backend_entry), bfh->max_num_backends, in);
    if (!glb_fread_ret_check(ret, bfh->max_num_backends, FALSE)) {
        return 0;
    }
    
    for (i = 0; i < num_backends; i++) {
        printf("\n i = %u, Family = %d, State = %d, Health = %d, IP = %s",
               i, backendp[i].inet_family, backendp[i].state,
               backendp[i].health,
               inet_ntop(AF_INET, (const void *)&backendp[i].ip, ip,
                         INET_ADDRSTRLEN));
    }


    
    /* Number of binds */
    ret = fread(&num_binds, sizeof(uint32_t), 1, in);
    if (!glb_fread_ret_check(ret, 1, TRUE)) {
        return 0;
    }

    
    /* Deal with binds: read & display each bind entry */
    bindp = (bind_entry *)malloc(bfh->max_num_binds * sizeof(bind_entry));
    if (!bindp) {
        return 0;
    }
    
    ret = fread(bindp, sizeof(bind_entry), bfh->max_num_binds, in);
    if (!glb_fread_ret_check(ret, bfh->max_num_binds, TRUE)) {
        return 0;
    }
#if 0
    for (i = 0; i < bfh->max_num_binds; i++) {

    }
#endif
    free(bindp);

    
    /* Hash key */
    ret = fread(hash_key, 16, 1, in);
    if (!glb_fread_ret_check(ret, 1, TRUE)) {
        return 0;
    }
    printf("\nHash-key is %s", hash_key);
    
    
    /* Deal with the rendezvous hash-table */
    tablep = (table_entry *)malloc(bfh->table_entries * sizeof(table_entry));
    if (!tablep) {
        return 0;
    }
    ret = fread(tablep, sizeof(table_entry), bfh->table_entries, in);
    if (!glb_fread_ret_check(ret, bfh->table_entries, FALSE)) {
        return 0;
    }
    
    for (i = 0; i < bfh->table_entries; i++) {
        int j = 0;
        int num_idxs;

        num_idxs = tablep->num_idxs;
        printf("\n");
        for (j = 0; j < num_idxs; j++) {
            printf(" \t\t  index= %x %s", tablep[i].idxs[j],
                   inet_ntop(AF_INET,
							 (const void *)&backendp[tablep[i].idxs[j]].ip, ip,
							 INET_ADDRSTRLEN));
		}
	}
	printf("\n");
	free(tablep);
	free(backendp);
	
	free(bfh);

}
