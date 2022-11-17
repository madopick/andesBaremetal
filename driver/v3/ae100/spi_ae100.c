/*
 * Copyright (c) 2012-2017 Andes Technology Corporation
 * All rights reserved.
 *
 */

#include "spi_ae100.h"

#include "RTE_Device.h"

#define NDS_SPI_DRV_VERSION NDS_DRIVER_VERSION_MAJOR_MINOR(2,9)

// SPI clock for ae100 is 20MHz
#define SPI_CLK    66000000

// SPI transfer operation
#define SPI_SEND                      0x0
#define SPI_RECEIVE                   0x1
#define SPI_TRANSFER                  0x2

#if (!RTE_SPI1)
#error "SPI not enabled in RTE_Device.h!"
#endif

// driver version
static const NDS_DRIVER_VERSION spi_driver_version = {NDS_SPI_API_VERSION, NDS_SPI_DRV_VERSION};

// SPI1
#if (RTE_SPI1)
static SPI_INFO spi1_info = {0};

static const SPI_RESOURCES spi1_resources = {
	{0},
	AE100_SPI,
	IRQ_SPI_VECTOR,
	&spi1_info
};
#endif

// local functions
static uint32_t spi_get_txfifo_size(SPI_RESOURCES *spi) {
	return 2 << ((spi->reg->CONFIG >> 4) & 0x3);
}

static void spi_polling_spiactive(SPI_RESOURCES *spi) {
	while ((spi->reg->STATUS) & 0x1);
}

// SPI driver functions
static NDS_DRIVER_VERSION spi_get_version(void) {
	return spi_driver_version;
}

static NDS_SPI_CAPABILITIES spi_get_capabilities(SPI_RESOURCES *spi) {
	return spi->capabilities;
}

static int32_t spi_initialize(NDS_SPI_SignalEvent_t cb_event, SPI_RESOURCES *spi) {

	if (spi->info->flags & SPI_FLAG_INITIALIZED)
		return NDS_DRIVER_OK;

	// initialize SPI run-time resources
	spi->info->cb_event          = cb_event;

	spi->info->status.busy       = 0U;
	spi->info->status.data_lost  = 0U;
	spi->info->status.mode_fault = 0U;

	spi->info->xfer.rx_buf       = 0U;
	spi->info->xfer.tx_buf       = 0U;
	spi->info->xfer.tx_buf_limit = 0U;
	spi->info->xfer.rx_cnt       = 0U;
	spi->info->xfer.tx_cnt       = 0U;

	spi->info->mode              = 0U;
	spi->info->txfifo_size = spi_get_txfifo_size(spi);

	spi->info->flags = SPI_FLAG_INITIALIZED;  // SPI is initialized

	return NDS_DRIVER_OK;
}

static int32_t spi_uninitialize(SPI_RESOURCES *spi) {

	spi->info->flags = 0U;                    // SPI is uninitialized

	return NDS_DRIVER_OK;
}

