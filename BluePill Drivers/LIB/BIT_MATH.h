 /*******************************************************************************
 *																				*
 * 	Module			: Utils														*
 *																				*
 * 	File Name		: BIT_MATH.h												*		
 *																				*	
 * 	Author			: AbdElRahman Sabry											*
 *																				*
 *	Date 			: 23/9/2021													*
 *																				*
 *	Version			: v1														*
 * 																				*
 *******************************************************************************/

#ifndef UTILS_H_
#define UTILS_H_

//MACROS FOR PIN

#define WRITE_BIT(REG , BIT , DATA)		( REG = (REG& ~(1<<BIT)) | (DATA<<BIT))
#define GET_BIT(REG , BIT)				(((REG)&(1<<BIT))>>BIT )
#define CHK_BIT(REG , BIT)				((REG) & (1<<BIT))
#define TOG_BIT(REG , BIT)				(REG  ^= (1<<BIT))
#define SET_BIT(REG , BIT)				(REG  |= (1<<BIT))
#define CLR_BIT(REG , BIT)				(REG  &=~(1<<BIT))

//MACROS FOR PORTS AND REGISTERS

#define WRITE_PORT(REG , DATA)			(REG = DATA)
#define PORT_SET_MASK(REG , MASK)		(REG |= (MASK))
#define PORT_CLEAR_MASK(REG ,MASK)		(REG &=~(MASK))



#define PORT_MASK(REG , CLR , SET)  	(REG = (REG&~(CLR))|(SET))



#define SET_REG(REG)					(REG = 0xffffffff)
#define CLEAR_REG(REG)					(REG = 0x00000000)
#define TOGGLE_REG(REG)					(REG ^=0xffffffff)

//LOWER NIBBLE MACROS	(0 - 15)

#define GET_LOWER(REG)					(REG &   0x0000ffff)
#define SET_LOWER_NIBBLE(REG)			(REG | = 0x0000ffff)
#define CLEAR_LOWER_NIBBLE(REG)			(REG & = 0xffff0000)


#define LOWER_NIBBLE_DATA(REG , DATA) 	(REG = (REG&0xffff0000)|DATA ) 

//HIGHER NIBBLE MACROS	(16 - 31)

#define GET_HIGHER(REG)					((REG&0xffff0000)>>16)
#define SET_HIGHER_NIBBLE(REG)			(REG | =0xffff0000)
#define CLEAR_HIGHER_NIBBLE(REG)		(REG & = 0x0000ffff)

#define HIGHER_NIBBLE_DATA(REG , DATA)	(REG = (REG&0x0000ffff) | (DATA<<16))



#endif /* UTILS_H_ */