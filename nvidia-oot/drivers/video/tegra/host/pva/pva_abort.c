// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022-2023, NVIDIA CORPORATION.  All rights reserved.
 */

#include <linux/nvhost.h>
#include <linux/wait.h>

#include "pva.h"
#include "pva_sec_ec.h"

static void pva_abort_handler(struct work_struct *work)
{
	struct pva *pva = container_of(work, struct pva,
				       pva_abort_handler_work);
	struct platform_device *pdev = pva->pdev;
	int i;

	/* Dump nvhost state to show the pending jobs */
	nvhost_debug_dump_device(pdev);


	/*wake up sync cmd waiters*/
        for (i = 0; i < pva->version_config->irq_count; i++) {
		if (pva->cmd_status[i] == PVA_CMD_STATUS_WFI) {
			pva->cmd_status[i] = PVA_CMD_STATUS_ABORTED;
			wake_up(&pva->cmd_waitqueue[i]);
			schedule();
		}
	}

	/* lock mailbox mutex to avoid synchronous communication. */
	do {
		schedule();
	} while (mutex_trylock(&pva->mailbox_mutex) == false);

        /* There is no ongoing activity anymore. Update mailbox status */
        for (i = 0; i < pva->version_config->irq_count; i++) {
            pva->cmd_status[i] = PVA_CMD_STATUS_INVALID;
        }

        /* Lock CCQ mutex to avoid asynchornous communication */
	mutex_lock(&pva->ccq_mutex);

	/*
	 * If boot was still on-going, skip over recovery and let boot-up
	 * routine handle the failure
	 */
	if (!pva->booted) {
		nvpva_warn(&pdev->dev, "Recovery skipped: PVA is not booted");
		goto skip_recovery;
	}

	/* disable error reporting to hsm*/
	pva_disable_ec_err_reporting(pva);

	/* Reset the PVA and reload firmware */
	nvhost_module_reset(pdev, true);

	/* enable error reporting to hsm*/
	pva_enable_ec_err_reporting(pva);

	/* Remove pending tasks from the queue */
	nvpva_queue_abort_all(pva->pool);

	nvpva_warn(&pdev->dev, "Recovery finished");

skip_recovery:
	mutex_unlock(&pva->ccq_mutex);
	mutex_unlock(&pva->mailbox_mutex);
}

void pva_abort(struct pva *pva)
{
	struct platform_device *pdev = pva->pdev;
	size_t i;
	/* For selftest mode to finish the test */
	if (host1x_readl(pdev, hsp_ss0_state_r())
		& PVA_TEST_MODE) {
		for (i = 0; i < pva->version_config->irq_count; i++) {
			pva->cmd_status[i] = PVA_CMD_STATUS_DONE;
			wake_up(&pva->cmd_waitqueue[i]);
		}
		return;
	}

	WARN(true, "Attempting to recover the engine");
	schedule_work(&pva->pva_abort_handler_work);
}

void pva_abort_init(struct pva *pva)
{
	INIT_WORK(&pva->pva_abort_handler_work, pva_abort_handler);
}
