/*******************************************************************************
* App : ZeroNet for Ledger Nano S
* Author: Me ;)
* Description : File with main logic (main loop + UI).
* License : DWTFYW
* TODO :
*   - Split file
*   - Docs + comment
*   - Learn C programmation
********************************************************************************/

#include "os.h"
#include "cx.h"

#include "os_io_seproxyhal.h"
#include "string.h"
unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

// UI currently displayed
enum UI_STATE { UI_IDLE, UI_CREATION, UI_SIGN };

enum UI_STATE uiState;

ux_state_t ux;

unsigned int io_seproxyhal_touch_exit(bagl_element_t *e);
unsigned int io_seproxyhal_touch_approve(bagl_element_t *e);
unsigned int io_seproxyhal_touch_deny(bagl_element_t *e);

void ui_idle(void);
void ui_creation(void);
//void ui_sign(void);

#define CLA 0x80
#define INS_SIGN 0x02 // Sign website update
#define INS_GET_PUBLIC_KEY 0x04 // Get pubKey for website creation
#define P1_LAST 0x80
#define P1_MORE 0x00

const cx_ecfp_private_key_t N_privateKey; // private key in flash. const and N_
                                          // variable name are mandatory here
const unsigned char N_initialized; // initialization marker in flash. const and
                                  // N_ variable name are mandatory here

//char lineBuffer[50];
//cx_sha256_t hash;


/*
 * Welcome first screen for Nano S
 */
const bagl_element_t ui_idle_nanos[] = {
    // type                               userid    x    y   w    h  str rad
    // fill      fg        bg      fid iid  txt   touchparams...       ]
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x01, 4, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px, 0},
     "Waiting for instructions",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x01, 4, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px, 0},
     "from ZeroNet plugin",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_ICON, 0x01, 118, 20, 7, 4, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_DOWN},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x02, 29, 9, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CROSS_BADGE},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x02, 50, 19, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px, 0},
     "Quit app",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_ICON, 0x02, 3, 14, 7, 4, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_UP},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
};
unsigned int ui_idle_nanos_button(unsigned int button_mask,
                                  unsigned int button_mask_counter);

unsigned int ui_idle_nanos_state;
unsigned int ui_idle_nanos_prepro(const bagl_element_t *element) {
    if (element->component.userid > 0) {
        return (ui_idle_nanos_state == element->component.userid - 1);
    }
    return 1;
}

unsigned int io_seproxyhal_touch_exit(bagl_element_t *e) {
    // go back to the home screen
    os_sched_exit(0);
    return 0; // DO NOT REDRAW THE BUTTON
}

unsigned int ui_idle_nanos_button(unsigned int button_mask,
                                  unsigned int button_mask_counter) {
    switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_LEFT: // UP
        if (ui_idle_nanos_state == 1) {
            ui_idle_nanos_state = 0;
            UX_DISPLAY(ui_idle_nanos, ui_idle_nanos_prepro);
        }
        break;

    case BUTTON_EVT_RELEASED | BUTTON_RIGHT: // DOWN
        if (ui_idle_nanos_state == 0) {
            ui_idle_nanos_state = 1;
            UX_DISPLAY(ui_idle_nanos, ui_idle_nanos_prepro);
        }
        break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // EXIT
        if (ui_idle_nanos_state == 1) {
            io_seproxyhal_touch_exit(NULL);
        }
        break;
    }
    return 0;
}

/*
 * Confirm ZeroNet website creation
 */
const bagl_element_t ui_confirm_creation_nanos[] = {
    // type                               userid    x    y   w    h  str rad
    // fill      fg        bg      fid iid  txt   touchparams...       ]
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_LABELINE, 0x02, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "Confirm creation",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CHECK},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
};

unsigned int
ui_confirm_creation_nanos_button(unsigned int button_mask,
                                 unsigned int button_mask_counter) {
    switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      // TODO : print confirmation of the pubkey creation for 3 secs then
      // back to idle
        io_seproxyhal_touch_approve(NULL);
        break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
        io_seproxyhal_touch_deny(NULL);
        break;
    }
    return 0;
}

unsigned int io_seproxyhal_touch_approve(bagl_element_t *e) {
    unsigned int tx = 0;

    cx_ecfp_public_key_t publicKey;
    cx_ecfp_private_key_t privateKey;
    os_memmove(&privateKey, &N_privateKey,
              sizeof(cx_ecfp_private_key_t));
    cx_ecfp_generate_pair(CX_CURVE_256K1, &publicKey,
                          &privateKey, 1);
    os_memmove(G_io_apdu_buffer, publicKey.W, 65);
    tx = 65;

    G_io_apdu_buffer[tx++] = 0x90;
    G_io_apdu_buffer[tx++] = 0x00;
    // Send back the response, do not restart the event loop
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
    // Display back the original UX
    ui_idle();
    return 0; // do not redraw the widget
}

