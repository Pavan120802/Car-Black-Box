/* 
 * File     : car_black_box.h
 * Author   : Pavan T A
 * Created on 9 February, 2025, 3:16 PM
 */

#ifndef CAR_BLACK_BOX_H
#define	CAR_BLACK_BOX_H

void display_dashboard(unsigned char event[], unsigned char speed);
void log_car_event(char event[], unsigned char speed);
void clear_screen(void);
unsigned char login(unsigned char key, unsigned char reset_flag);
unsigned char login_menu(unsigned char key, unsigned char reset_flag);
void view_log(unsigned char key, char reset_shift);
char clear_log(char reset_memory);
void download_log(void);
char change_password(unsigned char key, char reset_pwd );
char change_time(unsigned char key, unsigned char reset_time);

#endif	/* CAR_BLACK_BOX_H */