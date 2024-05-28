// Amazon FPGA Hardware Development Kit
//
// Copyright 2016 Amazon.com, Inc. or its affiliates. All Rights Reserved.
//
// Licensed under the Amazon Software License (the "License"). You may not use
// this file except in compliance with the License. A copy of the License is
// located at
//
//    http://aws.amazon.com/asl/
//
// or in the "license" file accompanying this file. This file is distributed on
// an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, express or
// implied. See the License for the specific language governing permissions and
// limitations under the License.
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pty.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

#include <utils/sh_dpi_tasks.h>
#include <fpga_pci.h>
#include <fpga_mgmt.h>
#include <utils/lcd.h>

#include "uart.h"

/*
 * pci_vendor_id and pci_device_id values below are Amazon's and avaliable to use for a given FPGA slot. 
 * Users may replace these with their own if allocated to them by PCI SIG
 */
static uint16_t pci_vendor_id = 0x1D0F; /* Amazon PCI Vendor ID */
static uint16_t pci_device_id = 0xF001; /* PCI Device ID preassigned by Amazon for F1 applications */



/*
 * check if the corresponding AFI for hello_world is loaded
 */
int check_afi_ready(int slot_id);

void usage(char* program_name) {
    fprintf(stderr, "usage: %s [--slot <slot-id>][<poke-value>]\n", program_name);
}

static pthread_t thread1, thread2;
int pty_fd;

int start_transmission(int slot_id, int pf_id, int bar_id);
void fail_thread(int rc, const char* err_msg);
void* inbound_handler(void* pci_bar_handle_ptr);
void* outbound_handler(void* pci_bar_handle_ptr);

static void term_handler(int sig) {
    pthread_cancel(thread1);     
    pthread_cancel(thread2);     
    close(pty_fd);
}


int main(int argc, char **argv) {
    int slot_id = 0;
    int rc;

    /* initialize the fpga_pci library so we could have access to FPGA PCIe from this applications */
    rc = fpga_pci_init();
    fail_on(rc, out, "Unable to initialize the fpga_pci library");

    /* initialize the fpga_plat library */
    rc = fpga_mgmt_init();
    fail_on(rc, out, "Unable to initialize the fpga_mgmt library");

    rc = check_afi_ready(slot_id);
    fail_on(rc, out, "AFI not ready");
    
    rc = start_transmission(slot_id, FPGA_APP_PF, APP_PF_BAR1);
    fail_on(rc, out, "transmission couldn't start");

    return rc;
out:
    return 1;
}

 int check_afi_ready(int slot_id) {
    struct fpga_mgmt_image_info info = {0}; 
    int rc;

    /* get local image description, contains status, vendor id, and device id. */
    rc = fpga_mgmt_describe_local_image(slot_id, &info,0);
    fail_on(rc, out, "Unable to get AFI information from slot %d. Are you running as root?",slot_id);

    /* check to see if the slot is ready */
    if (info.status != FPGA_STATUS_LOADED) {
        rc = 1;
        fail_on(rc, out, "AFI in Slot %d is not in READY state !", slot_id);
    }

    /* confirm that the AFI that we expect is in fact loaded */
    if (info.spec.map[FPGA_APP_PF].vendor_id != pci_vendor_id ||
       info.spec.map[FPGA_APP_PF].device_id != pci_device_id) {
     fprintf(stderr, "AFI does not show expected PCI vendor id and device ID. If the AFI "
            "was just loaded, it might need a rescan. Rescanning now.\n");

     rc = fpga_pci_rescan_slot_app_pfs(slot_id);
     fail_on(rc, out, "Unable to update PF for slot %d",slot_id);
     /* get local image description, contains status, vendor id, and device id. */
     rc = fpga_mgmt_describe_local_image(slot_id, &info,0);
     fail_on(rc, out, "Unable to get AFI information from slot %d",slot_id);


     /* confirm that the AFI that we expect is in fact loaded after rescan */
     if (info.spec.map[FPGA_APP_PF].vendor_id != pci_vendor_id ||
         info.spec.map[FPGA_APP_PF].device_id != pci_device_id) {
       rc = 1;
       fail_on(rc, out, "The PCI vendor id and device of the loaded AFI are not "
               "the expected values.");
     }
    }

    return rc;
 out:
    return 1;
 }

