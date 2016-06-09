/* -*-  mode:c; tab-width:8; c-basic-offset:8; indent-tabs-mode:nil;  -*- */
/* 
   Copyright (C) 2013 Ronnie Sahlberg <ronniesahlberg@gmail.com>
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>

#include <CUnit/CUnit.h>

#include "iscsi.h"
#include "scsi-lowlevel.h"
#include "iscsi-test-cu.h"

void
test_writesame16_0blocks(void)
{
        CHECK_FOR_DATALOSS;
        CHECK_FOR_SBC;

        if (!inq_bl) {
                CU_PASS("BlockLimits VPD is not available. Skipping test.\n");
                return;
        }

        logging(LOG_VERBOSE, LOG_BLANK_LINE);
        logging(LOG_VERBOSE, "Test WRITESAME16 0-blocks at LBA==0 (WSNZ=%d)",
                inq_bl->wsnz);
        memset(scratch, 0, block_size);

        if (inq_bl->wsnz) {
                WRITESAME16(sd, 0, block_size, 0, 0, 0, 0, 0, scratch,
                            EXPECT_INVALID_FIELD_IN_CDB);
                logging(LOG_NORMAL, "[SKIPPED] WRITESAME16 does not support 0-blocks.");
                return;
        }


        if (inq_bl->max_ws_len > 0 && num_blocks >= inq_bl->max_ws_len) {
            WRITESAME16(sd, 0, block_size, 0, 0, 0, 0, 0, scratch,
                        EXPECT_INVALID_FIELD_IN_CDB);
        } else {
            WRITESAME16(sd, 0, block_size, 0, 0, 0, 0, 0, scratch,
                        EXPECT_STATUS_GOOD);
        }

        if (inq_bl->max_ws_len > 0) {
            logging(LOG_VERBOSE,
                    "Test WRITESAME16 at MAXIMUM WRITE SAME LENGTH + 1 blocks "
                    "from end-of-LUN");
            CHECK_SIZE((inq_bl->max_ws_len + 1),
                       WRITESAME16(sd, num_blocks - (inq_bl->max_ws_len + 1),
                                   block_size, 0, 0, 0, 0, 0, scratch,
                                   EXPECT_INVALID_FIELD_IN_CDB));
            logging(LOG_VERBOSE,
                    "Test WRITESAME16 at MAXIMUM WRITE SAME LENGTH blocks "
                    "from end-of-LUN");
            CHECK_SIZE(inq_bl->max_ws_len,
                       WRITESAME16(sd, num_blocks - inq_bl->max_ws_len,
                                   block_size, 0, 0, 0, 0, 0, scratch,
                                   EXPECT_STATUS_GOOD));
            logging(LOG_VERBOSE,
                    "Test WRITESAME16 at MAXIMUM WRITE SAME LENGTH - 1 blocks "
                    "from end-of-LUN");
            CHECK_SIZE((inq_bl->max_ws_len - 1),
                       WRITESAME16(sd, num_blocks - (inq_bl->max_ws_len - 1),
                                   block_size, 0, 0, 0, 0, 0, scratch,
                                   EXPECT_STATUS_GOOD));
        } else {
            logging(LOG_VERBOSE, "[SKIPPING] No MAXIMUM WRITE SAME LENGTH - "
                    "skipping MAXIMUM WRITE SAME LENGTH asserts.");
        }

        logging(LOG_VERBOSE, "Test WRITESAME16 0-blocks one block past end-of-LUN");
        WRITESAME16(sd, num_blocks + 1, block_size, 0, 0, 0, 0, 0, scratch,
                    EXPECT_LBA_OOB);

        logging(LOG_VERBOSE, "Test WRITESAME16 0-blocks at LBA==2^63");
        WRITESAME16(sd, 0x8000000000000000ULL,
                    block_size, 0, 0, 0, 0, 0, scratch,
                    EXPECT_LBA_OOB);

        logging(LOG_VERBOSE, "Test WRITESAME16 0-blocks at LBA==-1");
        WRITESAME16(sd, -1, block_size, 0, 0, 0, 0, 0, scratch,
                    EXPECT_LBA_OOB);
}
