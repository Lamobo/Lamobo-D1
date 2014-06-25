#ifndef __AK_MAC_H
#define __AK_MAC_H

/* platfrom data for platfrom device structure's platfrom_data field */

#define MAC_ADDR_LEN 6
#define MAC_ADDR_STRING_LEN (MAC_ADDR_LEN * 3 - 1)


struct ak_mac_data {
	unsigned int	flags;
	unsigned char	dev_addr[MAC_ADDR_LEN];
	void (* gpio_init) (const struct gpio_info *);
	struct gpio_info pwr_gpio;			// power up
	struct gpio_info phy_rst_gpio;		// PHY RESET gpio
};


#endif	/* __AK_MAC_H */

