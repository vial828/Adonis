/***************************************************************************//**
* \file cyhal_pse84_lqfp_144.c
*
* \brief
* PSE84 device GPIO HAL header for LQFP-144 package
*
********************************************************************************
* \copyright
* (c) (2016-2023), Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.
*
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include "cy_device_headers.h"
#include "cyhal_hw_types.h"

#if defined(_GPIO_PSE84_LQFP_144_H_)
#include "pin_packages/cyhal_pse84_lqfp_144.h"

/* Pin connections */
/* Connections for: canfd_ttcan_rx */
const cyhal_resource_pin_mapping_t cyhal_pin_map_canfd_ttcan_rx[3] = {
    {0u, 0u, P16_0, P16_0_CANFD0_TTCAN_RX0},
    {0u, 1u, P16_2, P16_2_CANFD0_TTCAN_RX1},
    {0u, 0u, P20_4, P20_4_CANFD0_TTCAN_RX0},
};

/* Connections for: canfd_ttcan_tx */
const cyhal_resource_pin_mapping_t cyhal_pin_map_canfd_ttcan_tx[2] = {
    {0u, 0u, P16_1, P16_1_CANFD0_TTCAN_TX0},
    {0u, 1u, P16_3, P16_3_CANFD0_TTCAN_TX1},
};

/* Connections for: debug600_clk_swj_swclk_tclk */
const cyhal_resource_pin_mapping_t cyhal_pin_map_debug600_clk_swj_swclk_tclk[1] = {
    {0u, 0u, P6_3, P6_3_DEBUG600_CLK_SWJ_SWCLK_TCLK},
};

/* Connections for: debug600_rst_swj_trstn */
const cyhal_resource_pin_mapping_t cyhal_pin_map_debug600_rst_swj_trstn[1] = {
    {0u, 0u, P7_2, P7_2_DEBUG600_RST_SWJ_TRSTN},
};

/* Connections for: debug600_swj_swdio_tms */
const cyhal_resource_pin_mapping_t cyhal_pin_map_debug600_swj_swdio_tms[1] = {
    {0u, 0u, P6_2, P6_2_DEBUG600_SWJ_SWDIO_TMS},
};

/* Connections for: debug600_swj_swdoe_tdi */
const cyhal_resource_pin_mapping_t cyhal_pin_map_debug600_swj_swdoe_tdi[1] = {
    {0u, 0u, P6_1, P6_1_DEBUG600_SWJ_SWDOE_TDI},
};

/* Connections for: debug600_swj_swo_tdo */
const cyhal_resource_pin_mapping_t cyhal_pin_map_debug600_swj_swo_tdo[1] = {
    {0u, 0u, P6_0, P6_0_DEBUG600_SWJ_SWO_TDO},
};

/* Connections for: debug600_trace_clock */
const cyhal_resource_pin_mapping_t cyhal_pin_map_debug600_trace_clock[1] = {
    {0u, 0u, P20_0, P20_0_DEBUG600_TRACE_CLOCK},
};

/* Connections for: debug600_trace_data */
const cyhal_resource_pin_mapping_t cyhal_pin_map_debug600_trace_data[4] = {
    {0u, 3u, P20_1, P20_1_DEBUG600_TRACE_DATA3},
    {0u, 2u, P20_2, P20_2_DEBUG600_TRACE_DATA2},
    {0u, 1u, P20_3, P20_3_DEBUG600_TRACE_DATA1},
    {0u, 0u, P20_4, P20_4_DEBUG600_TRACE_DATA0},
};

/* Connections for: dft_ROdiv8k */
const cyhal_resource_pin_mapping_t cyhal_pin_map_dft_ROdiv8k[1] = {
    {0u, 0u, P2_0, P2_0_DFT_RODIV8K},
};

/* Connections for: eth_eth_tsu_timer_cmp_val */
const cyhal_resource_pin_mapping_t cyhal_pin_map_eth_eth_tsu_timer_cmp_val[1] = {
    {0u, 0u, NC, HSIOM_SEL_GPIO},
};

/* Connections for: eth_mdc */
const cyhal_resource_pin_mapping_t cyhal_pin_map_eth_mdc[1] = {
    {0u, 0u, P13_1, P13_1_ETH_MDC},
};

/* Connections for: eth_mdio */
const cyhal_resource_pin_mapping_t cyhal_pin_map_eth_mdio[1] = {
    {0u, 0u, P13_6, P13_6_ETH_MDIO},
};

/* Connections for: eth_ref_clk */
const cyhal_resource_pin_mapping_t cyhal_pin_map_eth_ref_clk[1] = {
    {0u, 0u, P11_7, P11_7_ETH_REF_CLK},
};

/* Connections for: eth_rx_clk */
const cyhal_resource_pin_mapping_t cyhal_pin_map_eth_rx_clk[1] = {
    {0u, 0u, NC, HSIOM_SEL_GPIO},
};

/* Connections for: eth_rx_ctl */
const cyhal_resource_pin_mapping_t cyhal_pin_map_eth_rx_ctl[1] = {
    {0u, 0u, P10_5, P10_5_ETH_RX_CTL},
};

/* Connections for: eth_rx_er */
const cyhal_resource_pin_mapping_t cyhal_pin_map_eth_rx_er[1] = {
    {0u, 0u, P11_1, P11_1_ETH_RX_ER},
};

/* Connections for: eth_rxd */
const cyhal_resource_pin_mapping_t cyhal_pin_map_eth_rxd[3] = {
    {0u, 0u, P10_6, P10_6_ETH_RXD0},
    {0u, 1u, P10_7, P10_7_ETH_RXD1},
    {0u, 2u, P11_0, P11_0_ETH_RXD2},
};

/* Connections for: eth_tx_clk */
const cyhal_resource_pin_mapping_t cyhal_pin_map_eth_tx_clk[1] = {
    {0u, 0u, P11_5, P11_5_ETH_TX_CLK},
};

/* Connections for: eth_tx_ctl */
const cyhal_resource_pin_mapping_t cyhal_pin_map_eth_tx_ctl[1] = {
    {0u, 0u, P11_4, P11_4_ETH_TX_CTL},
};

/* Connections for: eth_tx_er */
const cyhal_resource_pin_mapping_t cyhal_pin_map_eth_tx_er[1] = {
    {0u, 0u, NC, HSIOM_SEL_GPIO},
};

/* Connections for: eth_txd */
const cyhal_resource_pin_mapping_t cyhal_pin_map_eth_txd[3] = {
    {0u, 0u, P11_2, P11_2_ETH_TXD0},
    {0u, 1u, P11_3, P11_3_ETH_TXD1},
    {0u, 2u, P11_6, P11_6_ETH_TXD2},
};

/* Connections for: gfxss_dbi_csx */
const cyhal_resource_pin_mapping_t cyhal_pin_map_gfxss_dbi_csx[1] = {
    {0u, 0u, P16_0, P16_0_GFXSS_DBI_CSX},
};

/* Connections for: gfxss_dbi_d */
const cyhal_resource_pin_mapping_t cyhal_pin_map_gfxss_dbi_d[8] = {
    {0u, 0u, P16_3, P16_3_GFXSS_DBI_D0},
    {0u, 1u, P16_4, P16_4_GFXSS_DBI_D1},
    {0u, 2u, P16_5, P16_5_GFXSS_DBI_D2},
    {0u, 3u, P16_6, P16_6_GFXSS_DBI_D3},
    {0u, 4u, P16_7, P16_7_GFXSS_DBI_D4},
    {0u, 5u, P17_0, P17_0_GFXSS_DBI_D5},
    {0u, 6u, P17_1, P17_1_GFXSS_DBI_D6},
    {0u, 7u, P17_2, P17_2_GFXSS_DBI_D7},
};