int init_uart(pci_bar_handle_t pci_bar_handle) {
    /* init uart regs */
    int rc;

    rc = fpga_pci_poke(pci_bar_handle, IER_ADDR, UINT32_C(0));
    fail_on(rc, out, "Unable to write to the fpga !");

    rc = fpga_pci_poke(pci_bar_handle, FCR_ADDR, UINT32_C(0));
    fail_on(rc, out, "Unable to write to the fpga !");
    
    rc = fpga_pci_poke(pci_bar_handle, FCR_ADDR, FCR_XMIT_RESET|FCR_RCVR_RESET);
    fail_on(rc, out, "Unable to write to the fpga !");

    rc = fpga_pci_poke(pci_bar_handle, FCR_ADDR, FCR_FIFO_ENABLE);
    fail_on(rc, out, "Unable to write to the fpga !");

    rc = fpga_pci_poke(pci_bar_handle, LCR_ADDR, LCR_DLAB | LCR_8N1);
    fail_on(rc, out, "Unable to write to the fpga !");

    rc = fpga_pci_poke(pci_bar_handle, DLL_ADDR, 66);
    fail_on(rc, out, "Unable to write to the fpga !");

    rc = fpga_pci_poke(pci_bar_handle, DLM_ADDR, 0);
    fail_on(rc, out, "Unable to write to the fpga !");
    
    rc = fpga_pci_poke(pci_bar_handle, LCR_ADDR, LCR_8N1);
    fail_on(rc, out, "Unable to write to the fpga !");

    /* if there is an error code, exit with status 1 */
out:
    return (rc != 0 ? 1 : 0);
}

int open_pty_pair (int *amaster, char** slave_name) {
    int master;
    char *name;

    master = getpt ();
    if (master < 0)
        return errno;

    if (grantpt (master) < 0 || unlockpt (master) < 0)
        goto close_master;
    
    name = ptsname (master);
    if (name == NULL)
        goto close_master;

    *amaster = master;
    *slave_name = name;
    return 0;

close_master:
    close (master);
    return errno;
}

/*
 * An example to attach to an arbitrary slot, pf, and bar with register access.
 */
int start_transmission(int slot_id, int pf_id, int bar_id) {
    int rc;
    pci_bar_handle_t pci_bar_handle = PCI_BAR_HANDLE_INIT;

    /* attach to the fpga, with a pci_bar_handle out param */
    rc = fpga_pci_attach(slot_id, pf_id, bar_id, 0, &pci_bar_handle);
    fail_on(rc, out, "Unable to attach to the AFI on slot id %d", slot_id);

    rc = init_uart(pci_bar_handle);
    fail_on(rc, out, "Unable to init uart regs");
    
    signal(SIGTERM, term_handler);

    char* slave_name = NULL;
    rc = open_pty_pair(&pty_fd, &slave_name);
    fail_on(rc, out, "Unable to get pty pair !");
    fprintf(stdout, "terminal is open at %s\n", slave_name);
    
    pthread_create( &thread1, NULL, &inbound_handler,  (void*) &pci_bar_handle);
    pthread_create( &thread2, NULL, &outbound_handler, (void*) &pci_bar_handle);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

out:
    /* clean up */
    if (pci_bar_handle >= 0) {
        rc = fpga_pci_detach(pci_bar_handle);
        if (rc) {
            fprintf(stderr, "Failure while detaching from the fpga.\n");
        }
    }

    /* if there is an error code, exit with status 1 */
    return (rc != 0 ? 1 : 0);
}

void* inbound_handler(void* pci_bar_handle_ptr)  {
    int rc;
    pci_bar_handle_t pci_bar_handle = * ((pci_bar_handle_t*) pci_bar_handle_ptr);
    while (1) {
        /* Read a value */
        uint32_t value;
        uint32_t drdy = 0;
        do {
            uint32_t tmp = 0;
            rc = fpga_pci_peek(pci_bar_handle, LSR_ADDR, &tmp);
            fail_thread(rc, "Unable to read read from the fpga !");
            drdy = tmp & LSR_DRDY;
        } while (!drdy);

        rc = fpga_pci_peek(pci_bar_handle, RBR_ADDR, &value);
        fail_thread(rc, "Unable to read read from the fpga !");
	if (!write(pty_fd, &value, 1)) {
            rc = 1;
            fail_thread(rc, "Unable to write to stream!");
        }
    }
    return NULL;
}

void* outbound_handler(void* pci_bar_handle_ptr) {
    int rc;
    pci_bar_handle_t pci_bar_handle = * ((pci_bar_handle_t*) pci_bar_handle_ptr);
    while (1) {
        char c;
	if (!read(pty_fd, &c, 1)) {
            rc = 1;
            fail_thread(rc, "Unable to write to stream!");
        }
        /* Send a value */
        uint32_t temt = 0;
        do {
            uint32_t tmp = 0;
            rc = fpga_pci_peek(pci_bar_handle, LSR_ADDR, &tmp);
            fail_thread(rc, "Unable to read read from the fpga !");
            temt = tmp & LSR_TEMT;
        } while (!temt);

        rc = fpga_pci_poke(pci_bar_handle, THR_ADDR, (uint32_t) c);
        fail_thread(rc, "Unable to write to the fpga !");
    }
    return NULL;
}

void fail_thread(int rc, const char* err_msg) {
    if (rc != 0) {
        fprintf(stderr, "%s, error %d\n", err_msg, rc);
        pthread_exit(NULL);
    }
}