unsigned int io_seproxyhal_touch_deny(bagl_element_t *e) {
    //hashTainted = 1;
    G_io_apdu_buffer[0] = 0x69;
    G_io_apdu_buffer[1] = 0x85;
    // Send back the response, do not restart the event loop
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
    // Display back the original UX
    ui_idle();
    return 0; // do not redraw the widget
}

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    switch (channel & ~(IO_FLAGS)) {
    case CHANNEL_KEYBOARD:
        break;

    // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
    case CHANNEL_SPI:
        if (tx_len) {
            io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

            if (channel & IO_RESET_AFTER_REPLIED) {
                reset();
            }
            return 0; // nothing received from the master so far (it's a tx
                      // transaction)
        } else {
            return io_seproxyhal_spi_recv(G_io_apdu_buffer,
                                          sizeof(G_io_apdu_buffer), 0);
        }

    default:
        THROW(INVALID_PARAMETER);
    }
    return 0;
}

void zeronet_main(void) {
    volatile unsigned int rx = 0;
    volatile unsigned int tx = 0;
    volatile unsigned int flags = 0;

    // DESIGN NOTE: the bootloader ignores the way APDU are fetched. The only
    // goal is to retrieve APDU.
    // When APDU are to be fetched from multiple IOs, like NFC+USB+BLE, make
    // sure the io_event is called with a
    // switch event, before the apdu is replied to the bootloader. This avoid
    // APDU injection faults.
    for (;;) {
        volatile unsigned short sw = 0;

        BEGIN_TRY {
            TRY {
                rx = tx;
                tx = 0; // ensure no race in catch_other if io_exchange throws
                        // an error
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                // no apdu received, well, reset the session, and reset the
                // bootloader configuration
                if (rx == 0) {
                    THROW(0x6982);
                }

                if (G_io_apdu_buffer[0] != CLA) {
                    THROW(0x6E00);
                }

                // unauthenticated instruction
                switch (G_io_apdu_buffer[1]) {
                case 0x00: // reset
                    flags |= IO_RESET_AFTER_REPLIED;
                    THROW(0x9000);
                    break;

                case 0x01: // case 1
                    THROW(0x9000);
                    break;

                case INS_SIGN: // echo
                    tx = rx;
                    THROW(0x9000);
                    break;

                case INS_GET_PUBLIC_KEY: {
                    /*cx_ecfp_public_key_t publicKey;
                    cx_ecfp_private_key_t privateKey;
                    os_memmove(&privateKey, &N_privateKey,
                              sizeof(cx_ecfp_private_key_t));
                    cx_ecfp_generate_pair(CX_CURVE_256K1, &publicKey,
                                          &privateKey, 1);
                    os_memmove(G_io_apdu_buffer, publicKey.W, 65);
                    tx = 65;
                    THROW(0x9000);*/
                    ui_creation(); // We ask the user to confirm that he want to
                                  // a new zeronet website (really necessary ?)
                    flags |= IO_ASYNCH_REPLY;

                } break;

                case 0xFF: // return to dashboard
                    goto return_to_dashboard;

                default:
                    THROW(0x6D00);
                    break;
                }
            }
            CATCH_OTHER(e) {
                switch (e & 0xF000) {
                case 0x6000:
                case 0x9000:
                    sw = e;
                    break;
                default:
                    sw = 0x6800 | (e & 0x7FF);
                    break;
                }
                // Unexpected exception => report
                G_io_apdu_buffer[tx] = sw >> 8;
                G_io_apdu_buffer[tx + 1] = sw;
                tx += 2;
            }
            FINALLY {
            }
        }
        END_TRY;
    }

return_to_dashboard:
    return;
}

void io_seproxyhal_display(const bagl_element_t *element) {
    io_seproxyhal_display_default((bagl_element_t *)element);
}

unsigned char io_event(unsigned char channel) {
    // nothing done with the event, throw an error on the transport layer if
    // needed

    // can't have more than one tag in the reply, not supported yet.
    switch (G_io_seproxyhal_spi_buffer[0]) {
    case SEPROXYHAL_TAG_FINGER_EVENT:
        UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: // for Nano S
        UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
        if (UX_DISPLAYED()) {
            // TODO perform actions after all screen elements have been
            // displayed
        } else {
            UX_DISPLAY_PROCESSED_EVENT();
        }
        break;

    // unknown events are acknowledged
    default:
        break;
    }

    // close the event if not done previously (by a display or whatever)
    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }

    // command has been processed, DO NOT reset the current APDU transport
    return 1;
}

__attribute__((section(".boot"))) int main(void) {
    // exit critical section
    __asm volatile("cpsie i");

    UX_INIT();

    // ensure exception will work as planned
    os_boot();

    BEGIN_TRY {
        TRY {
            io_seproxyhal_init();

            // Create the private key if not initialized
            if (N_initialized != 0x01) {
                unsigned char canary;
                cx_ecfp_private_key_t privateKey;
                cx_ecfp_public_key_t publicKey;
                cx_ecfp_generate_pair(CX_CURVE_256K1, &publicKey, &privateKey,
                                      0);
                nvm_write(&N_privateKey, &privateKey, sizeof(privateKey));
                canary = 0x01;
                nvm_write(&N_initialized, &canary, sizeof(canary));
            }

            USB_power(0);
            USB_power(1);

            ui_idle();

            zeronet_main();
        }
        CATCH_OTHER(e) {
        }
        FINALLY {
        }
    }
    END_TRY;
}

void ui_idle(void) {
    ui_idle_nanos_state = 0; // start by displaying the idle first screen
    UX_DISPLAY(ui_idle_nanos, ui_idle_nanos_prepro);
}

void ui_creation(void) {
    uiState = UI_CREATION;
    UX_DISPLAY(ui_confirm_creation_nanos, NULL);
}
