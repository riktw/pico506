// Copyright (c) Kuba Szczodrzyński 2025-9-7.

#pragma once

#include <hardware/dma.h>

static inline void st506_pio_init_ctrl_chan(uint ctrl_chan, uint work_chan) {
	// Control Channel
	// - copy from the Control Block to Worker Channel's read+write addresses (and trigger)
	// - wrap write address (DMA registers)
	// - setting a read address (re)starts the cycle
	dma_channel_config ctrl_config = dma_channel_get_default_config(ctrl_chan);
	channel_config_set_transfer_data_size(&ctrl_config, DMA_SIZE_32);
	channel_config_set_read_increment(&ctrl_config, true);
	channel_config_set_write_increment(&ctrl_config, true);
	channel_config_set_ring(&ctrl_config, true, 3); // (1<<3) = 8 bytes
	dma_channel_configure(
		ctrl_chan,
		&ctrl_config,
		/* write_addr= */ &dma_hw->ch[work_chan].al2_read_addr,
		/* read_addr= */ NULL,
		/* transfer_count= */ 2,
		/* trigger= */ false
	);

	// Worker Channel
	// - make one transfer between dynamically defined read/write addresses (from Control Blocks)
	// - chain to Control Channel (to use next Control Block)
	dma_channel_config work_config = dma_channel_get_default_config(work_chan);
	channel_config_set_transfer_data_size(&work_config, DMA_SIZE_32);
	channel_config_set_read_increment(&work_config, false);
	channel_config_set_write_increment(&work_config, false);
	channel_config_set_chain_to(&work_config, ctrl_chan);
	dma_channel_configure(
		work_chan,
		&work_config,
		/* write_addr= */ NULL,
		/* read_addr= */ NULL,
		/* transfer_count= */ 1,
		/* trigger= */ false
	);
}