static int32_t spi_power_control(NDS_POWER_STATE state, SPI_RESOURCES *spi) {
	uint32_t val;

	switch (state) {
		case NDS_POWER_OFF:
			// disable SPI IRQ
			val = __nds32__mfsr(NDS32_SR_INT_MASK2);
			__nds32__mtsr(val & ~(1 << spi->irq_num), NDS32_SR_INT_MASK2);

			// clear pending SPI interrupts
			__nds32__mtsr(1 << spi->irq_num, NDS32_SR_INT_PEND2);

			// reset SPI and TX/RX FIFOs
			spi->reg->CTRL = (TXFIFORST | RXFIFORST | SPIRST);

			// clear SPI run-time resources
			spi->info->status.busy       = 0U;
			spi->info->status.data_lost  = 0U;
			spi->info->status.mode_fault = 0U;

			spi->info->xfer.rx_buf       = 0U;
			spi->info->xfer.tx_buf       = 0U;
			spi->info->xfer.rx_cnt       = 0U;
			spi->info->xfer.tx_cnt       = 0U;

			spi->info->mode              = 0U;

			spi->info->flags &= ~SPI_FLAG_POWERED;

			break;

		case NDS_POWER_LOW:
			return NDS_DRIVER_ERROR_UNSUPPORTED;

		case NDS_POWER_FULL:
			if ((spi->info->flags & SPI_FLAG_INITIALIZED) == 0U)
				return NDS_DRIVER_ERROR;

			if ((spi->info->flags & SPI_FLAG_POWERED) != 0U)
				return NDS_DRIVER_OK;

			// reset SPI and TX/RX FIFOs
			spi->reg->CTRL = (TXFIFORST | RXFIFORST | SPIRST);

			// clear SPI run-time resources
			spi->info->status.busy       = 0U;
			spi->info->status.data_lost  = 0U;
			spi->info->status.mode_fault = 0U;

			spi->info->xfer.rx_buf       = 0U;
			spi->info->xfer.tx_buf       = 0U;
			spi->info->xfer.rx_cnt       = 0U;
			spi->info->xfer.tx_cnt       = 0U;

			spi->info->mode              = 0U;

			// clear pending SPI interrupts
			__nds32__mtsr(1 << spi->irq_num, NDS32_SR_INT_PEND2);

			// enable SPI IRQ
			val = __nds32__mfsr(NDS32_SR_INT_MASK2);
			__nds32__mtsr(val | (1 << spi->irq_num), NDS32_SR_INT_MASK2);

			// enable GIE
			__nds32__setgie_en();

			spi->info->flags |= SPI_FLAG_POWERED;   // SPI is powered

			break;

		default:
			return NDS_DRIVER_ERROR_UNSUPPORTED;
	}
	return NDS_DRIVER_OK;
}

static int32_t spi_send(const void *data, uint32_t num, SPI_RESOURCES *spi) {
	if ((data == NULL) || (num == 0U))
		return NDS_DRIVER_ERROR_PARAMETER;

	if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
		return NDS_DRIVER_ERROR;

	if (spi->info->status.busy)
		return NDS_DRIVER_ERROR_BUSY;

	// set busy flag
	spi->info->status.busy       = 1U;

	spi->info->status.data_lost  = 0U;
	spi->info->status.mode_fault = 0U;

	spi->info->xfer.tx_buf   = (uint8_t *)data;
	spi->info->xfer.tx_cnt   = 0U;

	spi->info->xfer.transfer_op = SPI_SEND;

	// wait prior transfer finish
	spi_polling_spiactive(spi);

	// set transfer mode to write only and transfer count for write data
	spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRONLY | WR_TRANCNT(num));

	// set TX FIFO threshold to 2
	spi->reg->CTRL = TXTHRES(2);

	// prepare info that ISR needed
	spi->info->xfer.txfifo_refill = spi->info->txfifo_size - 2; // TX FIFO threshold = 2

	if (spi->info->data_bits <= 8) { // data bits = 1....8
		spi->info->xfer.tx_buf_limit = (uint8_t *)((uint32_t)(spi->info->xfer.tx_buf + num));
	} else if (spi->info->data_bits <= 16) { // data bits = 9....16
		spi->info->xfer.tx_buf_limit = (uint8_t *)((uint32_t)(spi->info->xfer.tx_buf + num * 2));
	} else { // data bits = 17....32
		spi->info->xfer.tx_buf_limit = (uint8_t *)((uint32_t)(spi->info->xfer.tx_buf + num * 4));
	}

	// enable interrupts
	spi->reg->INTREN = (SPI_TXFIFOINT | SPI_ENDINT);

	// enable TX FIFO underrun interrupt when slave mode
	if ((spi->info->mode & NDS_SPI_CONTROL_Msk) == NDS_SPI_MODE_SLAVE) {
		spi->reg->INTREN |= SPI_TXFIFOOURINT;
	}

	// trigger transfer when SPI master mode
	if ((spi->info->mode & NDS_SPI_CONTROL_Msk) == NDS_SPI_MODE_MASTER) {
		spi->reg->CMD = 0;
	}

	return NDS_DRIVER_OK;
}