/* Connections for: gfxss_dbi_dcx */
const cyhal_resource_pin_mapping_t cyhal_pin_map_gfxss_dbi_dcx[1] = {
    {0u, 0u, P16_2, P16_2_GFXSS_DBI_DCX},
};

/* Connections for: gfxss_dbi_e */
const cyhal_resource_pin_mapping_t cyhal_pin_map_gfxss_dbi_e[1] = {
    {0u, 0u, P16_1, P16_1_GFXSS_DBI_E},
};

/* Connections for: gfxss_dbi_wrx */
const cyhal_resource_pin_mapping_t cyhal_pin_map_gfxss_dbi_wrx[1] = {
    {0u, 0u, P16_1, P16_1_GFXSS_DBI_WRX},
};

/* Connections for: gfxss_spi_csx */
const cyhal_resource_pin_mapping_t cyhal_pin_map_gfxss_spi_csx[1] = {
    {0u, 0u, P16_0, P16_0_GFXSS_SPI_CSX},
};

/* Connections for: gfxss_spi_dcx */
const cyhal_resource_pin_mapping_t cyhal_pin_map_gfxss_spi_dcx[1] = {
    {0u, 0u, P16_3, P16_3_GFXSS_SPI_DCX},
};

/* Connections for: gfxss_spi_dout */
const cyhal_resource_pin_mapping_t cyhal_pin_map_gfxss_spi_dout[1] = {
    {0u, 0u, P16_2, P16_2_GFXSS_SPI_DOUT},
};

/* Connections for: gfxss_spi_scl */
const cyhal_resource_pin_mapping_t cyhal_pin_map_gfxss_spi_scl[1] = {
    {0u, 0u, P16_1, P16_1_GFXSS_SPI_SCL},
};

/* Connections for: i3c_i3c_scl */
const cyhal_resource_pin_mapping_t cyhal_pin_map_i3c_i3c_scl[1] = {
    {0u, 0u, NC, HSIOM_SEL_GPIO},
};

/* Connections for: i3c_i3c_sda */
const cyhal_resource_pin_mapping_t cyhal_pin_map_i3c_i3c_sda[1] = {
    {0u, 0u, NC, HSIOM_SEL_GPIO},
};

/* Connections for: lpcomp_inn_comp */
const cyhal_resource_pin_mapping_t cyhal_pin_map_lpcomp_inn_comp[2] = {
    {0u, 0u, P10_5, HSIOM_SEL_GPIO},
    {0u, 1u, P10_7, HSIOM_SEL_GPIO},
};

/* Connections for: lpcomp_inp_comp */
const cyhal_resource_pin_mapping_t cyhal_pin_map_lpcomp_inp_comp[1] = {
    {0u, 1u, P10_6, HSIOM_SEL_GPIO},
};

/* Connections for: m0seccpuss_clk_m0sec_swd */
const cyhal_resource_pin_mapping_t cyhal_pin_map_m0seccpuss_clk_m0sec_swd[1] = {
    {0u, 0u, P8_1, P8_1_M0SECCPUSS_CLK_M0SEC_SWD},
};

/* Connections for: m0seccpuss_m0sec_swd */
const cyhal_resource_pin_mapping_t cyhal_pin_map_m0seccpuss_m0sec_swd[1] = {
    {0u, 0u, P8_0, P8_0_M0SECCPUSS_M0SEC_SWD},
};

/* Connections for: m33syscpuss_fault */
const cyhal_resource_pin_mapping_t cyhal_pin_map_m33syscpuss_fault[1] = {
    {0u, 0u, NC, HSIOM_SEL_GPIO},
};

/* Connections for: opamp_out_10x */
const cyhal_resource_pin_mapping_t cyhal_pin_map_opamp_out_10x[2] = {
    {1u, 0u, P13_2, HSIOM_SEL_GPIO},
    {1u, 1u, P13_3, HSIOM_SEL_GPIO},
};

/* Connections for: opamp_vin_m0 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_opamp_vin_m0[2] = {
    {1u, 0u, P13_0, HSIOM_SEL_GPIO},
    {1u, 1u, P13_5, HSIOM_SEL_GPIO},
};

/* Connections for: opamp_vin_m1 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_opamp_vin_m1[2] = {
    {1u, 0u, P13_1, HSIOM_SEL_GPIO},
    {1u, 1u, P13_4, HSIOM_SEL_GPIO},
};

/* Connections for: opamp_vin_p0 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_opamp_vin_p0[2] = {
    {1u, 0u, P13_0, HSIOM_SEL_GPIO},
    {1u, 1u, P13_5, HSIOM_SEL_GPIO},
};

/* Connections for: opamp_vin_p1 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_opamp_vin_p1[2] = {
    {1u, 0u, P13_1, HSIOM_SEL_GPIO},
    {1u, 1u, P13_4, HSIOM_SEL_GPIO},
};

/* Connections for: pass_dac_pads */
const cyhal_resource_pin_mapping_t cyhal_pin_map_pass_dac_pads[1] = {
    {0u, 1u, P13_6, HSIOM_SEL_GPIO},
};

/* Connections for: pass_lppass_dout */
const cyhal_resource_pin_mapping_t cyhal_pin_map_pass_lppass_dout[5] = {
    {0u, 0u, P17_4, P17_4_PASS_LPPASS_DOUT0},
    {0u, 1u, P17_5, P17_5_PASS_LPPASS_DOUT1},
    {0u, 2u, P17_6, P17_6_PASS_LPPASS_DOUT2},
    {0u, 3u, P17_7, P17_7_PASS_LPPASS_DOUT3},
    {0u, 0u, P20_4, P20_4_PASS_LPPASS_DOUT0},
};

/* Connections for: pass_lppass_obsrv */
const cyhal_resource_pin_mapping_t cyhal_pin_map_pass_lppass_obsrv[5] = {
    {0u, 0u, P20_0, P20_0_PASS_LPPASS_OBSRV0},
    {0u, 1u, P20_1, P20_1_PASS_LPPASS_OBSRV1},
    {0u, 2u, P20_2, P20_2_PASS_LPPASS_OBSRV2},
    {0u, 3u, P20_3, P20_3_PASS_LPPASS_OBSRV3},
    {0u, 4u, P20_4, P20_4_PASS_LPPASS_OBSRV4},
};

/* Connections for: pass_ptc_pads */
const cyhal_resource_pin_mapping_t cyhal_pin_map_pass_ptc_pads[8] = {
    {0u, 0u, P15_0, HSIOM_SEL_GPIO},
    {0u, 1u, P15_1, HSIOM_SEL_GPIO},
    {0u, 2u, P15_2, HSIOM_SEL_GPIO},
    {0u, 3u, P15_3, HSIOM_SEL_GPIO},
    {0u, 4u, P15_4, HSIOM_SEL_GPIO},
    {0u, 5u, P15_5, HSIOM_SEL_GPIO},
    {0u, 6u, P15_6, HSIOM_SEL_GPIO},
    {0u, 7u, P15_7, HSIOM_SEL_GPIO},
};

