
#include <linux/kernel.h>
#include <linux/delay.h>
#include <plat-anyka/wrap_sensor.h>
#include <plat-anyka/aksensor.h>

/**
 * @brief millisecond delay
 * @author dengzhou
 * @date 2012-03-16
 * @param[in] minisecond minisecond delay number
 * @return T_VOID
 */
T_VOID mini_delay(T_U32 minisecond)
{
	mdelay(minisecond);
}

/**
 * @brief anyka specific printf
 * @author dengzhou
 * @date 2012-03-16
 * @param[in] level forbidden level
 * @param[in] mStr module string
 * @param[in] s format string
 * @return T_S32
 * @retval 0 is print ok, -1 is forbidden to print
 */
T_S32 akprintf(T_U8 level, T_pCSTR mStr, T_pCSTR s, ...)
{
	va_list args;
	T_S32 r;

	va_start(args, s);
	r = vprintk(s, args);
	va_end(args);

	return r;
}

/**
 * @brief write data to SCCB device
 *
 * write size length data to daddr's raddr register
 * @author dengzhou
 * @date 2012-03-16
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @param[in] data write data's point
 * @param[in] size write data's length
 * @return T_BOOL return write success or failed
 * @retval AK_FALSE operate failed
 * @retval AK_TRUE operate success
 */
T_BOOL sccb_write_data(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size)
{
	int ret = aksensor_i2c_write_byte_data(daddr, raddr, data, size);

	if (ret != 0)
	{
		return AK_FALSE;
	}
	else
	{
		return AK_TRUE;
	}
}

/**
 * @brief read data from SCCB device function
 *
 * read data from daddr's raddr register
 * @author dengzhou
 * @date 2012-03-16
 * @param[in] daddr SCCB device address
 * @param[in] raddr register address
 * @return T_U8
 * @retval read-back data
 */
T_U8 sccb_read_data(T_U8 daddr, T_U8 raddr)
{
	return aksensor_i2c_read_byte_data(daddr, raddr);
}

T_BOOL sccb_write_short(T_U8 daddr, T_U16 raddr, T_U8 *data, T_U32 size)
{
	int ret = aksensor_i2c_write_byte_short(daddr, raddr, data, size);

	if (ret != 0)
	{
		return AK_FALSE;
	}
	else
	{
		return AK_TRUE;
	}
}

T_U8 sccb_read_short(T_U8 daddr, T_U16 raddr)
{
	return aksensor_i2c_read_byte_short(daddr, raddr);
}

T_BOOL sccb_write_word(T_U8 daddr, T_U16 raddr, T_U16 *data, T_U32 size)
{
	int ret = aksensor_i2c_write_word_data(daddr, raddr, data, size);
	if (ret != 0)
	{
		return AK_FALSE;
	}
	else
	{
		return AK_TRUE;
	}
}

T_U16 sccb_read_word(T_U8 daddr, T_U16 raddr)
{
	return aksensor_i2c_read_word_data(daddr, raddr);
}

/*@{*/
/**
 * @brief SCCB interface initialize function
 *
 * setup SCCB interface
 * @author dengzhou
 * @date 2012-03-16
 * @param[in] pin_scl the pin assigned to SCL
 * @param[in] pin_sda the pin assigned to SDA
 * @return T_VOID
 */
T_VOID sccb_init(T_U32 pin_scl, T_U32 pin_sda)
{

}


  