static int32_t spi_receive(void *data, uint32_t num, SPI_RESOURCES *spi) {

	if ((data == NULL) || (num == 0U))
		return NDS_DRIVER_ERROR_PARAMETER;

	if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
		return NDS_DRIVER_ERROR;

	if (spi->info->status.busy)
		return NDS_DRIVER_ERROR_BUSY;

	// set busy flag
	spi->info->status.busy       = 1U;

	spi->info->status.data_lost  = 0U;
	spi->info->status.mode_fault = 0U;

	spi->info->xfer.rx_buf = (uint8_t *)data;
	spi->info->xfer.rx_cnt = 0U;

	spi->info->xfer.transfer_op = SPI_RECEIVE;

	// wait prior transfer finish
	spi_polling_spiactive(spi);

	// set transfer mode to read only and transfer count for read data
	spi->reg->TRANSCTRL = (SPI_TRANSMODE_RDONLY | RD_TRANCNT(num));

	// set RX FIFO threshold to 2
	spi->reg->CTRL = RXTHRES(2);

	// enable interrupts
	spi->reg->INTREN = (SPI_RXFIFOINT | SPI_ENDINT);

	// enable RX FIFO overrun interrupt when slave mode
	if ((spi->info->mode & NDS_SPI_CONTROL_Msk) == NDS_SPI_MODE_SLAVE) {
		spi->reg->INTREN |= SPI_RXFIFOOORINT;
	}

	// trigger transfer when SPI master mode
	if ((spi->info->mode & NDS_SPI_CONTROL_Msk) == NDS_SPI_MODE_MASTER) {
		spi->reg->CMD = 0;
	}

	return NDS_DRIVER_OK;
}

static int32_t spi_transfer(const void *data_out, void *data_in, uint32_t num, SPI_RESOURCES *spi) {
	if ((data_out == NULL) || (data_in == NULL) || (num == 0U))
		return NDS_DRIVER_ERROR_PARAMETER;

	if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
		return NDS_DRIVER_ERROR;

	if (spi->info->status.busy)
		return NDS_DRIVER_ERROR_BUSY;

	// set busy flag
	spi->info->status.busy       = 1U;

	spi->info->status.data_lost  = 0U;
	spi->info->status.mode_fault = 0U;

	spi->info->xfer.rx_buf = (uint8_t *)data_out;
	spi->info->xfer.tx_buf = (uint8_t *)data_in;

	spi->info->xfer.rx_cnt = 0U;
	spi->info->xfer.tx_cnt = 0U;

	spi->info->xfer.transfer_op = SPI_TRANSFER;

	// wait prior transfer finish
	spi_polling_spiactive(spi);

	// set transfer mode to write and read at the same time and transfer count for write/read data
	spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRnRD | WR_TRANCNT(num) | RD_TRANCNT(num));

	// set TX FIFO threshold and RX FIFO threshold to 2
	spi->reg->CTRL = (TXTHRES(2) | RXTHRES(2));

	// prepare info that ISR needed
	spi->info->xfer.txfifo_refill = spi->info->txfifo_size - 2; // TX FIFO threshold = 2

	if (spi->info->data_bits <= 8) {   // data bits = 1....8
		spi->info->xfer.tx_buf_limit = (uint8_t *)((uint32_t)(spi->info->xfer.tx_buf + num));
	} else if (spi->info->data_bits <= 16) { // data bits = 9....16
		spi->info->xfer.tx_buf_limit = (uint8_t *)((uint32_t)(spi->info->xfer.tx_buf + num * 2));
	} else {  // data bits = 17....32
		spi->info->xfer.tx_buf_limit = (uint8_t *)((uint32_t)(spi->info->xfer.tx_buf + num * 4));
	}

	// enable interrupts
	spi->reg->INTREN = (SPI_TXFIFOINT | SPI_RXFIFOINT | SPI_ENDINT);

	// enable TX FIFO underrun and RX FIFO overrun interrupt when slave mode
	if ((spi->info->mode & NDS_SPI_CONTROL_Msk) == NDS_SPI_MODE_SLAVE) {
		spi->reg->INTREN |= (SPI_TXFIFOOURINT | SPI_RXFIFOOORINT);
	}

	// trigger transfer when SPI master mode
	if ((spi->info->mode & NDS_SPI_CONTROL_Msk) == NDS_SPI_MODE_MASTER) {
		spi->reg->CMD = 0;
	}

	return NDS_DRIVER_OK;
}