/* Connections for: pass_sarmux_pads */
const cyhal_resource_pin_mapping_t cyhal_pin_map_pass_sarmux_pads[8] = {
    {0u, 0u, P15_0, HSIOM_SEL_GPIO},
    {0u, 1u, P15_1, HSIOM_SEL_GPIO},
    {0u, 2u, P15_2, HSIOM_SEL_GPIO},
    {0u, 3u, P15_3, HSIOM_SEL_GPIO},
    {0u, 4u, P15_4, HSIOM_SEL_GPIO},
    {0u, 5u, P15_5, HSIOM_SEL_GPIO},
    {0u, 6u, P15_6, HSIOM_SEL_GPIO},
    {0u, 7u, P15_7, HSIOM_SEL_GPIO},
};

/* Connections for: pdm_pdm_clk */
const cyhal_resource_pin_mapping_t cyhal_pin_map_pdm_pdm_clk[5] = {
    {0u, 2u, P8_2, P8_2_PDM_PDM_CLK2},
    {0u, 3u, P8_5, P8_5_PDM_PDM_CLK3},
    {0u, 5u, P9_0, P9_0_PDM_PDM_CLK5},
    {0u, 4u, P9_2, P9_2_PDM_PDM_CLK4},
    {0u, 1u, P13_2, P13_2_PDM_PDM_CLK1},
};

/* Connections for: pdm_pdm_data */
const cyhal_resource_pin_mapping_t cyhal_pin_map_pdm_pdm_data[5] = {
    {0u, 2u, P8_4, P8_4_PDM_PDM_DATA2},
    {0u, 3u, P8_6, P8_6_PDM_PDM_DATA3},
    {0u, 5u, P9_1, P9_1_PDM_PDM_DATA5},
    {0u, 4u, P9_3, P9_3_PDM_PDM_DATA4},
    {0u, 1u, P13_3, P13_3_PDM_PDM_DATA1},
};

/* Connections for: peri_tr_io_input */
/* The actual channel_num has no value to how the connection works. However, the HAL driver needs
   to know the index of the input or output trigger line. Store that in the channel_num field
   instead. */
const cyhal_resource_pin_mapping_t cyhal_pin_map_peri_tr_io_input[10] = {
    {0u, 7u, P8_0, P8_0_PERI0_TR_IO_INPUT7},
    {1u, 7u, P8_0, P8_0_PERI1_TR_IO_INPUT7},
    {0u, 0u, P11_0, P11_0_PERI0_TR_IO_INPUT0},
    {1u, 0u, P11_0, P11_0_PERI1_TR_IO_INPUT0},
    {0u, 1u, P11_1, P11_1_PERI0_TR_IO_INPUT1},
    {1u, 1u, P11_1, P11_1_PERI1_TR_IO_INPUT1},
    {0u, 2u, P11_2, P11_2_PERI0_TR_IO_INPUT2},
    {1u, 2u, P11_2, P11_2_PERI1_TR_IO_INPUT2},
    {0u, 3u, P11_3, P11_3_PERI0_TR_IO_INPUT3},
    {1u, 3u, P11_3, P11_3_PERI1_TR_IO_INPUT3},
};

/* Connections for: peri_tr_io_output */
/* The actual channel_num has no value to how the connection works. However, the HAL driver needs
   to know the index of the input or output trigger line. Store that in the channel_num field
   instead. */
const cyhal_resource_pin_mapping_t cyhal_pin_map_peri_tr_io_output[1] = {
    {0u, 0u, NC, HSIOM_SEL_GPIO},
};

/* Connections for: scb_i2c_scl */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_i2c_scl[9] = {
    {0u, 0u, P8_0, P8_0_SCB0_I2C_SCL},
    {1u, 0u, P9_3, P9_3_SCB1_I2C_SCL},
    {6u, 0u, P11_0, P11_0_SCB6_I2C_SCL},
    {7u, 0u, P13_1, P13_1_SCB7_I2C_SCL},
    {9u, 0u, P15_0, P15_0_SCB9_I2C_SCL},
    {10u, 0u, P16_0, P16_0_SCB10_I2C_SCL},
    {5u, 0u, P17_0, P17_0_SCB5_I2C_SCL},
    {11u, 0u, P17_2, P17_2_SCB11_I2C_SCL},
    {3u, 0u, P21_6, P21_6_SCB3_I2C_SCL},
};

/* Connections for: scb_i2c_sda */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_i2c_sda[9] = {
    {0u, 0u, P8_1, P8_1_SCB0_I2C_SDA},
    {1u, 0u, P9_2, P9_2_SCB1_I2C_SDA},
    {6u, 0u, P11_1, P11_1_SCB6_I2C_SDA},
    {7u, 0u, P13_2, P13_2_SCB7_I2C_SDA},
    {9u, 0u, P15_1, P15_1_SCB9_I2C_SDA},
    {10u, 0u, P16_1, P16_1_SCB10_I2C_SDA},
    {5u, 0u, P17_1, P17_1_SCB5_I2C_SDA},
    {11u, 0u, P17_3, P17_3_SCB11_I2C_SDA},
    {3u, 0u, P21_5, P21_5_SCB3_I2C_SDA},
};

/* Connections for: scb_spi_m_clk */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_spi_m_clk[9] = {
    {0u, 0u, P8_0, P8_0_SCB0_SPI_CLK},
    {1u, 0u, P9_3, P9_3_SCB1_SPI_CLK},
    {6u, 0u, P11_0, P11_0_SCB6_SPI_CLK},
    {7u, 0u, P13_1, P13_1_SCB7_SPI_CLK},
    {9u, 0u, P15_0, P15_0_SCB9_SPI_CLK},
    {10u, 0u, P16_0, P16_0_SCB10_SPI_CLK},
    {5u, 0u, P17_0, P17_0_SCB5_SPI_CLK},
    {11u, 0u, P17_2, P17_2_SCB11_SPI_CLK},
    {3u, 0u, P21_6, P21_6_SCB3_SPI_CLK},
};

/* Connections for: scb_spi_m_miso */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_spi_m_miso[10] = {
    {0u, 0u, P8_4, P8_4_SCB0_SPI_MISO},
    {1u, 0u, P9_1, P9_1_SCB1_SPI_MISO},
    {6u, 0u, P11_2, P11_2_SCB6_SPI_MISO},
    {7u, 0u, P13_3, P13_3_SCB7_SPI_MISO},
    {8u, 0u, P13_7, P13_7_SCB8_SPI_MISO},
    {9u, 0u, P15_2, P15_2_SCB9_SPI_MISO},
    {10u, 0u, P16_2, P16_2_SCB10_SPI_MISO},
    {5u, 0u, P16_5, P16_5_SCB5_SPI_MISO},
    {11u, 0u, P17_5, P17_5_SCB11_SPI_MISO},
    {3u, 0u, P21_4, P21_4_SCB3_SPI_MISO},
};

