import ftplib


def send_to_cpp(text):
    print("////////CPP\\\\\\\\\\\\\\\\")
    print(text)
    print("\\\\\\\\\\\\\\\\CPP////////")
    pass


name = ' '
psd = ' '
ftp = ftplib.FTP("209.51.188.20")
ls = ''


def USER(username):
    global name
    name = username
    result = ftp.login(user=name, passwd=psd)
    send_to_cpp(result)


def PASS(password):
    global psd
    psd = password
    send_to_cpp("Okay")


def PWD():
    result = ftp.pwd()
    send_to_cpp(result)


def CWD(pathname):
    result = ftp.cwd(pathname)
    send_to_cpp(result)


def PASV():
    ftp.set_pasv(True)
    send_to_cpp("Okay")


def LIST():
    global ls
    ls = ''

    def add_line(a):
        global ls
        ls += a
        ls += '\n'

    ftp.retrlines('LIST', callback=add_line)
    send_to_cpp(ls)


def RETR(filename):
    try:
        with open(filename, 'wb') as file:
            ftp.retrbinary("RETR " + filename, file.write)
        send_to_cpp("Download " + filename)
    except BaseException as e:
        send_to_cpp(e)


USER("anonymous")
PASS(" ")
LIST()
PWD()
RETR("README")
CWD("gnu")
LIST()