static int32_t spi_get_data_count(SPI_RESOURCES *spi) {

	if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
		return 0U;

	switch (spi->info->xfer.transfer_op) {
		case SPI_SEND:
		case SPI_TRANSFER:
			return (spi->info->xfer.tx_cnt);
		case SPI_RECEIVE:
			return (spi->info->xfer.rx_cnt);
		default:
			return NDS_DRIVER_OK;
	}
}

static int32_t spi_control(uint32_t control, uint32_t arg, SPI_RESOURCES *spi) {
	uint32_t sclk_div;

	if (!(spi->info->flags & SPI_FLAG_POWERED))
		return NDS_DRIVER_ERROR;

	if ((control & NDS_SPI_CONTROL_Msk) == NDS_SPI_ABORT_TRANSFER) {    // abort SPI transfer
		// disable SPI interrupts
		spi->reg->INTREN = 0;

		// clear SPI run-time resources
		spi->info->status.busy = 0U;

		spi->info->xfer.rx_buf = 0U;
		spi->info->xfer.tx_buf = 0U;
		spi->info->xfer.rx_cnt = 0U;
		spi->info->xfer.tx_cnt = 0U;

		spi->info->mode        = 0U;

		return NDS_DRIVER_OK;
	}

	if (spi->info->status.busy)
		return NDS_DRIVER_ERROR_BUSY;

	switch (control & NDS_SPI_CONTROL_Msk) {
		default:
			return NDS_DRIVER_ERROR_UNSUPPORTED;

		case NDS_SPI_MODE_MASTER_SIMPLEX:  // SPI master (output/input on MOSI); arg = bus speed in bps
		case NDS_SPI_MODE_SLAVE_SIMPLEX:   // SPI slave (output/input on MISO
		case NDS_SPI_SET_DEFAULT_TX_VALUE: // set default Transmit value; arg = value
		case NDS_SPI_CONTROL_SS:           // control slave select; arg = 0:inactive, 1:active
		       return NDS_SPI_ERROR_MODE;

		case NDS_SPI_MODE_INACTIVE: // SPI inactive
			spi->info->mode &= ~NDS_SPI_CONTROL_Msk;
			spi->info->mode |= NDS_SPI_MODE_INACTIVE;

			spi->info->flags &= ~SPI_FLAG_CONFIGURED;

			return NDS_DRIVER_OK;

		case NDS_SPI_MODE_MASTER: // SPI master (output on MOSI, input on MISO); arg = bus speed in bps
			// set master mode and disable data merge mode
			spi->reg->TRANSFMT &= ~(SPI_MERGE | SPI_SLAVE);

			spi->info->mode &= ~NDS_SPI_CONTROL_Msk;
			spi->info->mode |= NDS_SPI_MODE_MASTER;

			spi->info->flags |= SPI_FLAG_CONFIGURED;

			goto set_speed;

		case NDS_SPI_MODE_SLAVE: // SPI slave (output on MISO, input on MOSI)
			// set slave mode and disable data merge mode
			spi->reg->TRANSFMT &= ~SPI_MERGE;
			spi->reg->TRANSFMT |= SPI_SLAVE;

			spi->info->mode &= ~NDS_SPI_CONTROL_Msk;
			spi->info->mode |= NDS_SPI_MODE_SLAVE;

			spi->info->flags |= SPI_FLAG_CONFIGURED;

			break;

		case NDS_SPI_SET_BUS_SPEED: // set bus speed in bps; arg = value
set_speed:
			if (arg == 0U)
				return NDS_DRIVER_ERROR;

			sclk_div = (SPI_CLK / (2 * arg)) - 1;

			spi->reg->TIMING &= ~0xff;
			spi->reg->TIMING |= sclk_div;

			if ((control & NDS_SPI_CONTROL_Msk) == NDS_SPI_SET_BUS_SPEED)
				return NDS_DRIVER_OK;

			break;

		case NDS_SPI_GET_BUS_SPEED: // get bus speed in bps
			sclk_div = spi->reg->TIMING & 0xff;
			return (SPI_CLK / ((sclk_div + 1) * 2));
	}

	// SPI slave select mode for master
	if ((spi->info->mode & NDS_SPI_CONTROL_Msk) ==  NDS_SPI_MODE_MASTER) {
		switch (control & NDS_SPI_SS_MASTER_MODE_Msk) {
			case NDS_SPI_SS_MASTER_HW_OUTPUT:
				break;
			case NDS_SPI_SS_MASTER_UNUSED:
			case NDS_SPI_SS_MASTER_SW:
			case NDS_SPI_SS_MASTER_HW_INPUT:
				return NDS_SPI_ERROR_SS_MODE;
			default:
				break;
		}
	}

	// SPI slave select mode for slave
	if ((spi->info->mode & NDS_SPI_CONTROL_Msk) == NDS_SPI_MODE_SLAVE) {
		switch (control & NDS_SPI_SS_SLAVE_MODE_Msk) {
			case NDS_SPI_SS_SLAVE_HW:
				break;
			case NDS_SPI_SS_SLAVE_SW:
				return NDS_SPI_ERROR_SS_MODE;
			default:
				break;
		}
	}

	// set SPI frame format
	switch (control & NDS_SPI_FRAME_FORMAT_Msk) {
		case NDS_SPI_CPOL0_CPHA0:
			spi->reg->TRANSFMT &= ~(SPI_CPOL | SPI_CPHA);
			break;
		case NDS_SPI_CPOL0_CPHA1:
			spi->reg->TRANSFMT &= ~SPI_CPOL;
			spi->reg->TRANSFMT |= SPI_CPHA;
			break;
		case NDS_SPI_CPOL1_CPHA0:
			spi->reg->TRANSFMT |= SPI_CPOL;
			spi->reg->TRANSFMT &= ~SPI_CPHA;
			break;
		case NDS_SPI_CPOL1_CPHA1:
			spi->reg->TRANSFMT |= SPI_CPOL;
			spi->reg->TRANSFMT |= SPI_CPHA;
			break;
		case NDS_SPI_TI_SSI:
		case NDS_SPI_MICROWIRE:
		default:
			return NDS_SPI_ERROR_FRAME_FORMAT;
	}

	// set number of data bits
	spi->info->data_bits = ((control & NDS_SPI_DATA_BITS_Msk) >> NDS_SPI_DATA_BITS_Pos);

	if ((spi->info->data_bits < 1U) || (spi->info->data_bits > 32U))
		return NDS_SPI_ERROR_DATA_BITS;
	else
		spi->reg->TRANSFMT |= DATA_BITS(spi->info->data_bits);

	// set SPI bit order
	if ((control & NDS_SPI_BIT_ORDER_Msk) == NDS_SPI_LSB_MSB)
		spi->reg->TRANSFMT |= SPI_LSB;
	else
		spi->reg->TRANSFMT &= ~SPI_LSB;

	return NDS_DRIVER_OK;
}