/* Connections for: scb_spi_m_mosi */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_spi_m_mosi[9] = {
    {0u, 0u, P8_1, P8_1_SCB0_SPI_MOSI},
    {1u, 0u, P9_2, P9_2_SCB1_SPI_MOSI},
    {6u, 0u, P11_1, P11_1_SCB6_SPI_MOSI},
    {7u, 0u, P13_2, P13_2_SCB7_SPI_MOSI},
    {9u, 0u, P15_1, P15_1_SCB9_SPI_MOSI},
    {10u, 0u, P16_1, P16_1_SCB10_SPI_MOSI},
    {5u, 0u, P17_1, P17_1_SCB5_SPI_MOSI},
    {11u, 0u, P17_3, P17_3_SCB11_SPI_MOSI},
    {3u, 0u, P21_5, P21_5_SCB3_SPI_MOSI},
};

/* Connections for: scb_spi_m_select0 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_spi_m_select0[8] = {
    {0u, 0u, P8_2, P8_2_SCB0_SPI_SELECT0},
    {1u, 0u, P9_0, P9_0_SCB1_SPI_SELECT0},
    {6u, 0u, P11_3, P11_3_SCB6_SPI_SELECT0},
    {7u, 0u, P13_4, P13_4_SCB7_SPI_SELECT0},
    {9u, 0u, P15_3, P15_3_SCB9_SPI_SELECT0},
    {10u, 0u, P16_3, P16_3_SCB10_SPI_SELECT0},
    {5u, 0u, P16_6, P16_6_SCB5_SPI_SELECT0},
    {11u, 0u, P17_6, P17_6_SCB11_SPI_SELECT0},
};

/* Connections for: scb_spi_m_select1 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_spi_m_select1[11] = {
    {2u, 0u, P6_3, P6_3_SCB2_SPI_SELECT1},
    {0u, 0u, P8_3, P8_3_SCB0_SPI_SELECT1},
    {4u, 0u, P10_5, P10_5_SCB4_SPI_SELECT1},
    {6u, 0u, P11_4, P11_4_SCB6_SPI_SELECT1},
    {7u, 0u, P13_6, P13_6_SCB7_SPI_SELECT1},
    {9u, 0u, P15_4, P15_4_SCB9_SPI_SELECT1},
    {10u, 0u, P16_4, P16_4_SCB10_SPI_SELECT1},
    {5u, 0u, P16_7, P16_7_SCB5_SPI_SELECT1},
    {11u, 0u, P17_7, P17_7_SCB11_SPI_SELECT1},
    {1u, 0u, P20_0, P20_0_SCB1_SPI_SELECT1},
    {3u, 0u, P21_7, P21_7_SCB3_SPI_SELECT1},
};

/* Connections for: scb_spi_s_clk */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_spi_s_clk[9] = {
    {0u, 0u, P8_0, P8_0_SCB0_SPI_CLK},
    {1u, 0u, P9_3, P9_3_SCB1_SPI_CLK},
    {6u, 0u, P11_0, P11_0_SCB6_SPI_CLK},
    {7u, 0u, P13_1, P13_1_SCB7_SPI_CLK},
    {9u, 0u, P15_0, P15_0_SCB9_SPI_CLK},
    {10u, 0u, P16_0, P16_0_SCB10_SPI_CLK},
    {5u, 0u, P17_0, P17_0_SCB5_SPI_CLK},
    {11u, 0u, P17_2, P17_2_SCB11_SPI_CLK},
    {3u, 0u, P21_6, P21_6_SCB3_SPI_CLK},
};

/* Connections for: scb_spi_s_miso */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_spi_s_miso[10] = {
    {0u, 0u, P8_4, P8_4_SCB0_SPI_MISO},
    {1u, 0u, P9_1, P9_1_SCB1_SPI_MISO},
    {6u, 0u, P11_2, P11_2_SCB6_SPI_MISO},
    {7u, 0u, P13_3, P13_3_SCB7_SPI_MISO},
    {8u, 0u, P13_7, P13_7_SCB8_SPI_MISO},
    {9u, 0u, P15_2, P15_2_SCB9_SPI_MISO},
    {10u, 0u, P16_2, P16_2_SCB10_SPI_MISO},
    {5u, 0u, P16_5, P16_5_SCB5_SPI_MISO},
    {11u, 0u, P17_5, P17_5_SCB11_SPI_MISO},
    {3u, 0u, P21_4, P21_4_SCB3_SPI_MISO},
};

/* Connections for: scb_spi_s_mosi */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_spi_s_mosi[9] = {
    {0u, 0u, P8_1, P8_1_SCB0_SPI_MOSI},
    {1u, 0u, P9_2, P9_2_SCB1_SPI_MOSI},
    {6u, 0u, P11_1, P11_1_SCB6_SPI_MOSI},
    {7u, 0u, P13_2, P13_2_SCB7_SPI_MOSI},
    {9u, 0u, P15_1, P15_1_SCB9_SPI_MOSI},
    {10u, 0u, P16_1, P16_1_SCB10_SPI_MOSI},
    {5u, 0u, P17_1, P17_1_SCB5_SPI_MOSI},
    {11u, 0u, P17_3, P17_3_SCB11_SPI_MOSI},
    {3u, 0u, P21_5, P21_5_SCB3_SPI_MOSI},
};

/* Connections for: scb_spi_s_select0 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_spi_s_select0[8] = {
    {0u, 0u, P8_2, P8_2_SCB0_SPI_SELECT0},
    {1u, 0u, P9_0, P9_0_SCB1_SPI_SELECT0},
    {6u, 0u, P11_3, P11_3_SCB6_SPI_SELECT0},
    {7u, 0u, P13_4, P13_4_SCB7_SPI_SELECT0},
    {9u, 0u, P15_3, P15_3_SCB9_SPI_SELECT0},
    {10u, 0u, P16_3, P16_3_SCB10_SPI_SELECT0},
    {5u, 0u, P16_6, P16_6_SCB5_SPI_SELECT0},
    {11u, 0u, P17_6, P17_6_SCB11_SPI_SELECT0},
};

/* Connections for: scb_spi_s_select1 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_spi_s_select1[11] = {
    {2u, 0u, P6_3, P6_3_SCB2_SPI_SELECT1},
    {0u, 0u, P8_3, P8_3_SCB0_SPI_SELECT1},
    {4u, 0u, P10_5, P10_5_SCB4_SPI_SELECT1},
    {6u, 0u, P11_4, P11_4_SCB6_SPI_SELECT1},
    {7u, 0u, P13_6, P13_6_SCB7_SPI_SELECT1},
    {9u, 0u, P15_4, P15_4_SCB9_SPI_SELECT1},
    {10u, 0u, P16_4, P16_4_SCB10_SPI_SELECT1},
    {5u, 0u, P16_7, P16_7_SCB5_SPI_SELECT1},
    {11u, 0u, P17_7, P17_7_SCB11_SPI_SELECT1},
    {1u, 0u, P20_0, P20_0_SCB1_SPI_SELECT1},
    {3u, 0u, P21_7, P21_7_SCB3_SPI_SELECT1},
};

/* Connections for: scb_uart_cts */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_uart_cts[9] = {
    {1u, 0u, P9_1, P9_1_SCB1_UART_CTS},
    {6u, 0u, P11_2, P11_2_SCB6_UART_CTS},
    {7u, 0u, P13_3, P13_3_SCB7_UART_CTS},
    {8u, 0u, P13_7, P13_7_SCB8_UART_CTS},
    {9u, 0u, P15_2, P15_2_SCB9_UART_CTS},
    {10u, 0u, P16_2, P16_2_SCB10_UART_CTS},
    {5u, 0u, P16_5, P16_5_SCB5_UART_CTS},
    {11u, 0u, P17_5, P17_5_SCB11_UART_CTS},
    {3u, 0u, P21_4, P21_4_SCB3_UART_CTS},
};

