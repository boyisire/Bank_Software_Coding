#define BUFLEN		8196

typedef struct {
	char	type;			/* type of transfer (bin/ascii) */
	char    a[7];
	int	sockfd;			/* CNTRL connection socket id */
	int	listsd;			/* LISTEN connection socket id */
	int	datasd;			/* DATA connection socket id */
	int	debug;			/* flag to turn on debugging */
        int	timeout;
	int	loginok;
} FTPINFO;

extern int ftp_login(FTPINFO *, char *, char *, char *, char *);
extern int ftp_getfile(FTPINFO *, char *, char *);
extern int ftp_putfile(FTPINFO *, char *, char *);
extern int ftp_bye(FTPINFO *);
extern int ftp_ascii(FTPINFO *);
extern int ftp_binary(FTPINFO *);
extern int ftp_timeout(FTPINFO *);