static NDS_SPI_STATUS spi_get_status(SPI_RESOURCES *spi) {
	NDS_SPI_STATUS status;

	status.busy       = spi->info->status.busy;
	status.data_lost  = spi->info->status.data_lost;
	status.mode_fault = spi->info->status.mode_fault;

	return (status);
}

static void spi_irq_handler(SPI_RESOURCES *spi) {
	uint32_t i, j, status;
	uint32_t data = 0;
	uint32_t rx_num = 0;
	uint32_t event = 0;

	// read status register
	status = spi->reg->INTRST;

	if ((status & SPI_RXFIFOOORINT) || (status & SPI_TXFIFOOURINT)) {
		// TX FIFO underrun or RX FIFO overrun interrupt status
		spi->info->status.data_lost = 1U;

		event |= NDS_SPI_EVENT_DATA_LOST;
	}

	if (status & SPI_TXFIFOINT) {
		for (i = 0; i < spi->info->xfer.txfifo_refill; i++) {
			data = 0;
			if (spi->info->xfer.tx_buf < spi->info->xfer.tx_buf_limit) {
				// handle the data frame format
				if (spi->info->data_bits <= 8) { // data bits = 1....8
					data = *spi->info->xfer.tx_buf;
					spi->info->xfer.tx_buf++;
				} else if (spi->info->data_bits <= 16) { // data bits = 9....16
					for (j = 0; j < 2; j++) {
						data |= *spi->info->xfer.tx_buf << j * 8;
						spi->info->xfer.tx_buf++;
					}
				} else { // data bits = 17....32
					for (j = 0; j < 4; j++) {
						data |= *spi->info->xfer.tx_buf << j * 8;
						spi->info->xfer.tx_buf++;
					}
				}
				spi->reg->DATA = data;
				spi->info->xfer.tx_cnt++;
			} else {
				spi->reg->INTREN &= ~SPI_TXFIFOINT;
			}
		}
	}

	if (status & SPI_RXFIFOINT) {
		// get number of valid entries in the RX FIFO
		rx_num = (spi->reg->STATUS >> 8) & 0x1f;

		for (i = rx_num; i > 0; i--) {
			data = spi->reg->DATA;
			spi->info->xfer.rx_cnt++;

			// handle the data frame format
			if (spi->info->data_bits <= 8) {
				*spi->info->xfer.rx_buf = data & 0xff;
				spi->info->xfer.rx_buf++;
			} else if (spi->info->data_bits <= 16) {
				for (j = 0; j < 2; j++) {
					*spi->info->xfer.rx_buf = (data >> j * 8) & 0xff;
					spi->info->xfer.rx_buf++;
				}
			} else {
				for (j = 0; j < 4; j++) {
					*spi->info->xfer.rx_buf = (data >> j * 8) & 0xff;
					spi->info->xfer.rx_buf++;
				}
			}
		}
	}

	if (status & SPI_ENDINT) {
		// disable SPI interrupts
		spi->reg->INTREN = 0;

		// clear TX/RX FIFOs
		spi->reg->CTRL = (TXFIFORST | RXFIFORST);

		spi->info->status.busy = 0U;

		event |= NDS_SPI_EVENT_TRANSFER_COMPLETE;
	}

	// clear interrupt status
	spi->reg->INTRST = status;
	// make sure "write 1 clear" take effect before iret
	spi->reg->INTRST;

	if ((spi->info->cb_event != NULL) && (event != 0))
		spi->info->cb_event(event);

}