/* Connections for: scb_uart_rts */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_uart_rts[8] = {
    {1u, 0u, P9_0, P9_0_SCB1_UART_RTS},
    {6u, 0u, P11_3, P11_3_SCB6_UART_RTS},
    {7u, 0u, P13_4, P13_4_SCB7_UART_RTS},
    {9u, 0u, P15_3, P15_3_SCB9_UART_RTS},
    {10u, 0u, P16_3, P16_3_SCB10_UART_RTS},
    {5u, 0u, P16_6, P16_6_SCB5_UART_RTS},
    {11u, 0u, P17_6, P17_6_SCB11_UART_RTS},
    {3u, 0u, P21_7, P21_7_SCB3_UART_RTS},
};

/* Connections for: scb_uart_rx */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_uart_rx[8] = {
    {1u, 0u, P9_3, P9_3_SCB1_UART_RX},
    {6u, 0u, P11_0, P11_0_SCB6_UART_RX},
    {7u, 0u, P13_1, P13_1_SCB7_UART_RX},
    {9u, 0u, P15_0, P15_0_SCB9_UART_RX},
    {10u, 0u, P16_0, P16_0_SCB10_UART_RX},
    {5u, 0u, P17_0, P17_0_SCB5_UART_RX},
    {11u, 0u, P17_2, P17_2_SCB11_UART_RX},
    {3u, 0u, P21_6, P21_6_SCB3_UART_RX},
};

/* Connections for: scb_uart_tx */
const cyhal_resource_pin_mapping_t cyhal_pin_map_scb_uart_tx[8] = {
    {1u, 0u, P9_2, P9_2_SCB1_UART_TX},
    {6u, 0u, P11_1, P11_1_SCB6_UART_TX},
    {7u, 0u, P13_2, P13_2_SCB7_UART_TX},
    {9u, 0u, P15_1, P15_1_SCB9_UART_TX},
    {10u, 0u, P16_1, P16_1_SCB10_UART_TX},
    {5u, 0u, P17_1, P17_1_SCB5_UART_TX},
    {11u, 0u, P17_3, P17_3_SCB11_UART_TX},
    {3u, 0u, P21_5, P21_5_SCB3_UART_TX},
};

/* Connections for: sdhc_card_cmd */
const cyhal_resource_pin_mapping_t cyhal_pin_map_sdhc_card_cmd[1] = {
    {0u, 0u, P21_0, P21_0_SDHC0_CARD_CMD},
};

/* Connections for: sdhc_card_dat_3to0 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_sdhc_card_dat_3to0[4] = {
    {0u, 0u, P12_1, P12_1_SDHC0_CARD_DAT_3TO00},
    {0u, 1u, P12_2, P12_2_SDHC0_CARD_DAT_3TO01},
    {0u, 2u, P12_4, P12_4_SDHC0_CARD_DAT_3TO02},
    {0u, 3u, P12_5, P12_5_SDHC0_CARD_DAT_3TO03},
};

/* Connections for: sdhc_card_dat_7to4 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_sdhc_card_dat_7to4[1] = {
    {0u, 0u, NC, HSIOM_SEL_GPIO},
};

/* Connections for: sdhc_card_detect_n */
const cyhal_resource_pin_mapping_t cyhal_pin_map_sdhc_card_detect_n[2] = {
    {1u, 0u, P7_4, P7_4_SDHC1_CARD_DETECT_N},
    {0u, 0u, P21_1, P21_1_SDHC0_CARD_DETECT_N},
};

/* Connections for: sdhc_card_emmc_reset_n */
const cyhal_resource_pin_mapping_t cyhal_pin_map_sdhc_card_emmc_reset_n[1] = {
    {1u, 0u, P7_2, P7_2_SDHC1_CARD_EMMC_RESET_N},
};

/* Connections for: sdhc_card_if_pwr_en */
const cyhal_resource_pin_mapping_t cyhal_pin_map_sdhc_card_if_pwr_en[2] = {
    {1u, 0u, P6_2, P6_2_SDHC1_CARD_IF_PWR_EN},
    {0u, 0u, P21_4, P21_4_SDHC0_CARD_IF_PWR_EN},
};

/* Connections for: sdhc_card_mech_write_prot */
const cyhal_resource_pin_mapping_t cyhal_pin_map_sdhc_card_mech_write_prot[2] = {
    {1u, 0u, P6_0, P6_0_SDHC1_CARD_MECH_WRITE_PROT},
    {0u, 0u, P21_2, P21_2_SDHC0_CARD_MECH_WRITE_PROT},
};

/* Connections for: sdhc_clk_card */
const cyhal_resource_pin_mapping_t cyhal_pin_map_sdhc_clk_card[1] = {
    {0u, 0u, P12_0, P12_0_SDHC0_CLK_CARD},
};

/* Connections for: sdhc_io_volt_sel */
const cyhal_resource_pin_mapping_t cyhal_pin_map_sdhc_io_volt_sel[2] = {
    {1u, 0u, P6_3, P6_3_SDHC1_IO_VOLT_SEL},
    {0u, 0u, P21_3, P21_3_SDHC0_IO_VOLT_SEL},
};

/* Connections for: sdhc_led_ctrl */
const cyhal_resource_pin_mapping_t cyhal_pin_map_sdhc_led_ctrl[1] = {
    {1u, 0u, P6_1, P6_1_SDHC1_LED_CTRL},
};

/* Connections for: smif_spi_data0 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_smif_spi_data0[2] = {
    {0u, 0u, P1_0, HSIOM_SEL_ACT_15},
    {0u, 0u, SMIF0_SPIHB_DATA0, HSIOM_SEL_ACT_15},
};

/* Connections for: smif_spi_data1 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_smif_spi_data1[2] = {
    {0u, 0u, P1_1, HSIOM_SEL_ACT_15},
    {0u, 0u, SMIF0_SPIHB_DATA1, HSIOM_SEL_ACT_15},
};

/* Connections for: smif_spi_data2 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_smif_spi_data2[2] = {
    {0u, 0u, P1_2, HSIOM_SEL_ACT_15},
    {0u, 0u, SMIF0_SPIHB_DATA2, HSIOM_SEL_ACT_15},
};

/* Connections for: smif_spi_data3 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_smif_spi_data3[2] = {
    {0u, 0u, P1_3, HSIOM_SEL_ACT_15},
    {0u, 0u, SMIF0_SPIHB_DATA3, HSIOM_SEL_ACT_15},
};

/* Connections for: smif_spi_data4 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_smif_spi_data4[2] = {
    {0u, 0u, P1_4, HSIOM_SEL_ACT_15},
    {0u, 0u, SMIF0_SPIHB_DATA4, HSIOM_SEL_ACT_15},
};

/* Connections for: smif_spi_data5 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_smif_spi_data5[2] = {
    {0u, 0u, P1_5, HSIOM_SEL_ACT_15},
    {0u, 0u, SMIF0_SPIHB_DATA5, HSIOM_SEL_ACT_15},
};

/* Connections for: smif_spi_data6 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_smif_spi_data6[2] = {
    {0u, 0u, P1_6, HSIOM_SEL_ACT_15},
    {0u, 0u, SMIF0_SPIHB_DATA6, HSIOM_SEL_ACT_15},
};

/* Connections for: smif_spi_data7 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_smif_spi_data7[2] = {
    {0u, 0u, P1_7, HSIOM_SEL_ACT_15},
    {0u, 0u, SMIF0_SPIHB_DATA7, HSIOM_SEL_ACT_15},
};

/* Connections for: smif_spi_select0 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_smif_spi_select0[2] = {
    {0u, 0u, P5_0, P5_0_SMIF_SMIF0_SPIHB_SELECT0},
    {1u, 0u, P21_7, P21_7_SMIF_SMIF1_SPIHB_SELECT0},
};

/* Connections for: smif_spi_select1 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_smif_spi_select1[1] = {
    {0u, 0u, P2_0, P2_0_SMIF_SMIF0_SPIHB_SELECT1},
};

/* Connections for: smif_spi_select2 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_smif_spi_select2[1] = {
    {0u, 0u, NC, HSIOM_SEL_GPIO},
};

/* Connections for: smif_spi_select3 */
const cyhal_resource_pin_mapping_t cyhal_pin_map_smif_spi_select3[1] = {
    {0u, 0u, NC, HSIOM_SEL_GPIO},
};

