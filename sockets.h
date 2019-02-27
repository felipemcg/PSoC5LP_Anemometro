/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
int listen(int puerto,int backlog);
int accept (int sockfd);
int recv(int sockfd, void *buf, int len, unsigned int flags);
int send(int sockfd, void *msg, int len, int flags);
/* [] END OF FILE */