#if (RTE_SPI1)
// SPI1 driver wrapper functions
static NDS_SPI_CAPABILITIES spi1_get_capabilities(void) {
	return spi_get_capabilities(&spi1_resources);
}

static int32_t spi1_initialize(NDS_SPI_SignalEvent_t cb_event) {
	return spi_initialize(cb_event, &spi1_resources);
}

static int32_t spi1_uninitialize (void) {
	return spi_uninitialize(&spi1_resources);
}

static int32_t spi1_power_control(NDS_POWER_STATE state) {
	return spi_power_control(state, &spi1_resources);
}

static int32_t spi1_send(const void *data, uint32_t num) {
	return spi_send(data, num, &spi1_resources);
}

static int32_t spi1_receive(void *data, uint32_t num) {
	return spi_receive(data, num, &spi1_resources);
}

static int32_t spi1_transfer(const void *data_out, void *data_in, uint32_t num) {
	return spi_transfer(data_out, data_in, num, &spi1_resources);
}

static uint32_t spi1_get_data_count(void) {
	return spi_get_data_count(&spi1_resources);
}

static int32_t spi1_control(uint32_t control, uint32_t arg) {
	return spi_control(control, arg, &spi1_resources);
}

static NDS_SPI_STATUS spi1_get_status(void) {
	return spi_get_status(&spi1_resources);
}

void spi1_irq_handler(void) {
	spi_irq_handler(&spi1_resources);
}

// SPI1 driver control block
NDS_DRIVER_SPI Driver_SPI1 = {
	spi_get_version,
	spi1_get_capabilities,
	spi1_initialize,
	spi1_uninitialize,
	spi1_power_control,
	spi1_send,
	spi1_receive,
	spi1_transfer,
	spi1_get_data_count,
	spi1_control,
	spi1_get_status
};
#endif