/* Connections for: tcpwm_line */
const cyhal_resource_pin_mapping_t cyhal_pin_map_tcpwm_line[97] = {
    {0u, 1u, SMIF0_SPIHB_DATA0, SMIF0_SPIHB_DATA0_TCPWM0_LINE1},
    {0u, 1u, P1_0, SMIF0_SPIHB_DATA0_TCPWM0_LINE1},
    {1u, 1u, P1_0, SMIF0_SPIHB_DATA0_TCPWM0_LINE257},
    {1u, 1u, SMIF0_SPIHB_DATA0, SMIF0_SPIHB_DATA0_TCPWM0_LINE257},
    {0u, 2u, P1_2, SMIF0_SPIHB_DATA2_TCPWM0_LINE2},
    {0u, 2u, SMIF0_SPIHB_DATA2, SMIF0_SPIHB_DATA2_TCPWM0_LINE2},
    {1u, 2u, SMIF0_SPIHB_DATA2, SMIF0_SPIHB_DATA2_TCPWM0_LINE258},
    {1u, 2u, P1_2, SMIF0_SPIHB_DATA2_TCPWM0_LINE258},
    {0u, 3u, P1_4, SMIF0_SPIHB_DATA4_TCPWM0_LINE3},
    {0u, 3u, SMIF0_SPIHB_DATA4, SMIF0_SPIHB_DATA4_TCPWM0_LINE3},
    {1u, 3u, P1_4, SMIF0_SPIHB_DATA4_TCPWM0_LINE259},
    {1u, 3u, SMIF0_SPIHB_DATA4, SMIF0_SPIHB_DATA4_TCPWM0_LINE259},
    {0u, 4u, SMIF0_SPIHB_DATA6, SMIF0_SPIHB_DATA6_TCPWM0_LINE4},
    {0u, 4u, P1_6, SMIF0_SPIHB_DATA6_TCPWM0_LINE4},
    {1u, 4u, P1_6, SMIF0_SPIHB_DATA6_TCPWM0_LINE260},
    {1u, 4u, SMIF0_SPIHB_DATA6, SMIF0_SPIHB_DATA6_TCPWM0_LINE260},
    {0u, 5u, P2_0, P2_0_TCPWM0_LINE5},
    {1u, 5u, P2_0, P2_0_TCPWM0_LINE261},
    {0u, 3u, P6_0, P6_0_TCPWM0_LINE3},
    {1u, 11u, P6_0, P6_0_TCPWM0_LINE267},
    {0u, 4u, P6_2, P6_2_TCPWM0_LINE4},
    {1u, 12u, P6_2, P6_2_TCPWM0_LINE268},
    {0u, 0u, P7_2, P7_2_TCPWM0_LINE0},
    {1u, 16u, P7_2, P7_2_TCPWM0_LINE272},
    {0u, 1u, P7_4, P7_4_TCPWM0_LINE1},
    {1u, 17u, P7_4, P7_4_TCPWM0_LINE273},
    {0u, 3u, P8_0, P8_0_TCPWM0_LINE3},
    {1u, 19u, P8_0, P8_0_TCPWM0_LINE275},
    {0u, 4u, P8_2, P8_2_TCPWM0_LINE4},
    {1u, 20u, P8_2, P8_2_TCPWM0_LINE276},
    {0u, 5u, P8_4, P8_4_TCPWM0_LINE5},
    {1u, 21u, P8_4, P8_4_TCPWM0_LINE277},
    {0u, 6u, P8_6, P8_6_TCPWM0_LINE6},
    {1u, 22u, P8_6, P8_6_TCPWM0_LINE278},
    {0u, 7u, P9_0, P9_0_TCPWM0_LINE7},
    {0u, 0u, P9_1, P9_1_TCPWM0_LINE0},
    {1u, 23u, P9_2, P9_2_TCPWM0_LINE279},
    {1u, 0u, P9_3, P9_3_TCPWM0_LINE256},
    {0u, 4u, P10_6, P10_6_TCPWM0_LINE4},
    {1u, 4u, P10_6, P10_6_TCPWM0_LINE260},
    {0u, 5u, P11_0, P11_0_TCPWM0_LINE5},
    {1u, 5u, P11_0, P11_0_TCPWM0_LINE261},
    {0u, 6u, P11_2, P11_2_TCPWM0_LINE6},
    {1u, 6u, P11_2, P11_2_TCPWM0_LINE262},
    {0u, 7u, P11_4, P11_4_TCPWM0_LINE7},
    {1u, 7u, P11_4, P11_4_TCPWM0_LINE263},
    {0u, 0u, P11_6, P11_6_TCPWM0_LINE0},
    {1u, 8u, P11_6, P11_6_TCPWM0_LINE264},
    {0u, 1u, P12_0, P12_0_TCPWM0_LINE1},
    {1u, 9u, P12_0, P12_0_TCPWM0_LINE265},
    {0u, 2u, P12_2, P12_2_TCPWM0_LINE2},
    {1u, 10u, P12_2, P12_2_TCPWM0_LINE266},
    {0u, 3u, P12_4, P12_4_TCPWM0_LINE3},
    {1u, 11u, P12_4, P12_4_TCPWM0_LINE267},
    {0u, 4u, P13_1, P13_1_TCPWM0_LINE4},
    {1u, 12u, P13_1, P13_1_TCPWM0_LINE268},
    {0u, 5u, P13_3, P13_3_TCPWM0_LINE5},
    {1u, 13u, P13_3, P13_3_TCPWM0_LINE269},
    {0u, 6u, P13_6, P13_6_TCPWM0_LINE6},
    {1u, 14u, P13_6, P13_6_TCPWM0_LINE270},
    {0u, 2u, P15_0, P15_0_TCPWM0_LINE2},
    {1u, 18u, P15_0, P15_0_TCPWM0_LINE274},
    {0u, 3u, P15_2, P15_2_TCPWM0_LINE3},
    {1u, 19u, P15_2, P15_2_TCPWM0_LINE275},
    {0u, 4u, P15_4, P15_4_TCPWM0_LINE4},
    {1u, 20u, P15_4, P15_4_TCPWM0_LINE276},
    {0u, 5u, P15_6, P15_6_TCPWM0_LINE5},
    {1u, 21u, P15_6, P15_6_TCPWM0_LINE277},
    {0u, 0u, P16_0, P16_0_TCPWM0_LINE0},
    {1u, 0u, P16_0, P16_0_TCPWM0_LINE256},
    {0u, 1u, P16_1, P16_1_TCPWM0_LINE1},
    {1u, 1u, P16_1, P16_1_TCPWM0_LINE257},
    {0u, 2u, P16_2, P16_2_TCPWM0_LINE2},
    {1u, 2u, P16_2, P16_2_TCPWM0_LINE258},
    {0u, 3u, P16_3, P16_3_TCPWM0_LINE3},
    {1u, 3u, P16_3, P16_3_TCPWM0_LINE259},
    {0u, 4u, P16_4, P16_4_TCPWM0_LINE4},
    {1u, 4u, P16_4, P16_4_TCPWM0_LINE260},
    {0u, 5u, P16_5, P16_5_TCPWM0_LINE5},
    {1u, 5u, P16_5, P16_5_TCPWM0_LINE261},
    {0u, 6u, P16_6, P16_6_TCPWM0_LINE6},
    {1u, 22u, P16_6, P16_6_TCPWM0_LINE278},
    {0u, 7u, P16_7, P16_7_TCPWM0_LINE7},
    {1u, 23u, P16_7, P16_7_TCPWM0_LINE279},
    {0u, 7u, P20_0, P20_0_TCPWM0_LINE7},
    {0u, 0u, P20_1, P20_1_TCPWM0_LINE0},
    {0u, 1u, P20_2, P20_2_TCPWM0_LINE1},
    {0u, 2u, P20_3, P20_3_TCPWM0_LINE2},
    {1u, 7u, P20_4, P20_4_TCPWM0_LINE263},
    {0u, 3u, P21_0, P21_0_TCPWM0_LINE3},
    {1u, 11u, P21_0, P21_0_TCPWM0_LINE267},
    {0u, 4u, P21_2, P21_2_TCPWM0_LINE4},
    {1u, 12u, P21_2, P21_2_TCPWM0_LINE268},
    {0u, 5u, P21_4, P21_4_TCPWM0_LINE5},
    {1u, 13u, P21_4, P21_4_TCPWM0_LINE269},
    {0u, 6u, P21_6, P21_6_TCPWM0_LINE6},
    {1u, 14u, P21_6, P21_6_TCPWM0_LINE270},
};

