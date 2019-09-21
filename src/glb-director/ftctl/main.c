#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../glb-includes/glb_common_includes.h"
#include "../glb_consts.h"
#include "glb_config_types.h"

/*
 * This file contains code for a utility to dump the contents of a binary
 * forwarding table (BFT) used by GLB-director.
 * 
 * In the future, the displayed items can be displayed in more fancy ways.
 * This is just a start.
 */

/*
 * glb_ftctl_usage()
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

/*
 * glb_ip_addr_to_str()
 * 
 * Converts an IP address (v4 or v6) to a corresponding string representation,
 * based on the inet_family provided.
 *
 * Caller's responsibility to ensure that dst can hold an appropriate length of
 * characters.
 */

static boolean
glb_ip_addr_to_str(uint32_t glb_inet_family, const void *src, char *dst)
{
    socklen_t size = 0;
    uint32_t inet_family = inet_family;
    
    if (glb_inet_family == GLB_FAMILY_IPV4) {
        inet_family = AF_INET;
        size = INET_ADDRSTRLEN;
    } else if (glb_inet_family == GLB_FAMILY_IPV6) {
        inet_family = AF_INET6;
        size = INET6_ADDRSTRLEN;
    } else {
        return FALSE;
    }
    
    inet_ntop(inet_family, src, dst, size);
    return TRUE;
}


/*
 * glb_read_per_table_fields()
 *
 * Prints ther per-table fields.
 * Assumption is that the read-offet for "in" is at the appropriate offset for
 * a table that is sought to be read.
 *
 */
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
        return -1;
    }
    
    /* Deal with backends : read each backend & display same */
    backendp = (backend_entry *) malloc(bfh->max_num_backends * \
                                       sizeof(backend_entry));
    if (!backendp) {
        return -1;
    }   
    ret = fread(backendp, sizeof(backend_entry), bfh->max_num_backends, in);
    if (!glb_fread_ret_check(ret, bfh->max_num_backends, FALSE)) {
        return -1;
    }

    printf("\n\nBackends");
    for (i = 0; i < num_backends; i++) {
        if (glb_ip_addr_to_str(backendp[i].inet_family, &backendp[i].ip, ip)) {
            printf("\n i = %u, \tFamily = %d, \tState = %s, \tHealth = %s, " \
                   "\tIP = %s",
                   i, backendp[i].inet_family,
                   glb_state_names[backendp[i].state],
                   glb_backend_health_status[backendp[i].health], ip);
        }
    }


    
    /* Number of binds */
    ret = fread(&num_binds, sizeof(uint32_t), 1, in);
    if (!glb_fread_ret_check(ret, 1, TRUE)) {
        return -1;
    }

    /* Deal with binds: read & display each bind entry */
    bindp = (bind_entry *)malloc(bfh->max_num_binds * sizeof(bind_entry));
    if (!bindp) {
        return -1;
    }
    
    ret = fread(bindp, sizeof(bind_entry), bfh->max_num_binds, in);
    if (!glb_fread_ret_check(ret, bfh->max_num_binds, TRUE)) {
        return -1;
    }

    printf("\n\nBinds:");
    for (i = 0; i < num_binds; i++) {
      if (glb_ip_addr_to_str(bindp[i].inet_family, &bindp[i].ip, ip)) {
            printf("\nIP: %40s, \t\tIP-proto: %u, Port: %u-%u",
                   ip,
                   bindp[i].ipproto,
                   bindp[i].port_start,
                   bindp[i].port_end);
        }
    }

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
        return -1;
    }
    
    printf("\n\nForwarding table:");
    for (i = 0; i < bfh->table_entries; i++) {
        int j = 0;
        uint32_t num_idxs;

        /* Read the # of idxs in this row */
        ret = fread(&num_idxs, sizeof(num_idxs), 1, in);
        if (!glb_fread_ret_check(ret, 1, TRUE)) {
          return -1;
        }

        ret = fread(tablep->idxs, GLB_MAX_GUE_HOPS * sizeof(uint32_t), 1, in);
        if (!glb_fread_ret_check(ret, 1, TRUE)) {
          return -1;
        }

        printf("\n\nEntry: 0x%x", i);
        for (j = 0; j < num_idxs; j++) {
            if ((j % 4 )) {
                printf("\t");
            } else {
                printf("\n");
            }
            if (glb_ip_addr_to_str(backendp->inet_family,
                                   (const void *)&backendp[tablep->idxs[j]].ip,
                                   ip)) {
                printf("index= 0x%x %s", tablep->idxs[j], ip);
            }
        }
    }
    printf("\n");
    free(tablep);
    free(backendp);
    return 0;
}

/*
 * glb_print_bin_file_header()
 *
 * Print the contents of the file header
 */ 
static void
glb_print_bin_file_header(bin_file_header *bfh)
{
    printf("\nFile header");
    printf("\n\tVersion: %u", bfh->file_fmt_ver);
    printf("\n\tNumber of tables: %u", bfh->num_tables);
    printf("\n\tTable entries: %u", bfh->table_entries);
    printf("\n\tMax. # of backends: %u", bfh->max_num_backends);
    printf("\n\tMax. # of binds: %u\n", bfh->max_num_binds);
}

int
main(int argc, char **argv)
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

    /* Print the header */
    glb_print_bin_file_header(bfh);

    printf("\nNumber of tables: %u", bfh->num_tables);
    
    /* For each table, read the fields & display same */
    for (i = 0; i < bfh->num_tables; i++) {
        printf("\n\n*** Table #: %d ***", i+1);
        ret = glb_read_per_table_fields(in, bfh);

        printf("\n");
    }
    
    free(bfh);
    return(ret);
}