/* Connections for: tcpwm_line_compl */
const cyhal_resource_pin_mapping_t cyhal_pin_map_tcpwm_line_compl[93] = {
    {0u, 1u, SMIF0_SPIHB_DATA1, SMIF0_SPIHB_DATA1_TCPWM0_LINE_COMPL1},
    {0u, 1u, P1_1, SMIF0_SPIHB_DATA1_TCPWM0_LINE_COMPL1},
    {1u, 1u, P1_1, SMIF0_SPIHB_DATA1_TCPWM0_LINE_COMPL257},
    {1u, 1u, SMIF0_SPIHB_DATA1, SMIF0_SPIHB_DATA1_TCPWM0_LINE_COMPL257},
    {0u, 2u, P1_3, SMIF0_SPIHB_DATA3_TCPWM0_LINE_COMPL2},
    {0u, 2u, SMIF0_SPIHB_DATA3, SMIF0_SPIHB_DATA3_TCPWM0_LINE_COMPL2},
    {1u, 2u, P1_3, SMIF0_SPIHB_DATA3_TCPWM0_LINE_COMPL258},
    {1u, 2u, SMIF0_SPIHB_DATA3, SMIF0_SPIHB_DATA3_TCPWM0_LINE_COMPL258},
    {0u, 3u, SMIF0_SPIHB_DATA5, SMIF0_SPIHB_DATA5_TCPWM0_LINE_COMPL3},
    {0u, 3u, P1_5, SMIF0_SPIHB_DATA5_TCPWM0_LINE_COMPL3},
    {1u, 3u, SMIF0_SPIHB_DATA5, SMIF0_SPIHB_DATA5_TCPWM0_LINE_COMPL259},
    {1u, 3u, P1_5, SMIF0_SPIHB_DATA5_TCPWM0_LINE_COMPL259},
    {0u, 4u, SMIF0_SPIHB_DATA7, SMIF0_SPIHB_DATA7_TCPWM0_LINE_COMPL4},
    {0u, 4u, P1_7, SMIF0_SPIHB_DATA7_TCPWM0_LINE_COMPL4},
    {1u, 4u, P1_7, SMIF0_SPIHB_DATA7_TCPWM0_LINE_COMPL260},
    {1u, 4u, SMIF0_SPIHB_DATA7, SMIF0_SPIHB_DATA7_TCPWM0_LINE_COMPL260},
    {0u, 2u, P5_0, P5_0_TCPWM0_LINE_COMPL2},
    {1u, 10u, P5_0, P5_0_TCPWM0_LINE_COMPL266},
    {0u, 3u, P6_1, P6_1_TCPWM0_LINE_COMPL3},
    {1u, 11u, P6_1, P6_1_TCPWM0_LINE_COMPL267},
    {0u, 4u, P6_3, P6_3_TCPWM0_LINE_COMPL4},
    {1u, 12u, P6_3, P6_3_TCPWM0_LINE_COMPL268},
    {0u, 3u, P8_1, P8_1_TCPWM0_LINE_COMPL3},
    {1u, 19u, P8_1, P8_1_TCPWM0_LINE_COMPL275},
    {0u, 4u, P8_3, P8_3_TCPWM0_LINE_COMPL4},
    {1u, 20u, P8_3, P8_3_TCPWM0_LINE_COMPL276},
    {0u, 5u, P8_5, P8_5_TCPWM0_LINE_COMPL5},
    {1u, 21u, P8_5, P8_5_TCPWM0_LINE_COMPL277},
    {0u, 6u, P8_7, P8_7_TCPWM0_LINE_COMPL6},
    {1u, 22u, P8_7, P8_7_TCPWM0_LINE_COMPL278},
    {1u, 23u, P9_0, P9_0_TCPWM0_LINE_COMPL279},
    {1u, 0u, P9_1, P9_1_TCPWM0_LINE_COMPL256},
    {0u, 7u, P9_2, P9_2_TCPWM0_LINE_COMPL7},
    {0u, 0u, P9_3, P9_3_TCPWM0_LINE_COMPL0},
    {0u, 3u, P10_5, P10_5_TCPWM0_LINE_COMPL3},
    {1u, 3u, P10_5, P10_5_TCPWM0_LINE_COMPL259},
    {0u, 4u, P10_7, P10_7_TCPWM0_LINE_COMPL4},
    {1u, 4u, P10_7, P10_7_TCPWM0_LINE_COMPL260},
    {0u, 5u, P11_1, P11_1_TCPWM0_LINE_COMPL5},
    {1u, 5u, P11_1, P11_1_TCPWM0_LINE_COMPL261},
    {0u, 6u, P11_3, P11_3_TCPWM0_LINE_COMPL6},
    {1u, 6u, P11_3, P11_3_TCPWM0_LINE_COMPL262},
    {0u, 7u, P11_5, P11_5_TCPWM0_LINE_COMPL7},
    {1u, 7u, P11_5, P11_5_TCPWM0_LINE_COMPL263},
    {0u, 0u, P11_7, P11_7_TCPWM0_LINE_COMPL0},
    {1u, 8u, P11_7, P11_7_TCPWM0_LINE_COMPL264},
    {0u, 1u, P12_1, P12_1_TCPWM0_LINE_COMPL1},
    {1u, 9u, P12_1, P12_1_TCPWM0_LINE_COMPL265},
    {0u, 3u, P12_5, P12_5_TCPWM0_LINE_COMPL3},
    {1u, 11u, P12_5, P12_5_TCPWM0_LINE_COMPL267},
    {0u, 4u, P13_2, P13_2_TCPWM0_LINE_COMPL4},
    {1u, 12u, P13_2, P13_2_TCPWM0_LINE_COMPL268},
    {0u, 5u, P13_4, P13_4_TCPWM0_LINE_COMPL5},
    {1u, 13u, P13_4, P13_4_TCPWM0_LINE_COMPL269},
    {0u, 6u, P13_7, P13_7_TCPWM0_LINE_COMPL6},
    {1u, 14u, P13_7, P13_7_TCPWM0_LINE_COMPL270},
    {0u, 2u, P15_1, P15_1_TCPWM0_LINE_COMPL2},
    {1u, 18u, P15_1, P15_1_TCPWM0_LINE_COMPL274},
    {0u, 3u, P15_3, P15_3_TCPWM0_LINE_COMPL3},
    {1u, 19u, P15_3, P15_3_TCPWM0_LINE_COMPL275},
    {0u, 4u, P15_5, P15_5_TCPWM0_LINE_COMPL4},
    {1u, 20u, P15_5, P15_5_TCPWM0_LINE_COMPL276},
    {0u, 5u, P15_7, P15_7_TCPWM0_LINE_COMPL5},
    {1u, 21u, P15_7, P15_7_TCPWM0_LINE_COMPL277},
    {0u, 0u, P17_0, P17_0_TCPWM0_LINE_COMPL0},
    {1u, 0u, P17_0, P17_0_TCPWM0_LINE_COMPL256},
    {0u, 1u, P17_1, P17_1_TCPWM0_LINE_COMPL1},
    {1u, 1u, P17_1, P17_1_TCPWM0_LINE_COMPL257},
    {0u, 2u, P17_2, P17_2_TCPWM0_LINE_COMPL2},
    {1u, 2u, P17_2, P17_2_TCPWM0_LINE_COMPL258},
    {0u, 3u, P17_3, P17_3_TCPWM0_LINE_COMPL3},
    {1u, 3u, P17_3, P17_3_TCPWM0_LINE_COMPL259},
    {0u, 4u, P17_4, P17_4_TCPWM0_LINE_COMPL4},
    {1u, 4u, P17_4, P17_4_TCPWM0_LINE_COMPL260},
    {0u, 5u, P17_5, P17_5_TCPWM0_LINE_COMPL5},
    {1u, 5u, P17_5, P17_5_TCPWM0_LINE_COMPL261},
    {0u, 6u, P17_6, P17_6_TCPWM0_LINE_COMPL6},
    {1u, 22u, P17_6, P17_6_TCPWM0_LINE_COMPL278},
    {0u, 7u, P17_7, P17_7_TCPWM0_LINE_COMPL7},
    {1u, 23u, P17_7, P17_7_TCPWM0_LINE_COMPL279},
    {1u, 7u, P20_0, P20_0_TCPWM0_LINE_COMPL263},
    {1u, 8u, P20_1, P20_1_TCPWM0_LINE_COMPL264},
    {1u, 9u, P20_2, P20_2_TCPWM0_LINE_COMPL265},
    {1u, 10u, P20_3, P20_3_TCPWM0_LINE_COMPL266},
    {0u, 7u, P20_4, P20_4_TCPWM0_LINE_COMPL7},
    {0u, 3u, P21_1, P21_1_TCPWM0_LINE_COMPL3},
    {1u, 11u, P21_1, P21_1_TCPWM0_LINE_COMPL267},
    {0u, 4u, P21_3, P21_3_TCPWM0_LINE_COMPL4},
    {1u, 12u, P21_3, P21_3_TCPWM0_LINE_COMPL268},
    {0u, 5u, P21_5, P21_5_TCPWM0_LINE_COMPL5},
    {1u, 13u, P21_5, P21_5_TCPWM0_LINE_COMPL269},
    {0u, 6u, P21_7, P21_7_TCPWM0_LINE_COMPL6},
    {1u, 14u, P21_7, P21_7_TCPWM0_LINE_COMPL270},
};

/* Connections for: tdm_tdm_rx_fsync */
const cyhal_resource_pin_mapping_t cyhal_pin_map_tdm_tdm_rx_fsync[2] = {
    {0u, 1u, P11_5, P11_5_TDM_TDM_RX_FSYNC1},
    {0u, 0u, P21_5, P21_5_TDM_TDM_RX_FSYNC0},
};

/* Connections for: tdm_tdm_rx_mck */
const cyhal_resource_pin_mapping_t cyhal_pin_map_tdm_tdm_rx_mck[2] = {
    {0u, 1u, P11_3, P11_3_TDM_TDM_RX_MCK1},
    {0u, 0u, P21_7, P21_7_TDM_TDM_RX_MCK0},
};

/* Connections for: tdm_tdm_rx_sck */
const cyhal_resource_pin_mapping_t cyhal_pin_map_tdm_tdm_rx_sck[2] = {
    {0u, 1u, P11_4, P11_4_TDM_TDM_RX_SCK1},
    {0u, 0u, P21_6, P21_6_TDM_TDM_RX_SCK0},
};

/* Connections for: tdm_tdm_rx_sd */
const cyhal_resource_pin_mapping_t cyhal_pin_map_tdm_tdm_rx_sd[2] = {
    {0u, 1u, P11_6, P11_6_TDM_TDM_RX_SD1},
    {0u, 0u, P21_4, P21_4_TDM_TDM_RX_SD0},
};

/* Connections for: tdm_tdm_tx_fsync */
const cyhal_resource_pin_mapping_t cyhal_pin_map_tdm_tdm_tx_fsync[1] = {
    {0u, 1u, P11_1, P11_1_TDM_TDM_TX_FSYNC1},
};

/* Connections for: tdm_tdm_tx_mck */
const cyhal_resource_pin_mapping_t cyhal_pin_map_tdm_tdm_tx_mck[2] = {
    {0u, 1u, P11_7, P11_7_TDM_TDM_TX_MCK1},
    {0u, 0u, P21_3, P21_3_TDM_TDM_TX_MCK0},
};

/* Connections for: tdm_tdm_tx_sck */
const cyhal_resource_pin_mapping_t cyhal_pin_map_tdm_tdm_tx_sck[2] = {
    {0u, 1u, P11_0, P11_0_TDM_TDM_TX_SCK1},
    {0u, 0u, P21_2, P21_2_TDM_TDM_TX_SCK0},
};

/* Connections for: tdm_tdm_tx_sd */
const cyhal_resource_pin_mapping_t cyhal_pin_map_tdm_tdm_tx_sd[2] = {
    {0u, 1u, P11_2, P11_2_TDM_TDM_TX_SD1},
    {0u, 0u, P21_1, P21_1_TDM_TDM_TX_SD0},
};

#endif
